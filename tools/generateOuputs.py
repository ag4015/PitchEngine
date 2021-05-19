
import os
import numpy as np

count = 0
for i in np.linspace(0, 2, 100):
    count = count + 1
    print(count)
    os.system("./bin/WSL-GCC-Debug/DSPSim sine_short.wav output_" + str(i).replace(".", "_") + ".wav " + str(i))
