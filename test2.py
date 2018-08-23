from ctypes import *
from random import randint
# gcc filename.c -share -o filename.so
# mylib = CDLL("func_lib.so")

# array_obj = c_int * 3
# c_array = array_obj(888,777,666)
# a = c_int(9)
# pa = pointer(a)
# mylib.change(pa,c_array)
# print(a.value,c_array[2])

# mylib = CDLL("/home/lihansen/pyproject/test/asr/main.so")
mylib = cdll.LoadLibrary('/home/lihansen/pyproject/test/asr/main.so')



in1 = c_char*1024
in2 = c_short*1024


out1 = c_short * (1024 * 6)
out2 = c_short * (1024 * 6)
c_out1 = out1(*[66 for i in range(1024*6)])
c_out2 = out2(*[66 for i in range(1024*6)])


flag = c_int(1)
c_p_flag = pointer(flag)

mylib.init()

call_time = 0
for i in range(100):

    # c_in1 = in1(*[randint(1, 5) for i in range(1024)])
    c_in1 = in1(b"a",b"\0",b"\0",b"9")
    c_in2 = in2(*[randint(1, 9) for i in range(1024)])
    mylib.get_flag2(c_in1,c_in2,c_out1,c_out2,c_p_flag)
    if call_time == 5:
        call_time = 0
        print("flag   ",flag.value)
        print("python_in1     ",len(c_in1),c_in1[:])
        print("python_in2     ",len(c_in2))
        print("python_out1    ",len(c_out1))
        print("python_out2    ",len(c_out2))
        # print(type(c_out2))
        print()
    else:
        call_time+=1