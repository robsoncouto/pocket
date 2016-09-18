
import serial,time #You need the pyserial library
import struct

ser = serial.Serial("/dev/ttyUSB0", 115200, timeout=0)
#time.sleep(10);#my arduino bugs if data is written to the port after opening it
#filename='sonic.bin'#name of the rom, bin format
#f=open(name,'rb');
#with open(filename,'rb') as f:
romsize=1024

while True:
    print("===========================================")
    print("           What do you want do do?         ")
    print("                                           ")
    print("              1-dump                       ")
    print("              2-burn                       ")
    print("              3-info                       ")
    print("                                           ")
    print("===========================================")
    option=int(input())
    romsize=128*1024
    ramsize=128*1024
    block=0
    print(option)
    if(option==1):
        name=input("What the name of the file?")
        
        f = open(name, 'w')
        f.close()
        ser.flushInput()
        ser.write(bytes("R","ASCII"))
        numBytes=0
        f = open(name, 'ab')#yes, that simple
        while (numBytes<romsize):
            while ser.inWaiting()==0:
                print("waiting...\n",numBytes)
                time.sleep(0.1)
            data = ser.read(1)#must read the bytes and put in a file
            f.write(data)
            numBytes=numBytes+1
            #if(numBytes%8192==0):
            #    //block=block+1
            #    f.close()
            #    f = open(name+str(block)+".bin", 'ab')        
        f.close()
    if(option==2):
        name=input("What the name of the file?")
        print(name)
        f = open(name, 'rb')
        
        for i in range(1024):
            ser.write(bytes("W","ASCII" ))
            time.sleep(0.001)
            ser.write(struct.pack(">B",i>>8))
            CHK=i>>8
            #CHK=ord(CHK)
            time.sleep(0.001)
            ser.write(struct.pack(">B",i&0xFF))
            CHK^=i&0xFF
            time.sleep(0.001)
            data=f.read(128);
            print(data)
            print("CHK:", CHK)
            for i in range(len(data)):
                 CHK =CHK^data[i]
            time.sleep(0.001)
            print("CHK:", CHK)
            #ser.write(data)
            response=~CHK
            while response!=CHK:
                ser.write(data)
                while ser.inWaiting()==0:
                    #print(ser.inWaiting());
                    time.sleep(0.0001)
                response=ord(ser.read(1))
                print("rsp", response)    
        f.close()
