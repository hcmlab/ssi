import socket

UDP_IP = "127.0.0.1"
UDP_PORT = 1111
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

input("\n\tpress enter to start pipeline\n")

MESSAGE = "SSI:STRT:RUN1\0"
sock.sendto(bytes(MESSAGE, "utf-8"), (UDP_IP, UDP_PORT))

input("\n\tpress enter to stop pipeline\n")

MESSAGE = "SSI:STOP:RUN1\0"
sock.sendto(bytes(MESSAGE, "utf-8"), (UDP_IP, UDP_PORT))
