import sys
import os
import time
from unittest.mock import MagicMock
import glob

sys.modules['pynput'] = MagicMock()
sys.modules['pynput.keyboard'] = MagicMock()

from xdpchandler import *

"""
DOT0:D4:22:CD:07:AB:58
DOT1:D4:22:CD:07:AA:E1
DOT2:D4:22:CD:07:AA:0E
DOT3:D4:22:CD:07:AB:5B
DOT4:D4:22:CD:07:AA:B5
"""

TARGET_MAC = "D4:22:CD:07:AB:58"

def main():
    # コマンドライン引数からディレクトリを取得 (指定がなければカレント)
    out_dir = sys.argv[1] if len(sys.argv) > 1 else "."
    # 念のためディレクトリが存在するか確認し、なければ作成
    os.makedirs(out_dir, exist_ok=True)
    
    print(f"Initializing Xsens DOT SDK... Targeting: {TARGET_MAC}")
    print(f"Output directory set to: {out_dir}")
    
    handler = XdpcHandler()
    if not handler.initialize():
        print("Failed to initialize XdpcHandler")
        return
        
    handler.scanForDots()
    if len(handler.detectedDots()) == 0:
        print("No DOTs detected")
        return
        
    handler.connectDots()
    connected = handler.connectedDots()
    if not connected:
        print("Failed to connect")
        return

    print("Setting up all devices (Mirroring Official Script)...")
    for device in connected:
        device.setOutputRate(60)
        mac = device.portInfo().bluetoothAddress()
        
        # dataフォルダ内の「一番新しいフォルダ（SSIが今作ったフォルダ）」を自動取得
        out_dir = "."
        dirs = glob.glob("data/*/")
        if dirs:
            out_dir = max(dirs, key=os.path.getctime)
            
        filename = os.path.join(out_dir, f"logfile_{mac.replace(':', '-')}.csv")
        if device.enableLogging(filename):
            print(f"[Movella Info] Enabled native logging for {mac} to {filename}")
        if device.enableLogging(filename):
            print(f"[Movella Info] Enabled native logging for {mac} to {filename}")
        
        device.startMeasurement(movelladot_pc_sdk.XsPayloadMode_ExtendedQuaternion)

    print("Starting main loop...")
    count = 0
    
    try:
        while True:
            if handler.packetsAvailable():
                for device in connected:
                    mac = device.portInfo().bluetoothAddress()
                    packet = handler.getNextPacket(mac)
                    
                    if mac == TARGET_MAC and packet and packet.containsOrientation():
                        time_fine = packet.sampleTimeFine()
                        quat = packet.orientationQuaternion()
                        
                        try:
                            qw, qx, qy, qz = quat[0], quat[1], quat[2], quat[3]
                        except:
                            qw = quat.w() if hasattr(quat, 'w') else 0.0
                            qx = quat.x() if hasattr(quat, 'x') else 0.0
                            qy = quat.y() if hasattr(quat, 'y') else 0.0
                            qz = quat.z() if hasattr(quat, 'z') else 0.0

                        ax, ay, az = 0.0, 0.0, 0.0
                        if packet.containsFreeAcceleration():
                            acc = packet.freeAcceleration()
                            try:
                                ax, ay, az = acc[0], acc[1], acc[2]
                            except:
                                ax = acc.x() if hasattr(acc, 'x') else 0.0
                                ay = acc.y() if hasattr(acc, 'y') else 0.0
                                az = acc.z() if hasattr(acc, 'z') else 0.0
                                
                        status = packet.status()
                        
                        print(f"{time_fine},{qw},{qx},{qy},{qz},{ax},{ay},{az},{status}", flush=True)
                        
                        count += 1
                        if count % 300 == 0:
                            print(f"[Movella Worker Info] Successfully streamed {count} frames from {TARGET_MAC}", flush=True)
            else:
                time.sleep(0.001)
                
    except Exception as e:
        import traceback
        print(f"CRITICAL ERROR in worker: {e}")
        traceback.print_exc()
    except KeyboardInterrupt:
        pass
    finally:
        for device in connected:
            device.stopMeasurement()
            device.disableLogging()
        handler.cleanup()

if __name__ == "__main__":
    main()