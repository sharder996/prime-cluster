#!/usr/bin/python3

import socket
import sys
import time


HEADER_LENGTH = 15


class Client:
    def __init__(self, port, host):
        self.port = port
        self.host = host
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)


    def start(self):
        try:
            self.s.connect((self.host, self.port))
            self.s.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
            print("Successfully connected to server")
        except:
            print("Error connecting to server. Retrying...")
            time.sleep(5)
            self.start()


    def send(self, payload):
        msg = str.encode(payload)
        header = str.encode(str(len(msg)).zfill(HEADER_LENGTH))
        self.s.sendall(header)
        self.s.sendall(str.encode(payload))


    def receive(self):
        msg = self.s.recv(1024).decode("utf-8")
        return msg


    def shutdown(self):
        self.s.shutdown(socket.SHUT_RDWR)
        self.s.close()
