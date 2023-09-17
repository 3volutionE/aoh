import os
import socket
import sys
 
# IPC parameters
SOCK_FILE = '/tmp/simple-ipc.socket'
 
# Init socket object
if not os.path.exists(SOCK_FILE):
    print(f"File {SOCK_FILE} doesn't exists")
    sys.exit(-1)
 
s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s.connect(SOCK_FILE)
 
# Send request
s.sendall(b'Hello, world')
 
# Wait for response
data = s.recv(1024)
print('Received bytes: ' + data)
