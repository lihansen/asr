import os
import pyaudio
from ctypes import *
from threading import Thread

class decoder:

    def __init__(self,config_filename=None,so_path="libsyapi.so"):
        self.lib = CDLL(os.path.join(".",so_path))

        decoder_mdl = c_int(None)
        self.mdl_p = pointer(decoder_mdl)
        self.mdl_p_p = pointer(self.mdl_p)

        decoder_obj = c_int(None)
        self.obj_p = pointer(decoder_obj)
        self.obj_p_p = pointer(self.obj_p_p)

        if config_filename:

            self.sy_init(config_filename)
            self.sy_create()



    def sy_init(self,config_filename):
        state =  self.lib.sy_init(self.mdl_p_p,c_char_p(config_filename))
        if state == -1:
            raise Exception("init error")


    def sy_create(self):
        state = self.lib.sy_create(self.mdl_p_p,self.obj_p_p)
        if state == -1:
            raise Exception("create error")


    def sy_put_data(self,data,len,chunk_no):
        if not isinstance(data,bytes):
            raise TypeError("must be 'bytes'!")

        if len < 0 and len/2 != 0:
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


    def sy_get_result(self,max_result_length=1024):
        retult = c_char * max_result_length
        result_p = retult()
        state = self.lib.sy_get_result(self.obj_p,result_p,c_int(max_result_length))
        if state is -1:
            raise Exception("result write error")

        return result_p[:].decode("utf-8")


    def sy_finish(self):
        if self.lib.sy_finish(self.obj_p) is -1:
            raise Exception("finish error")


    def sy_release(self):
        if self.lib.sy_release(self.mdl_p) is -1:
            raise Exception("release error")


class recorder:
    def __init__(self,n_frames=1024):
        self.n_frame = n_frames
        self.d = decoder("config")

        self._pyaudio = pyaudio.PyAudio()
        self._stream = self._pyaudio.open(
            format=pyaudio.paInt16,  # 2 bytes pre frame
            channels=2,
            rate=16000,
            input=True,
            frames_per_buffer=n_frames)
        print("microphone start working:")

    def run(self):
        package_no = 1
        while True:
            data = self._stream.read(self.n_frame)
            self.d.sy_put_data(data,1024,package_no)
            print("ret",self.d.sy_get_result())
            package_no += 1

    def start(self):
        Thread(target=self.run).start()


recorder().start()




