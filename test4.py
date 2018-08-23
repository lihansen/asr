from ctypes import *

lib = CDLL("main.so")

stream = b"".join([chr(i).encode() for i in range(32,127)])
stream *= 30000
print(len(stream))
print(127-32)



a= c_char*(2048*6)
pa = a()
b = c_char*(2048*6)
pb = b()
flag = c_int(8)
flag_p = pointer(flag)

for i in range(90):
    # lib.get_channel2(stream[4096*i:4096*(i+1)],pa,pb)
    lib.get_flag(stream[4096*i:4096*(i+1)],pa,pb,flag_p)
    print(i)
    print("a",len(pa.value),pa.value)
    print("b",len(pb.value),pb.value)
    print(flag.value,"\n")