import socket
import sys
import time
from struct import *
import os

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

clear = lambda: os.system('cls')
server_address = ('192.168.1.107', 4210)
# Note to self 
# Ghulam Mujtaba 16/4/2018 (Experimentation of UDP communication to ESP32 board over ethernet)
# If message string length is 42, the free heap size in the ESP32 board starts )
# decreasing and eventualy crashes the application, only happens at message size of 42 byte for example
# b'This is the message.  It will be repeated.'
# causes a crash on the ESP32 end.
loop=1
var1=0
cnt=0
message1 = pack('BBBBBBBBBBBBBBBBBBBBBB', 0x7D, 0xD7, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFA)
message11 = pack('BBBBBBBBBBBBBBBBBBBBBB', 0x7D, 0xD7, 0x01, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFA)
message2 = pack('BBBBBBB', 0x7D, 0xD7, 0x02, 0x02, 0x90, 0xFF, 0xFA)
message3 = pack('BBBBBBB', 0x7D, 0xD7, 0x03, 0x04, 0x90, 0xFF, 0xFA)
message4 = pack('BBBBBBB', 0x7D, 0xD7, 0x03, 0x04, 0x91, 0xFF, 0xFA)
try:
    while True:
        num = int(input("Enter function:\n1. Configure\n2. Read\n3. Write\n"))
        if num==1:
            sent = sock.sendto(message11 , server_address)
            print('sending {!r}'.format(message11))
            #time.sleep(1);
            #Receive Port A response
            print('waiting to receive')
            data, server = sock.recvfrom(4096)
            print('received {!r}'.format(data))
            #time.sleep(1);
        elif num==2:
            while loop == 1:
                sent = sock.sendto(message2 , server_address)
                data0, server = sock.recvfrom(4096)
                data1, server = sock.recvfrom(4096)
                data2, server = sock.recvfrom(4096)
                data3, server = sock.recvfrom(4096)
                data4, server = sock.recvfrom(4096)
                data5, server = sock.recvfrom(4096)
                data6, server = sock.recvfrom(4096)
                data7, server = sock.recvfrom(4096)
                if var1 != data1:
                    print ("\n" * 100)
                    print('received {!r}'.format(data0))
                    print('received {!r}'.format(data1))
                    print('received {!r}'.format(data2))
                    print('received {!r}'.format(data3))
                    print('received {!r}'.format(data4))
                    print('received {!r}'.format(data5))
                    print('received {!r}'.format(data6))
                    print('received {!r}'.format(data7))
                var1=data1
                
        elif num==3:
            while True:
                while cnt<5:
                    sent = sock.sendto(message3 , server_address)
                    cnt=cnt+1
                    sent = sock.sendto(message2 , server_address)
                    data0, server = sock.recvfrom(4096)
                    data1, server = sock.recvfrom(4096)
                    data2, server = sock.recvfrom(4096)
                    data3, server = sock.recvfrom(4096)
                    data4, server = sock.recvfrom(4096)
                    data5, server = sock.recvfrom(4096)
                    data6, server = sock.recvfrom(4096)
                    data7, server = sock.recvfrom(4096)
                    if var1 != data1:
                        print ("\n" * 100)
                        print('received {!r}'.format(data0))
                        print('received {!r}'.format(data1))
                        print('received {!r}'.format(data2))
                        print('received {!r}'.format(data3))
                        print('received {!r}'.format(data4))
                        print('received {!r}'.format(data5))
                        print('received {!r}'.format(data6))
                        print('received {!r}'.format(data7))
                    var1=data1
                    #time.sleep(0.001);
                cnt=0
                while cnt<5:
                    sent = sock.sendto(message4 , server_address)
                    cnt=cnt+1
                    sent = sock.sendto(message2 , server_address)
                    data0, server = sock.recvfrom(4096)
                    data1, server = sock.recvfrom(4096)
                    data2, server = sock.recvfrom(4096)
                    data3, server = sock.recvfrom(4096)
                    data4, server = sock.recvfrom(4096)
                    data5, server = sock.recvfrom(4096)
                    data6, server = sock.recvfrom(4096)
                    data7, server = sock.recvfrom(4096)
                    if var1 != data1:
                        print ("\n" * 100)
                        print('received {!r}'.format(data0))
                        print('received {!r}'.format(data1))
                        print('received {!r}'.format(data2))
                        print('received {!r}'.format(data3))
                        print('received {!r}'.format(data4))
                        print('received {!r}'.format(data5))
                        print('received {!r}'.format(data6))
                        print('received {!r}'.format(data7))
                    var1=data1
                    #time.sleep(0.001);
                cnt=0
                
        
finally:
    print('closing socket')
    sock.close()
