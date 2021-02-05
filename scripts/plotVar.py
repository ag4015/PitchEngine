
import matplotlib.pyplot as plt
import numpy as np
import time
from numpy import genfromtxt
import os
import glob
import pdb

path = "/mnt/c/Users/alexg/Google Drive/Projects/Guitar Pedal/Software/Pedal/DSPSimulator/debugData/"
extension = 'csv'
os.chdir(path)

FFTLEN = 2048
NFREQ = 1 + FFTLEN/2
FSAMP = 44100
TFRAME = (1/FSAMP)*FFTLEN
hopA = 256
steps = 4 
numFrames = int(FFTLEN/hopA)
hopS = int(hopA*2**(steps/12))

def plot_everything():
    names = glob.glob('*.{}'.format(extension))
    for name in names:
        fig = plt.figure()
        var = np.nan_to_num(genfromtxt(path + name, delimiter=';',dtype=float))
        plt.plot(var)
        plt.savefig(path + name[:-4] + "Plot.png", dpi=300);
        plt.close()

def plot_delta():
    numDeltas = 100
    delta_t = np.nan_to_num(genfromtxt("delta_t" + ".csv", delimiter=';',dtype=float))
    delta_t = delta_t * FSAMP/(2*np.pi)
    delta_f = np.nan_to_num(genfromtxt("delta_f" + ".csv", delimiter=';',dtype=float))
    # delta_f = delta_f * FSAMP/(2*np.pi)
    fig = plt.figure()
    im = plt.imshow(np.flip(delta_t[:,int(FFTLEN/2):].T,0), cmap='jet', interpolation='nearest', aspect='auto', origin="lowest")
    plt.colorbar(im)
    plt.xlabel('Frame')
    plt.ylabel('Frequency bin')
    plt.savefig(path + "delta_t" + "Plot.png", dpi=300)
    plt.close()

    fig = plt.figure()
    im = plt.imshow(np.flip(delta_f[:,int(FFTLEN/2):].T,0), cmap='jet', interpolation='nearest', aspect='auto', origin="lowest")
    plt.colorbar(im)
    plt.xlabel('Frame')
    plt.ylabel('Frequency bin')
    plt.savefig(path + "delta_f" + "Plot.png", dpi=300)
    plt.close()



plot_delta()