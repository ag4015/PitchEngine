import csv

t1 = open("/mnt/c/Users/alexg/source/repos/PitchEngine/data/debugData/constant_guitar_short/algo_pvdr_buflen_2048_hopA_256_constant_guitar_short_magTol_1e-06_steps_3/delta_t.csv")
t2 = open("/mnt/c/Users/alexg/source/repos/PitchEngine/data/debugData/constant_guitar_short/algo_pv_buflen_2048_hopA_256_constant_guitar_short_magTol_1e-06_steps_3/delta_t.csv")

fileone = t1.readlines()
filetwo = t2.readlines()
t1.close()
t2.close()

outFile = open('diff.csv', 'w')
x = 0
for i in fileone:
    if i != filetwo[x]:
        outFile.write(filetwo[x])
    x += 1
outFile.close()

