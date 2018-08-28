import threading
import pyaudio
import wave
import queue
from ctypes import *


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
                # print("flag:",self._flag.value,)
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



def file_writer(q):
    frames = []
    filename = threading.current_thread().getName()
    name = "channel"+filename
    times = int(16000 / 1024 * 2)
    while times:
        if q.empty():
            continue
        else:
            d = q.get()

            print(name,"正在录音")
            frames.append(d)
            times -= 1

    wf = wave.open(filename+".wav", 'wb')
    wf.setnchannels(1)
    p = pyaudio.PyAudio()
    wf.setsampwidth(p.get_sample_size(pyaudio.paInt16))
    wf.setframerate(16000)
    wf.writeframes(b''.join(frames))
    wf.close()
    print(name, "录音结束")



q1 = queue.Queue()
q2 = queue.Queue()
writer1_thead = threading.Thread(target=file_writer,args=(q1,),name="1")
writer2_thead = threading.Thread(target=file_writer,args=(q2,),name="2")
recorder().start(q1,q2)
writer1_thead.start()
writer2_thead.start()
