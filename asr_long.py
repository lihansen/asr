#coding:utf-8
# ubuntu python 3.6
#实时连续asr 客户端，双声道可选择

import socket
import struct
import time
import queue
import threading
import pyaudio

## socket 连接api
def get_option(config, section, option):
    if not config.has_section(section) or not config.has_option(section, option):
        return None
    return config.get(section, option)

# def recv_int(sock):
#     buf = s.recv(4)
#     while (len(buf) < 4):
#         buf += s.recv(4-len(buf))
#     return struct.unpack('!i', buf[:4])[0]
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
    #buf += struct.pack('!10s', "jida")
    buf += struct.pack('!i', length)
    buf += data
    buf += struct.pack('!i', chunk_no)
    sock.sendall(buf)
    return recv_int(sock)

def decoder_get_result(sock):
    sock.sendall(struct.pack('!i', 3))
    res_len = recv_int(sock)
    if (res_len == 0):
        return ""
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

# if __name__ =="__main__":
#连接到socket服务器
# HOST = "srv2.freeneb.com" or "192.168.0.66" or "192.168.0.164"
HOST = "192.168.0.164"
PORT = 9002 or 9000
#
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
print("Client connect successful")

decoder_init(s)
time.sleep(1)

q = queue.Queue()

def sender(q, s):
    package_no = 1
    while True:
        if q.empty():
            continue
        else:
            d = q.get()
            ret = decoder_put_data(s, d, 1024*6*2,package_no)#字节数
            package_no += 1
            ret = decoder_get_result(s)
            # th_name = threading.current_thread().getName()
            print(to_str(ret))


def receiver(s):
    before = ""
    while True:
        result= decoder_get_result(s)
        retsult = to_str(result)
        if before == result :
            pass
        else :
            print(retsult)
            before=retsult

#将双声道字节数据拆分
def get_channel(data):
    c1 = b''
    c2 = b''
    p = 0
    length = len(data)
    while p+4<=length:
        c1 += data[p:p+2]
        c2 += data[p+2:p+4]
        p += 4
    # print(len(c1),len(c2))
    return (c1,c2)



def record(q):
    stream = pyaudio.PyAudio().open(format=pyaudio.paInt16,#数据格式，每帧占用两个字节
                    channels=2,#双声道
                    rate=16000,
                    input=True,
                    frames_per_buffer=1024*6)#每次读取的帧数，
    print("开始录音")

    while True:
        data = stream.read(1024*6)#读取的帧数
        q.put(get_channel(data)[1])#读取的第二个声道

def multi_sender(num=3):
    HOST = "srv2.freeneb.com" or "192.168.0.66"
    PORT = 9002 or 9000
    for i in range(num):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((HOST, PORT))
        send_thead = threading.Thread(target=sender, args=(q, sock), name="sender"+"i")
        send_thead.start()


record_thread = threading.Thread(target=record,args=(q,),name="record")
send_thead = threading.Thread(target=sender,args=(q,s),name="sender")

record_thread.start()
send_thead.start()
# multi_sender()


# receive_thead = threading.Thread(target=receiver,args=(s,),name="receiver")
# receive_thead.start()

# # send_thead = Process(target=sender,args=(q,s),name="sender")
# # receive_thead = Process(target=receiver,args=(s,),name="receiver")
# # record_thread = Process(target=record,args=(q,),name="record")
#

