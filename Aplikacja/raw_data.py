import socket
import struct
import numpy as np
import matplotlib.pyplot as plt

WIDTH, HEIGHT = 32, 24
BUFFER_SIZE = 1668
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('0.0.0.0', 3333))



print("Listening for UDP float data on port 3333...")

for i in range(5):
    data, _ = sock.recvfrom(BUFFER_SIZE)
    data_t = np.frombuffer(data,dtype="<H")
    np.savetxt(f"/home/qki/Documents/raw_data{i}.txt", data_t)