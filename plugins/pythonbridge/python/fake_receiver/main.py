# This is a sample Python script.

# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.

import socket
import struct
import time

HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
PORT = 1234         # Port to listen on (non-privileged ports are > 1023)

def receive():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind((HOST, PORT))
    s.listen()
    conn, addr = s.accept()

    while True:
        test = conn.recv(1024)
        print(str(test))

if __name__ == '__main__':
    receive()

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
