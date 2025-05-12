import socket
import struct
import numpy as np
import matplotlib.pyplot as plt

WIDTH, HEIGHT = 32, 24
FLOAT_SIZE = 4
BUFFER_SIZE = WIDTH * HEIGHT * FLOAT_SIZE

MIN_TEMP = 0.0
MAX_TEMP = 30.0

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('0.0.0.0', 3333))

print("Listening for UDP float data on port 3333...")

try:
    while True:
        data, _ = sock.recvfrom(BUFFER_SIZE)
        if len(data) == BUFFER_SIZE:
            floats = struct.unpack(f'{WIDTH * HEIGHT}f', data)
            temp_array = np.array(floats).reshape((HEIGHT, WIDTH))

            # Create a new plot for each packet
            fig, ax = plt.subplots()
            cax = ax.imshow(temp_array, cmap='binary', vmin=MIN_TEMP, vmax=MAX_TEMP)
            fig.colorbar(cax, ax=ax)
            ax.set_title("Thermal Frame")
            ax.set_xlabel(f"Min: {temp_array.min():.1f}°C   Max: {temp_array.max():.1f}°C")
            plt.show(block=False)
        else:
            print(f"Invalid packet size: {len(data)} bytes")
except KeyboardInterrupt:
    print("Stream stopped")
finally:
    sock.close()