
import matplotlib.pyplot as plt
import numpy as np
import time
from numpy import genfromtxt

FFTLEN = 1024
NFREQ = 1 + FFTLEN/2
FSAMP = 44100
TFRAME = (1/FSAMP)*FFTLEN

plt.ion()

fig = plt.figure()
plt.grid(True,which="both",ls="-")
ax = fig.add_subplot(111)
fft = np.nan_to_num(genfromtxt('freq.csv', delimiter=';',dtype=complex))
mag = np.abs(fft)
line1, = ax.loglog(np.linspace(0, FSAMP/2, FFTLEN), np.ones(FFTLEN), 'r-') 
axes = plt.gca()
axes.set_ylim([0.0001,10000])
plt.xticks([10,100,1000,10000], ('10', '100', '1,000', '10,000'))
plt.yticks([0.0001, 0.001, 0.01, 0.1, 1, 10, 100, 1000, 10000], ['-40','-30','-20','-10','0','10','20','30','40'])
plt.xlabel('$Hz$')
plt.ylabel('$dB$')
plt.show()

for x in [3,2,1]:
    b = "Starting in " + str(x)
    print(b, end="\r")
    time.sleep(1)
    
start = time.time()
frame_start = 60

while int(frame_start/TFRAME) < len(mag):
    line1.set_ydata(20*np.log(mag[int(frame_start/TFRAME),:FFTLEN]))
    fig.canvas.draw()
    fig.canvas.flush_events()
    end = time.time()
    b = "Time:  " + str(int((end-start)/60)) + " : " + str(int((end-start)%60))
    plt.title(b)
    print(b, end="\r")
    time.sleep(TFRAME)
    frame_start = time.time() - start


    
