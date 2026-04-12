import subprocess
import os
import time

PYTHON_EXE = r"C:\Users\shogo\anaconda3\envs\gopro\python.exe"
SCRIPT_PATH = r"C:\ssi\lonebula\gopro_action.py"

def getOptions(opts, vars):
    pass

def getChannelNames(opts, vars):
    return { 'state' : 'GoPro connection status' }

def initChannel(name, channel, types, opts, vars):
    if name == 'state':
        channel.dim = 1
        channel.type = types.FLOAT
        channel.sr = 10.0
    else: 
        print('unknown channel name')

def trigger_gopro(action):
    clean_env = os.environ.copy()
    clean_env.pop("PYTHONHOME", None)
    clean_env.pop("PYTHONPATH", None)
    try:
        subprocess.Popen([PYTHON_EXE, SCRIPT_PATH, action], env=clean_env)
    except Exception as e:
        print(f"Error {action} GoPro: {e}")

def connect(opts, vars):
    print(">>> [GoPro] Connected. Waiting for Movella setup...")
    vars['has_started'] = False
    vars['state_val'] = 0.0
    # 初期状態ではフラグを0にリセット
    os.environ["SSI_MOVELLA_READY"] = "0"

def read(name, sout, reset, board, opts, vars):
    if not vars['has_started']:
        # 1. MovellaのBluetooth接続完了を待機 (環境変数で通信)
        while os.environ.get("SSI_MOVELLA_READY") != "1":
            time.sleep(0.5)
            
        # 2. SSIのカウントダウン(wait="5.0")に正確に合わせる
        print(">>> [GoPro] Movella Ready detected. Waiting 5.0s for SSI countdown...")
        time.sleep(5.0)
        
        print("\n" + "="*60)
        print(">>> [SSI] PIPELINE RUNNING! Triggering GoPro REC...")
        trigger_gopro("start")
        vars['has_started'] = True
        vars['state_val'] = 1.0
            
    for i in range(sout.num):
        sout[i, 0] = vars['state_val']

def disconnect(opts, vars):
    print(">>> [SSI] Stopping GoPro...")
    trigger_gopro("stop")
    trigger_gopro("stream_stop")