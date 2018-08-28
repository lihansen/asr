from wsgiref.simple_server import make_server
import os

class http_server():

    def __init__(self):
        self.refresh = False
        with open(os.path.join("home.html"),"rb") as f:
            self.html = f.read()


    def start(self):
        make_server('', 8000, self._home_page).serve_forever()


    def _home_page(self,environ, start_response):
        start_response("200 OK", [('Content-Type', 'text/html')])
        url = environ['PATH_INFO'][1:]
        if url =="flag":
            if self.refresh is True:
                self.refresh = False
                return [self.flag]
            else:
                return [b"none"]
        if url =="channel1" :
            return [self.channel1]
        if url =="channel2":
            return [self.channel2]

        return [self.html]


    def set_channel1(self,text,encoding="gbk"):
        self.channel1=text.encode(encoding)


    def set_channel2(self, text,encoding="gbk"):
        self.channel2 = text.encode(encoding)


    def set_flag(self, text,encoding="gbk"):
        self.flag = text.encode(encoding)
        self.refresh = True



hs = http_server()
# hs.set_flag("不知道")
# hs.set_channel1("通道1")
# hs.set_channel2("通道2")
hs.start()

