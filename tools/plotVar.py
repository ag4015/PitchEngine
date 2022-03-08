
import matplotlib.pyplot as plt
import numpy as np
import time
from scipy import signal
from numpy import genfromtxt
from os import listdir
from os.path import isfile, join

def plot_delta_t(delta_t, N, fsamp, hopA):
    delta_t = delta_t * fsamp/(2*np.pi)
    fig = plt.figure()
    im = plt.imshow(delta_t[:,:int(N/2)].T, cmap='jet', interpolation='nearest', aspect='auto', origin="lower", extent=[0,(delta_t.shape[0] * hopA)/fsamp,0,fsamp/2])
    plt.colorbar(im)
    plt.xlabel('Time (s)')
    plt.ylabel('Frequency (Hz)')
    plt.savefig(path + "delta_t" + ".png", dpi=300)
    plt.close()

def plot_delta_f(delta_f, N, fsamp, hopA):
    delta_f = delta_f * fsamp/(2*np.pi)
    fig = plt.figure()
    im = plt.imshow(abs(delta_f[:,:int(N/2)].T), cmap='jet', interpolation='nearest', aspect='auto', origin="lower", extent=[0,(delta_f.shape[0] * hopA)/fsamp,0,fsamp/2])
    plt.colorbar(im)
    plt.xlabel('Time (s)')
    plt.ylabel('Frequency (Hz)')
    plt.savefig(path + "delta_f" + ".png", dpi=300)
    plt.close()

def plot_mag(mag, N, fsamp, hopA):
    fig = plt.figure()
    im = plt.imshow(mag[:,:int(N/2)].T, cmap='jet', interpolation='nearest', aspect='auto', origin="lower", extent=[0,(mag.shape[0] * hopA)/fsamp,0,fsamp/2])
    plt.colorbar(im)
    plt.xlabel('Time (s)')
    plt.ylabel('Frequency (Hz)')
    plt.savefig(path + "mag" + ".png", dpi=300)
    plt.close()

def plot_everything(buflen, fsamp, hopA):
    for name in onlyfiles:
        if name == "timings.csv":
            continue
        var = genfromtxt(path + name, delimiter=';',dtype=float)
        var = var[:, ~np.isnan(var).any(axis=0)]
        if len(var) == 0:
            continue
        elif name == "delta_t.csv":
            plot_delta_t(var, buflen, fsamp, hopA)
        elif name == "delta_f.csv":
            plot_delta_f(var, buflen, fsamp, hopA)
        elif name == "mag.csv":
            plot_mag(var, buflen, fsamp, hopA)

BUFLEN = 1024
FSAMP = 44100
TFRAME = (1/FSAMP)*BUFLEN
HOPA = 256
steps = 3 
numFrames = int(BUFLEN/HOPA)
hopS = int(HOPA*2**(steps/12))
algo = "pvdr"
#filename = "sine_chirp_impulse_short"
filename = "sine_short"
magTol = "1e-06"

variationName = "algo_" + algo + "_buflen_" + str(BUFLEN) + "_hopA_" + str(HOPA) + "_" + filename + "_magTol_" + magTol + "_steps_" + str(steps) + "/"

path = "C:/Users/alexg/source/repos/PitchEngine/data/debugData"
path += "/" + filename + "/" + variationName

onlyfiles = [f for f in listdir(path) if isfile(join(path, f)) and f[-4:] == '.csv']

plot_everything(BUFLEN, FSAMP, HOPA)

