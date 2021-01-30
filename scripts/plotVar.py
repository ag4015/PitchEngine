
import matplotlib.pyplot as plt
import numpy as np
import time
from numpy import genfromtxt
import os
import glob

path = "/mnt/c/Users/alexg/Google Drive/Projects/Guitar Pedal/Software/Pedal/DSPSimulator/debugData/"
extension = 'csv'
os.chdir(path)
names = glob.glob('*.{}'.format(extension))

FFTLEN = 2048
NFREQ = 1 + FFTLEN/2
FSAMP = 44100
TFRAME = (1/FSAMP)*FFTLEN
hopA = 256
steps = 4 
numFrames = int(FFTLEN/hopA)
hopS = int(hopA*2**(steps/12))
# names = ["inbuffer", "cpx", "previousPhase", "phase", "inwin", "outwin", "deltaPhi", "deltaPhiPrime", "deltaPhiPrimeMod", "trueFreq", "phaseCumulative"]
# mynames = ["phi_s"]

xcoordsA = [hopA*i for i in range(0,numFrames)]
xcoordsS = [hopS*i for i in range(0,numFrames)]
for name in names:
    # if "phi_s" not in name:
    #     continue
    fig = plt.figure()
    var = np.nan_to_num(genfromtxt(path + name, delimiter=';',dtype=float))
    plt.plot(var)
    for xc in xcoordsA:
        plt.axvline(x=xc,color='blue')
    for xc in xcoordsS:
        plt.axvline(x=xc,color='red')
    plt.savefig(path + name[:-4] + "Plot.png", dpi=300);
    plt.close()

