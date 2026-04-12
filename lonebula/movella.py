import subprocess
import os
import threading
import queue
import time

PYTHON_EXE = r"C:\Users\shogo\anaconda3\envs\movella\python.exe"
SCRIPT_PATH = r"C:\ssi\lonebula\movella_worker.py"

def getOptions(opts, vars):
    # XMLから dir="data/$(date)" を受け取るための枠を定義
    opts['dir'] = '.' 

def getChannelNames(opts, vars):
    return { 'sensor' : 'Movella DOT 9-ch (Time, Quat, FreeAcc, Status)' }

def initChannel(name, channel, types, opts, vars):
    if name == 'sensor':
        channel.dim = 9
        channel.type = types.FLOAT
        channel.sr = 60.0

def enqueue_output(out, q):
    for line in iter(out.readline, b''):
        line_str = line.decode('utf-8', errors='ignore').strip()
        if not line_str: 
            continue
        
        # カンマで分割し、要素数がきっちり9個あるかチェック
        parts = line_str.split(',')
        if len(parts) == 9:
            try:
                vals = [float(x.strip()) for x in parts]
                q.put(vals)
            except ValueError:
                # 数値変換に失敗した場合はログとして扱う
                print(f"[Movella Worker Info] {line_str}")
        else:
            # 9要素のCSVでない出力（公式サンプルのprint文など）はすべてログとして表示
            print(f"[Movella Worker Info] {line_str}")
            
    out.close()

def connect(opts, vars):
    print(">>> [SSI] Starting Movella subprocess...")
    clean_env = os.environ.copy()
    clean_env.pop("PYTHONHOME", None)
    clean_env.pop("PYTHONPATH", None)
    
    # 受け取った出力先ディレクトリ
    out_dir = opts['dir'] 
    
    try:
        # 引数として out_dir を追加して起動
        proc = subprocess.Popen(
            [PYTHON_EXE, "-u", SCRIPT_PATH, out_dir],
            env=clean_env,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT
        )
    except Exception as e:
        print(f"[Movella Error] Failed to start subprocess: {e}")
        return False
        
    q = queue.Queue()
    t = threading.Thread(target=enqueue_output, args=(proc.stdout, q))
    t.daemon = True
    t.start()
    
    print(">>> [SSI] Waiting for Movella to complete Bluetooth setup (Max 45s)...")
    
    # --- 修正ポイント：最初のデータが来るまでSSIの起動をブロックする ---
    got_data = False
    timeout = 45.0 
    start_wait = time.time()
    
    while time.time() - start_wait < timeout:
        if not q.empty():
            got_data = True
            break
        time.sleep(0.5) # 0.5秒ごとにチェック
        
    if not got_data:
        print(">>> [SSI] ERROR: Movella initialization timeout.")
        return False # 失敗をSSIに伝え、パイプラインを停止させる
    # -------------------------------------------------------------------

    vars['proc'] = proc
    vars['queue'] = q
    vars['last_data'] = q.get() 
    vars['is_ready'] = True
    
    print(">>> [SSI] Movella Ready! Starting synchronized pipeline.")
    
    # --- ここを追加：GoProに準備完了を知らせる ---
    os.environ["SSI_MOVELLA_READY"] = "1"
    # --------------------------------------------
    
    return True

def read(name, sout, reset, board, opts, vars):
    q = vars['queue']
    last_data = vars['last_data']
    
    got_new = False
    while not q.empty():
        try:
            last_data = q.get_nowait()
            got_new = True
        except queue.Empty: 
            break
            
    if got_new and not vars['is_ready']:
        vars['is_ready'] = True
        print(">>> [SSI] Movella Real Data Detected! Stream is unblocked.")
        
    vars['last_data'] = last_data
    
    for i in range(sout.num):
        for d in range(9):
            sout[i, d] = last_data[d]

def disconnect(opts, vars):
    if 'proc' in vars: 
        vars['proc'].terminate()