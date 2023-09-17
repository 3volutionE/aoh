import socket
import numpy as np

 

msgFromClient       = "Hello UDP Server"

bytesToSend         = str.encode(msgFromClient)

serverAddressPort   = ("127.0.0.1", 33333)

bufferSize          = 1024

 

# Create a UDP socket at client side

UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

 

# Send to server using created UDP socket

UDPClientSocket.sendto(bytesToSend, serverAddressPort)

 

msgFromServer = UDPClientSocket.recvfrom(bufferSize)

 

#msg = "Message from Server {}".format(msgFromServer[0])
msg = msgFromServer[0]
msg_size = len(msg)
record_count = (int) (msg_size / 16)


data = np.ndarray(shape=(4,record_count),buffer=msg,dtype='int32')
print("Record counr = %d " % (record_count))
print(data)
