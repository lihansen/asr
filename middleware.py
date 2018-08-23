import socket
import time
from util import decoder
from threading import Thread,Condition

class Server():

    def __init__(self):
        self.serv_sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        self.host = "192.168.0.163"
        self.port = 9999
        self.serv_sock.bind((self.host,self.port))
        self.serv_sock.listen(8)

        self.stop = False


    def run_server(self):
        print("server is running")
        while True:
            s, addr = self.serv_sock.accept()
            print(addr," connect")
            con = Condition()
            Thread(target=self._recv_send2,args=(s,con)).start()
            Thread(target=self._recv_send2,args=(s,con)).start()




    # only receive 2048*6 size data chunk api
    def _recv_send(self,s,host="192.168.0.164",post=9002 or 9000,chunk_size=2048*6):

        # asr server socket
        send_sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        send_sock.connect((host,post))
        d = decoder()
        d.decoder_init(send_sock)

        package_no = 1
        while not self.stop:
            # get data chunk from client
            data = s.recv(2048*6)
            data = self._process(data)
            # send data chunk to asr sever
            d.decoder_put_data(send_sock,data,chunk_size,package_no)
            ret = d.decoder_get_result(send_sock)
            # return result to client
            s.send(b" "+ret)
            ret = ret.decode("utf-8")
            print("ret",ret)
            package_no += 1


    # received data chunk length <= 2048*10 api
    def _recv_send2(self,client_s,con,host="192.168.0.164",post=9002 or 9000,chunk_size=2048*6):

        while True:

            # asr server socket
            send_sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
            send_sock.connect((host,post))
            d = decoder()
            d.decoder_init(send_sock)

            package_no = 1
            buffer = b""
            last_ret = b""
            same_times = 0

            con.acquire()

            while not self.stop and same_times < 4:
                data = client_s.recv(2048*10)
                # data = self._process(data)
                buffer += data

                if len(buffer) >= chunk_size:
                    d.decoder_put_data(send_sock,buffer[:chunk_size],chunk_size,package_no)
                    ret = d.decoder_get_result(send_sock)
                    buffer = buffer[chunk_size:]

                    if last_ret == ret and ret != b"":
                        same_times +=1
                    else :
                        last_ret=ret

                    package_no += 1
                    client_s.send(b" " + ret)

            # print("ret", ret)

            send_sock.close()

            con.notify()
            con.wait()
            # con.release()




    def _process(self,data):
        # print("len ",len(data),"data ",data[:30])
        return data


    def _to_str(self,bytes_or_str):
        if isinstance(bytes_or_str, bytes):
            s = bytes_or_str.decode("utf-8")
        else:
            s = bytes_or_str
        return s




class client:
    def __init__(self,host="192.168.0.163",port=9999):
        self.host = host
        self.port = port
        self._s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._s.connect((self.host,self.port))


    def send(self,data,package_no,size=2048*6):
        # self._decoder.decoder_put_data(self._s,data_chunk,size,package_no)
        data = bytes(package_no) + data
        self._s.send(data)


    def result(self):
        b_result = self._decoder.decoder_get_result(self._s)
        return self._to_str(b_result)




Server().run_server()




