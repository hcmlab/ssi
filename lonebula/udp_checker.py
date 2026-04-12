import socket

UDP_IP = "0.0.0.0"
UDP_PORT = 8554

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"UDPポート {UDP_PORT} でパケットを待機中...")
print("※パケットが届くと画面に表示されます。終了は Ctrl+C")

try:
    while True:
        data, addr = sock.recvfrom(1024)
        print(f"[{addr[0]}] から {len(data)} バイトのデータを受信！")
except KeyboardInterrupt:
    print("終了します。")