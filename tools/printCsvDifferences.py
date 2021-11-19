import sys


file1 = "/mnt/c/Users/alexg/source/repos/DSPSim/data/debugData/constant_guitar_short/buflen_1024_hopS_256_hopA_256_algo_pv/cpxIn.csv"
file2 = "/mnt/c/Users/alexg/source/repos/DSPSim/data/debugData/constant_guitar_short/buflen_1024_hopS_256_hopA_256_algo_au/cpxIn.csv"

#if len(sys.argv) != 2:
#    print("Error, incorrect number of arguments")

with open(file1, 'r') as t1, open(file2, 'r') as t2:
    fileone = t1.readlines()
    filetwo = t2.readlines()

with open('diff.csv', 'w') as outFile:
    for line in filetwo:
        if line not in fileone:
            outFile.write(line)




