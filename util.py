import queue
import pyaudio
import threading
import socket
import struct
import wave
import time

from ctypes import *
from multiprocessing import Process



class recorder:

    def __init__(self,n_frames=1024):
        self._out1 = c_char * (2048*6)
        self._pout1 = self._out1()

        self._out2 = c_char * (2048*6)
        self._pout2 = self._out2()

        self._flag = c_int(11)
        self._pflag = pointer(self._flag)

        self._out_flag = c_int(22)
        self._pout_flag = pointer(self._out_flag)

        self._number_flagch = c_int(33)
        self._pnumber_flagch = pointer(self._number_flagch)

        self._n_frames = n_frames
        self._stop = False

        self._lib = CDLL("./main.so")
        self._lib.init()

        self._pyaudio = pyaudio.PyAudio()
        self._stream = self._pyaudio.open(
            format=pyaudio.paInt16,  # 2 bytes pre frame
            channels=2,
            rate=16000,
            input=True,
            frames_per_buffer=n_frames)
        print("microphone start working:")


    def _running(self,q1,q2):
        print("channel-selector start")
        while not self._stop:
            chunk = self._stream.read(self._n_frames)
            self._lib.get_flag(chunk,self._pout1,self._pout2,self._pflag,self._pout_flag,self._pnumber_flagch)

            if self._out_flag.value is 1 and self._number_flagch.value >= 16 and (self._number_flagch.value-16)%6 is 0:
                print("flag:",self._flag.value,)
                if self._flag.value is 0 or self._flag.value is 3:
                    q1.put(self._pout1[:])
                    q2.put(self._pout2[:])
                if self._flag.value is 1:
                    q1.put(self._pout1[:])
                elif self._flag.value is 2:
                    q2.put(self._pout2[:])
            elif self._out_flag.value == 0:
                print("噪声检测")


    def start(self,q1,q2):
        threading.Thread(target=self._running,args=(q1,q2),name="record").start()
        # Process(target=self._running,args=(q1,q2),name="record").start()


    def stop(self):

        self._stream.stop_stream()
        self._stream.close()
        self._stop = True
        self._pyaudio.terminate()
        # self._thread.join()
        print("recorder", " stopped")



class wav_writer:
    def __init__(self,seconds,filename,n_frames=1024):
        self._time = int(16000 / n_frames * seconds)
        self._filename = filename
        self._data = []
        self._stop = False

    def _running(self,q):
        while self._time and not self._stop:
            print(self._filename,self._time)
            chunk = q.get()
            self._data.append(chunk)
            self._time -= 1
        self.save()

    def start(self,q):
        threading.Thread(target=self._running,args=(q,)).start()

    def save(self):
        wf = wave.open(self._filename + ".wav","wb")
        wf.setnchannels(1)
        wf.setsampwidth(pyaudio.PyAudio().get_sample_size(pyaudio.paInt16))
        wf.setframerate(16000)
        wf.writeframes(b"".join(self._data))
        wf.close()
        print(self._filename,".wav 录音结束")

    def stop(self):
        self._stop = True



class sender:

    def __init__(self,save=True,HOST="192.168.0.164",PORT=9002 or 9000):
        self._host = HOST
        self._port = PORT
        self._connect()

        self._alive_times = 20

        self._save = save
        self._ret = ""
        self._name = ""
        self._stop = False

    def _connect(self):
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.connect((self._host, self._port))
        self._decoder = decoder()
        self._decoder.decoder_init(self._sock)
        # print("socket connect successful")


    def _running(self, q):
        package_no = 1
        while not self._stop :

            chunk = q.get() # block method,thread will be blocked when queue is empty
            self._decoder.decoder_put_data(self._sock,chunk,2048*6,package_no)
            package_no += 1
            ret = self._decoder.decoder_get_result(self._sock)
            self._ret = self._decoder.to_str(ret)
            print(self._name,"-ret:",self._ret)
            self._alive_times -= 1

            if not self._alive_times:
                self._sock.close()
                self._connect()
                package_no = 1
                self._alive_times = 10



    def start(self,q,name):
        self._name = name
        threading.Thread(target=self._running,args=(q,)).start()
        # Process(target=self._running(q,)).start()


    def stop(self):
        self._stop = True
        self._decoder.decoder_release(self._sock)
        self._sock.close()

        if self._save:
            with open(self._name+".txt","w+") as f:
                f.write(self._ret)
        print(self._name," stopped")


class send_to_middleware:

    def __init__(self,host="192.168.0.163",port=9999):
        self.sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        self.sock.connect((host,port))
        print("connect to middleware successful")
        self._stop = False


    def _running(self,q):

        while not self._stop:
            d = q.get()
            self.sock.send(d)
            ret = self.sock.recv(2048)
            ret = ret.decode("utf-8")
            print(self._name,ret)

    def start(self,q,name):
        self._name = name
        threading.Thread(target=self._running,args=(q,)).start()



class decoder():

    def get_option(self,config, section, option):
        if not config.has_section(section) or not config.has_option(section, option):
            return None
        return config.get(section, option)


    def recv_int(self,sock):
        buf = sock.recv(4)
        while (len(buf) < 4):
            buf += sock.recv(4 - len(buf))
        return struct.unpack('!i', buf[:4])[0]


    def decoder_init(self,sock):
        sock.sendall(struct.pack('!i', 1))
        return self.recv_int(sock)


    def decoder_put_data(self,sock, data, length, chunk_no):
        buf = struct.pack('!i', 2)
        buf += struct.pack('!i', length)
        buf += data
        buf += struct.pack('!i', chunk_no)
        sock.sendall(buf)
        return self.recv_int(sock)


    def decoder_get_result(self,sock):
        sock.sendall(struct.pack('!i', 3))
        res_len = self.recv_int(sock)
        if (res_len == 0):
            return b""
        else:
            buf = sock.recv(res_len)
            while (len(buf) < res_len):
                buf += sock.recv(res_len - len(buf))
            return buf


    def decoder_release(self,sock):
        sock.sendall(struct.pack('!i', 4))
        return self.recv_int(sock)


    def to_str(self,bytes_or_str):
        if isinstance(bytes_or_str, bytes):
            s = bytes_or_str.decode("utf-8")
        else:
            s = bytes_or_str
        return s

if __name__ == "__main__":
    left = queue.Queue()
    right = queue.Queue()

    rec = recorder()
    rec.start(left,right)

    s1 = send_to_middleware()
    s1.start(left,"通道1")
    s2 = send_to_middleware()
    s2.start(right,"通道2")



    input("press enter to exit")
    rec.stop()
    s1.stop()
    s2.stop()



# from channel_test import file_writer
# writer1_thead = threading.Thread(target=file_writer,args=(left,),name="1")
# writer2_thead = threading.Thread(target=file_writer,args=(left,),name="2")
#
# writer1_thead.start()
# writer2_thead.start()

