
import os
import numpy as np

count = 0
for i in np.linspace(0, 2, 100):
    count = count + 1
    os.system("./build/main constant_guitar_short.wav output" + str(count) + ".wav " + str(i))