#coding:utf-8
# ubuntu python 3.6
#实时连续asr 客户端，双声道可选择

import socket
import struct
import queue
import threading
import pyaudio
from ctypes import *
import wave
from datetime import datetime

## socket 连接api

def get_option(config, section, option):
    if not config.has_section(section) or not config.has_option(section, option):
        return None
    return config.get(section, option)
def recv_int(sock):
    buf = sock.recv(4)
    while (len(buf) < 4):
        buf += sock.recv(4-len(buf))
    return struct.unpack('!i', buf[:4])[0]
def decoder_init(sock):
    sock.sendall(struct.pack('!i', 1))
    return recv_int(sock)

def decoder_put_data(sock, data, length, chunk_no):
    buf = struct.pack('!i', 2)
    buf += struct.pack('!i', length)
    buf += data
    buf += struct.pack('!i', chunk_no)
    sock.sendall(buf)
    return recv_int(sock)

def decoder_get_result(sock):
    sock.sendall(struct.pack('!i', 3))
    res_len = recv_int(sock)
    if (res_len == 0):
        return b""
    else:
        buf = sock.recv(res_len)
        while (len(buf) < res_len):
            buf += sock.recv(res_len - len(buf))
        return buf

def decoder_release(sock):
    sock.sendall(struct.pack('!i', 4))
    return recv_int(sock)

def to_str(bytes_or_str):
    if isinstance(bytes_or_str,bytes):
        s = bytes_or_str.decode("utf-8")
    else :
        s = bytes_or_str
    return s



lib = CDLL("./main.so")
lib.init()

q1 = queue.Queue()
q2 = queue.Queue()

frame_num = 1024 #4096 bytes

flag = c_int(8)
out_flag = c_int(9)
number_flagch = c_int(7)
flag_p = pointer(flag)
out_flag_p = pointer(out_flag)
number_flagch_p = pointer(number_flagch)

call_times_noise = 0
call_times = 0
temp  = 0

frame1 = []
frame2 = []
stop =False
n =0



def channel_selector(q1,q2,out1,out2,data):
    lib.get_flag(data,out1,out2,flag_p)

    global call_times,call_times_noise
    if call_times_noise <69:
        call_times_noise += 0
        return
    if call_times is 5:
        call_times=0
        if flag.value is 0:
            q1.put(out1[:])
            q2.put(out2[:])
        elif flag.value is 1:
            q1.put(out1[:])
        elif flag.value is 2:
            q2.put(out2[:])
        elif flag.value is 3:
            print("3")
    else:
        call_times+=1


def record(q1,q2):
    stream = pyaudio.PyAudio().open(format=pyaudio.paInt16,#数据格式，每帧占用两个字节
                    channels=2,#双声道
                    rate=16000,
                    input=True,
                    frames_per_buffer=frame_num)#每次读取的帧数，
    print("开始录音")

    a = c_char * (2048*6)
    b = c_char * (2048*6)
    pa = a()
    pb = b()

    # a1 = c_char * (2048*6)
    # b1 = c_char * (2048*6)
    # pa1 = a1()
    # pb1 = b1()

    while True:
        data = stream.read(frame_num)#读取的帧数

        # lib.get_channel2(data,pa,pb)
        lib.get_flag(data, pa, pb, flag_p,out_flag_p,number_flagch_p)

        # print("flag",flag.value)

        if out_flag.value is 1 and number_flagch.value >= 16 and (number_flagch.value-16)%6 is 0:
            flag.value=0
            if flag.value is 0:
                q1.put(pa[:])
                q2.put(pb[:])
            elif flag.value is 1:
                q1.put(pa[:])
            elif flag.value is 2:
                q2.put(pb[:])
            elif flag.value is 3:
                print("3")
        elif out_flag.value ==0:
            print("噪声检测")

        # q1.put(pa1[:])
        # q2.put(pb1[:])

        # left,right = get_channel(data)
        # lib.get_flag2(left,right, pa, pb, flag_p)

        # continue

        # print("flag",flag.value)
        # flag.value = 0
        # global call_times,call_times_noise
        # if call_times_noise < 69:
        #     call_times_noise += 1
        #     print(69-call_times_noise," 噪声检测")
        #     continue
        # # flag.value = 0
        # if call_times is 5:
        #     call_times = 0
        #     if flag.value is 0:
        #         q1.put(pa[:])
        #         q2.put(pb[:])
        #     elif flag.value is 1:
        #         q1.put(pa[:])
        #     elif flag.value is 2:
        #         q2.put(pb[:])
        #     elif flag.value is 3:
        #         print("3")
        # else:
        #     call_times += 1
    # write(frame1,"channel1")
    # write(frame2,"channel2")

def  sender1(q):
    HOST = "192.168.0.164"
    PORT = 9002 or 9000
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, PORT))
    decoder_init(s)

    th_name = threading.current_thread().getName()
    print(th_name, ":socket connect successful")
    package_no = 1

    while True:
        # if q.empty():
        #     continue
        # else:
        d = q.get()

        decoder_put_data(s, d[:], 1024 * 2 * 6, package_no)  # 字节数
        package_no += 1
        ret = decoder_get_result(s)
        print(th_name, "-ret:", to_str(ret))

def sender(q):
    HOST =  "192.168.0.164"
    PORT = 9002 or 9000
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, PORT))
    decoder_init(s)

    th_name = threading.current_thread().getName()
    print(th_name,":socket connect successful")
    package_no = 1

    global n
    n +=1
    while package_no < 4:
        if q.empty():
            continue
        else:
            d = q.get()

            decoder_put_data(s, d[:], 1024*2*6,package_no)#字节数
            package_no += 1
            ret = decoder_get_result(s)
            print(th_name,"-ret:",to_str(ret))
    n -= 1
    print("die")

def write(frames,name):
    wf = wave.open(str(name)+".wav", 'wb')
    wf.setnchannels(1)
    p = pyaudio.PyAudio()
    wf.setsampwidth(p.get_sample_size(pyaudio.paInt16))
    wf.setframerate(16000)
    wf.writeframes(b''.join(frames))
    wf.close()
    print(name, "录音结束")


def file_writer(q):
    frames = []
    filename = threading.current_thread().getName()
    name = "channel"+filename
    times = int(16000 / frame_num * 2)
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

def get_channel(data):
    c1 = b''
    c2 = b''
    p = 0
    length = len(data)
    while p+4<=length:
        c1 += data[p:p+2]
        c2 += data[p+2:p+4]
        p += 4
    return (c1,c2)

def sender_offline(q):
    th_name = threading.current_thread().getName()
    while True:
        if q.empty():
            continue
        else:
            d = q.get()
            print(th_name,"  -",d.value)


record_thread = threading.Thread(target=record,args=(q1,q2),name="record")
send1_thead = threading.Thread(target=sender,args=(q1,),name="channel2")
send2_thead = threading.Thread(target=sender,args=(q2,),name="channel1")
writer1_thead = threading.Thread(target=file_writer,args=(q1,),name="1")
writer2_thead = threading.Thread(target=file_writer,args=(q2,),name="2")
from util import recorder
recorder().start(q1,q2)
# record_thread.start()
# while True:
#     if n == 2:
#         continue
#     else:
#         print("start")
#         send2_thead.start()
#         # send1_thead.start()
writer1_thead.start()
writer2_thead.start()
