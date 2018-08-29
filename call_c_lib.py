import os
import pyaudio
import socket
from ctypes import *
from threading import Thread,Condition


class decoder:

    def __init__(self, config_filename="config", so_path="libsyapi.so"):
        # self.lib2 = CDLL(os.path.join(".","libdecoder.so"))
        self.lib = CDLL(os.path.join(".", so_path))

        self.mdl_p = c_void_p()
        self.obj_p = c_void_p()

        self.sy_init(config_filename)


    def sy_init(self, config_filename):
        state = self.lib.sy_init(byref(self.mdl_p), c_char_p(config_filename.encode("utf-8")))
        if (state == -1):
            raise Exception("init error")

    def sy_create(self):
        state = self.lib.sy_create(self.mdl_p, byref(self.obj_p))
        if state == -1:
            raise Exception("create error")

    def sy_put_data(self, data, len, chunk_no):
        if not isinstance(data, bytes):
            raise TypeError("must be 'bytes'!")

        if len < 0 and len / 2 != 0:
            raise Exception("len must > 0 and even num")

        state = self.lib.sy_put_data(self.obj_p,
                                     data,
                                     c_int(len),
                                     c_int(chunk_no))
        if state is -1:
            raise Exception("feature error")
        elif state is -2:
            raise Exception("nn error")
        elif state is -3:
            raise Exception("FST error")

    def sy_get_result(self, max_result_length=1024):
        result = c_char * max_result_length
        result_p = result()
        state = self.lib.sy_get_result(self.obj_p, result_p, c_int(max_result_length))
        if state is -1:
            raise Exception("result write error")

        return result_p[:]

    def sy_finish(self):
        if self.lib.sy_finish(self.obj_p) is -1:
            raise Exception("finish error")

    def sy_release(self):
        if self.lib.sy_release(self.mdl_p) is -1:
            raise Exception("release error")



class Server:
    def __init__(self):
        self.d = decoder()

        self.serv_sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        self.host = "192.168.0.164"
        self.port = 9002
        self.serv_sock.bind((self.host,self.port))
        self.serv_sock.listen(8)

    def run_server(self):
        while True:
            s, addr = self.serv_sock.accept()
            print(addr," connect")
            # con = Condition()
            Thread(target=self.send_ret, args=(s, )).start()
            # Thread(target=self._recv_send2, args=(s, con)).start()

    def send_ret(self,s):
        # self.d.sy_create()
        package_no = 1
        while True:
            data = s.recv(2048*6)
            print(data)
            self.d.sy_put_data(data,len(data),package_no)
            ret = self.d.sy_get_result()
            print(ret)
            s.send(b" "+ret)
            # s.send(data[:20])
            package_no += 1



class client:
    def __init__(self, n_frames=1024*6):
        self.n_frame = n_frames
        # self.d = decoder("config")
        self._pyaudio = pyaudio.PyAudio()
        self._stream = self._pyaudio.open(
            format=pyaudio.paInt16,  # 2 bytes pre frame
            channels=2,
            rate=16000,
            input=True,
            frames_per_buffer=1024)
        print("microphone start working:")


    def run(self,host="192.168.0.164",post=9002):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((host, post))
        while True:
            data = self._stream.read(self.n_frame)
            sock.send(data)
            print("ret:",sock.recv(1024))
            # print(sock.recv(1024))


    def start(self):
        Thread(target=self.run).start()


# Server().run_server()
client().start()