import socket
import struct
import numpy as np
import matplotlib.pyplot as plt

WIDTH, HEIGHT = 32, 24
BUFFER_SIZE = WIDTH * HEIGHT * 2

MIN_TEMP = -30
MAX_TEMP = 30

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('0.0.0.0', 3333))

Fsum = np.zeros((HEIGHT, WIDTH))


print("Listening for UDP float data on port 3333...")

try:
    for i in range(50):
        data, _ = sock.recvfrom(BUFFER_SIZE)

        if len(data) == BUFFER_SIZE:
            floats = struct.unpack(f'{WIDTH * HEIGHT}h', data)
            temp_array = np.array(floats).reshape((HEIGHT, WIDTH))

            # Create a new plot for each packet
            fig, ax = plt.subplots()
            cax = ax.imshow(temp_array, cmap='binary', vmin=MIN_TEMP, vmax=MAX_TEMP)
            fig.colorbar(cax, ax=ax)
            ax.set_title("Thermal Frame")
            ax.set_xlabel(f"Min: {temp_array.min():.1f}°C   Max: {temp_array.max():.1f}°C")
            plt.show(block=False)
            Fsum+=temp_array

        else:
            print(f"Invalid packet size: {len(data)} bytes")
except KeyboardInterrupt:
    print("Stream stopped")
finally:
    Favg = Fsum / 50
    Favg -= 23
    Favg=Favg.astype(np.float32)
    for v in Favg.flatten():
        print(f'{v}, ',end="")
    print()
    plt.figure()
    plt.imshow(Favg, cmap='binary')
    plt.colorbar()
    plt.show()
    sock.close()