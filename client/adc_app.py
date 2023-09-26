import socket
import numpy as np
import pandas as pd
from datetime import datetime
import matplotlib.pyplot as plt


#msgFromClient       = "Hello UDP Server"
#bytesToSend         = str.encode(msgFromClient)

serverAddressPort   = ("127.0.0.1", 33333)
bufferSize          = 65000
msg_capture         = "capture"
 


# Create a UDP socket at client side
UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

while True:
    
    keyboard_inp = input("press 'enter' to capture or type 'exit' to exit program\n >> ")
    if (keyboard_inp == "exit") :
        break

    # Send start capture command to server
    UDPClientSocket.sendto(str.encode(msg_capture), serverAddressPort)
    
    # Wait until there is a trigger
    data, server = UDPClientSocket.recvfrom(bufferSize)
    
    # When trig'd and data arrive, log the time
    now = datetime.now()
    csv_filename = now.strftime("%Y%m%d-%H%M%S.csv")
    filepath = "./capture_data/"
    print ("File name = ", csv_filename)

    # extract data from byte string
    msg_size = len(data)
    print("Message size = %d " % (msg_size))
    record_count = (int) (msg_size / 8)
    print("Record count = %d " % (record_count))

    # Data format [time3, time2, time1, time0, ch-a0, ch-a1, ch-b0, chb1]
    data = np.ndarray(shape=(record_count,8), buffer=data, dtype='uint8')    
    #print (data)
    
    # Timestamp of the sample
    #timestamp = np.arange(record_count)
    timestamp = (data[:,3] * 16777216) + (data[:,2] * 65536) + (data[:,1] * 256) + (data[:,0])
    ref_time = timestamp[0]
    sample_time = timestamp - ref_time
    
    # Raw data in binary
    data_ch1 = ((data[:,5] * 256) + data[:,4]).astype(np.int16)
    data_ch3 = ((data[:,7] * 256) + data[:,6]).astype(np.int16)

    # Calculation to VOltage
    vol_ch1 = data_ch1 * 2 * 3.3 / 65536
    vol_ch3 = data_ch3 * 2 * 3.3 / 65536
    
    # Save to CSV file
    dict_of_arrs = {"Timestamp" : timestamp, "Sample_Time" : sample_time, "CH1" : data_ch1, "CH3" : data_ch3}
    df = pd.DataFrame(dict_of_arrs)
    df.to_csv(filepath + csv_filename, header=True, index=False)
    
    
    # Create graph
    fig = plt.figure()
    fig.set_figwidth(15)
    
    plt1 = fig.add_subplot(1,2,1)
    plt2 = fig.add_subplot(1,2,2)
    
    plt1.set_ylim(-3.5, 3.5)
    plt1.plot(sample_time / 1000 , vol_ch1)     # show in us
    plt1.set_title("CH1")
    
    plt2.set_ylim(-3.5, 3.5)
    plt2.plot(sample_time / 1000000 , vol_ch3) # show in ms
    plt2.set_title("CH3")
    
    fig.subplots_adjust(hspace=.5,wspace=0.5)
    plt.show()


print ("Exit program")
