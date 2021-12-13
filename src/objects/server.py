import gzip
import json
import re
import select
import socket
import sys
import time
from datetime import datetime
from queue import Queue


HEADER_LENGTH = 15


class Server:
    def __init__(self, port, host):
        self.port = port
        self.host = host
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.inputs = []
        self.outputs = []
        self.connections = 0


    def start(self):
        self.__bind_socket()


    def __bind_socket(self):
        try:
            self.s.bind((self.host, self.port))
            self.s.setblocking(1)
            self.s
            print("Port successfully binded!")
            self.s.listen(socket.SOMAXCONN)
            print("Listening on port: " + str(self.port))
            self.inputs.append(self.s)
        except socket.error as msg:
            if msg.errno != 10022:
                print("Failed to bind socket: " + str(msg))


    def send(self, task_list, enqueued_list):
        infds, outfds, _ = select.select(self.inputs, self.outputs, [], 5)
        for fds in infds:
            if fds is self.s:
                (client_sock, client_addr) = fds.accept()
                client_sock.setblocking(1)
                self.outputs.append(client_sock)
                self.connections += 1
                print("Connection established with " + str(client_addr))

        for fds in outfds:
            if len(task_list) > 0 and fds is not self.s:
                task = task_list[0]
                msg = str.encode(json.dumps(task.__dict__))
                header = str.encode(str(len(msg)).zfill(HEADER_LENGTH))
                fds.send(header)
                fds.send(msg)
                enqueued_list.append((task, datetime.now()))
                del task_list[0]
                self.inputs.append(fds)
                self.outputs.remove(fds)


    def receive(self, received_list):
        infds, _, _ = select.select(self.inputs, [], [], 5)
        for fds in infds:
            if fds is not self.s:
                msg_size = int(re.search(r'\d+', fds.recv(HEADER_LENGTH).decode("utf-8")).group())
                # msg_size = int(fds.recv(HEADER_LENGTH).decode("utf-8"))

                start = time.time()

                msg = b''
                while len(msg) != msg_size:
                    # msg += fds.recv(msg_size).decode("utf-8")
                    msg += fds.recv(msg_size)

                end = time.time()

                # size = (msg_size/pow(1024,2))*8
                # rate = size/(end-start)
                # print('Received {:,} bytes at {:f}Mbps'.format(msg_size, rate))

                if len(msg) > 0:
                    if fds not in self.outputs:
                        self.outputs.append(fds)
                    self.inputs.remove(fds)
                    received_list.append(gzip.decompress(msg))
                    # received_list.append(msg)
                else:
                    if fds in self.outputs:
                        self.outputs.remove(fds)
                    self.inputs.remove(fds)
                    fds.close()
                    self.connections -= 1


    def shutdown(self):
        self.s.close()
