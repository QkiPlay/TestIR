import socket
import struct
import numpy as np
import matplotlib.pyplot as plt

WIDTH, HEIGHT = 32, 24
INT16_SIZE = 2
BUFFER_SIZE = WIDTH * HEIGHT * INT16_SIZE

MIN_TEMP = 0
MAX_TEMP = 50

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('0.0.0.0', 3333))

print("Listening for UDP int16_t data on port 3333...")

try:
    while True:
        data, _ = sock.recvfrom(BUFFER_SIZE)

        if len(data) == BUFFER_SIZE:
            ints = struct.unpack(f'{WIDTH * HEIGHT}h', data)
            temp_array = np.array(ints).reshape((HEIGHT, WIDTH))

            fig, ax = plt.subplots()
            cax = ax.imshow(temp_array, cmap='binary', vmin=MIN_TEMP, vmax=MAX_TEMP)
            fig.colorbar(cax, ax=ax)
            ax.set_title("Thermal Frame")
            ax.set_xlabel(f"Min: {temp_array.min():.1f}°C   Max: {temp_array.max():.1f}°C")
            plt.show(block=False)
            plt.pause(0.001)
            plt.close(fig)
        else:
            print(f"Invalid packet size: {len(data)} bytes")
except KeyboardInterrupt:
    print("Stream stopped")
finally:
    sock.close()
