
import numpy as np
import pandas as pd

# import the dataset
debugDataPath = "C:\\Users\\alexg\\source\\repos\\PitchEngine\\data\\debugData\\"
generatedSignalFolder = "sine_440\\"
#algorithmFolder = "algo_pv_buflen_1024_freq_440_numSamp_122880_sine_steps_3\\"
algorithmFolder = "algo_pv_buflen_8_freq_440_numSamp_1024_sine_steps_3\\"
folderPath = debugDataPath + generatedSignalFolder + algorithmFolder

magFile = folderPath + "mag.csv"
phaseFile = folderPath + "phi_a.csv"

# Turn csv to numpy array and remove last nan column
mag = np.genfromtxt(magFile, delimiter=';')[:,:-1] 
phase = np.genfromtxt(phaseFile, delimiter=';')[:,:-1]
data = np.concatenate((mag,phase), axis=1)

