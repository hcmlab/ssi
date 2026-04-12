from xdpchandler import *
import time
import csv

if __name__ == "__main__":
    xdpcHandler = XdpcHandler()

    if not xdpcHandler.initialize():
        print("初期化に失敗しました。")
        exit(-1)

    xdpcHandler.scanForDots()
    if len(xdpcHandler.detectedDots()) == 0:
        print("Movella DOTが見つかりません。")
        xdpcHandler.cleanup()
        exit(-1)

    xdpcHandler.connectDots()
    connected_devices = xdpcHandler.connectedDots()
    if len(connected_devices) == 0:
        print("Movella DOTに接続できませんでした。")
        xdpcHandler.cleanup()
        exit(-1)

    # テストとして1台目のデバイスを使用
    device = connected_devices[0]
    mac_address = device.bluetoothAddress()
    print(f"接続成功: {mac_address}")

    # 【修正ポイント】9軸の生データをリアルタイムストリーミングするモード
    print("測定モードを RateQuantitieswMag (9軸生データ取得) に設定します...")
    if not device.startMeasurement(movelladot_pc_sdk.XsPayloadMode_RateQuantitieswMag):
        print(f"測定開始エラー: {device.lastResultText()}")
        xdpcHandler.cleanup()
        exit(-1)

    csv_filename = f"9axis_raw_data_{mac_address.replace(':', '')}.csv"
    print(f"\n10秒間、9軸データを {csv_filename} に記録します...")

    with open(csv_filename, mode='w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(['Timestamp', 'Acc_X', 'Acc_Y', 'Acc_Z', 'Gyr_X', 'Gyr_Y', 'Gyr_Z', 'Mag_X', 'Mag_Y', 'Mag_Z'])

        start_time = time.time()
        packet_count = 0

        # メインループ (10秒間データを受信)
        while time.time() - start_time <= 10.0:
            if xdpcHandler.packetsAvailable():
                # 該当MACアドレスのパケットを取り出す
                packet = xdpcHandler.getNextPacket(mac_address)

                # パケットが存在し、かつ角速度データ(キャリブレーション済)が含まれているかチェック
                if packet and packet.containsCalibratedGyroscopeData():
                    # 9軸データの抽出
                    acc = packet.calibratedAcceleration()
                    gyr = packet.calibratedGyroscopeData()
                    mag = packet.calibratedMagneticField()

                    writer.writerow([
                        packet.sampleTimeFine(),
                        f"{acc[0]:.4f}", f"{acc[1]:.4f}", f"{acc[2]:.4f}",
                        f"{gyr[0]:.4f}", f"{gyr[1]:.4f}", f"{gyr[2]:.4f}",
                        f"{mag[0]:.4f}", f"{mag[1]:.4f}", f"{mag[2]:.4f}"
                    ])
                    packet_count += 1
                    
                    print(f"記録中... {packet_count} フレーム取得済み\r", end="", flush=True)

    print(f"\n\n記録終了！ 合計 {packet_count} フレームのデータを保存しました。")

    # 終了処理
    device.stopMeasurement()
    xdpcHandler.cleanup()