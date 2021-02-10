import matplotlib.pyplot as plt
from mpl_toolkits import mplot3d
import numpy as np
import pdb
import os
import csv
import sys

def solveEquation(order, start, end, th, plotFunction = False, printVars = False):

    numpoints = order + 1
    command = "./syseqsolver " + str(numpoints) + " " + str(start) + " " + str(end) + " " + str(th)
    os.system(command)

    with open('polyCoeff.csv', newline='\n') as f:
        reader = csv.reader(f)
        dataCoeff = list(reader)
    with open('aData.csv', newline='\n') as f:
        reader = csv.reader(f)
        dataA = list(reader)
    with open('bData.csv', newline='\n') as f:
        reader = csv.reader(f)
        dataB = list(reader)

    coeffs  = []
    aMatrix = []
    bMatrix = []

    for i in range(0, numpoints*numpoints):
        aMatrix.append(float(dataA[i][0]))
    for i in range(0, numpoints):
        bMatrix.append(float(dataB[i][0]))

    for c1 in dataCoeff:
        for c2 in c1:
            coeffs.append(float(c2))

    A   = np.array(aMatrix).reshape(numpoints, numpoints)
    b   = np.array(bMatrix)
    c   = np.array(coeffs)
    err = np.average(abs(np.matmul(A,c) - b))

    if plotFunction:
        plotPoints = 100

        pointsX = []
        pointsY = []

        with open('pointsX.csv', newline='\n') as f:
            reader = csv.reader(f)
            dataX = list(reader)
        with open('pointsY.csv', newline='\n') as f:
            reader = csv.reader(f)
            dataY = list(reader)

        for x1,y1, in zip(dataX, dataY):
            for x2,y2, in zip(x1, y1):
                pointsX.append(float(x2))
                pointsY.append(float(y2))

        pX  = np.array(pointsX)
        pY  = np.array(pointsY)

        y = [0]*plotPoints
        x = list(np.linspace(start, end, plotPoints))

        for n in range(0, plotPoints):
            for i in range(0, order + 1):
                y[n] += coeffs[order - i]*(x[n]**i)

        plt.figure()
        plt.plot(x,y)
        plt.plot(pointsX,pointsY,'*')
        plt.grid()
        plt.ylim(start, end)
        plt.xlim(start, end)
        plt.xlabel("x")
        plt.ylabel("y")
        plt.savefig("function.png", dpi=300)

    if printVars:
        print("A = ")
        print(A)
        print("b = ")
        print(b)
        print("x = ")
        print(c)
        print("Error = " + str(err))

    return err

def plotErrorSurface(minOrder, maxOrder, minLim, maxLim, numLim):

    x = []
    y = []
    z = []

    for o in range(minOrder, maxOrder):
        print("order = " + str(o))
        for lim in list(np.linspace(minLim, maxLim, numLim)):
            x.append(o)
            y.append(lim)
            z.append(solveEquation(o,lim))

    x = np.array(x).reshape(order-minOrder, numLims)
    y = np.array(y).reshape(order-minOrder, numLims)
    z = np.array(z).reshape(order-minOrder, numLims)

    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    ax.plot_surface(x, y, z)

    ax.set_xlabel('X Label')
    ax.set_ylabel('Y Label')
    ax.set_zlabel('Z Label')

    plt.savefig("errorDistribution.png", dpi=300)

def plotError2d(var, static, min, max):

    x = []
    y = []

    if var == "order":
        for o in range(min, max):
            x.append(o)
            y.append(solveEquation(o,static))

    if var == "lim":
        for lim in list(np.linspace(min, max, 100)):
            x.append(lim)
            y.append(solveEquation(static,lim))

    plt.figure()
    plt.plot(x,y)
    plt.grid()
    plt.xlabel(var)
    plt.ylabel("Error")
    plt.title("Error of polynomial with " + ("order" if var == "lim" else "lim") + " = " + str(static))
    plt.savefig("errorDistribution.png", dpi=300)

np.set_printoptions(formatter={'float': '{: 0.5f}'.format})
order = 8
start = 0
end = 1
th = 0.3
numpoints = order + 1
os.system("gcc -Wall -pedantic -g syseqsolver.c -o syseqsolver -lgsl -lgslcblas -lm")

# plotError2d("order", 1, 1, 20)
# plotError2d("lim"  , 12, 2, 2)
solveEquation(order, start, end, th, True, True)

# order = int(sys.argv[1]) if len(sys.argv) > 1 else 10
# xylim = float(sys.argv[2]) if len(sys.argv) > 2 else 1