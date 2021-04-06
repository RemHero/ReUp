import numpy as np
import matplotlib
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from matplotlib import cm
from matplotlib.ticker import LinearLocator, FormatStrFormatter
import sys

name = ["feng","syy","fgru","jjx","lab"]
color = ["red","black","green","blue","yellow"]

def test1(name,file_num,color):
    fig = plt.figure()
    plt.title('Output test')
    plt.xlabel('number of line')
    plt.ylabel('time(ms)')
    label = ["to_file","to_terminal"]
    file_name = [name + "_to_file.log", name + "_to_ternimal.log"]
    color = ["red","blue"]
    for i in range(2):
        data = np.loadtxt(file_name[i], dtype=int)
        
        print(data)
        plt.plot(data[:, 0], data[:, 1], label=label[i],
                 color=color[i], linewidth=2)
    plt.legend()
    plt.savefig(name+"_test1.png")
    plt.close()

def test2(name,color):
    plt.title('Thread test')
    plt.xlabel('number of thread')
    plt.ylabel('time(ms)')
    data = np.loadtxt(name+"_thread.log", dtype=int)
    plt.plot(data[:, 0], data[:, 1], label= name,
                 color=color, linewidth=2)
    plt.legend()
    plt.savefig("test2.png")

def test3(name,color):
    plt.title('Block test')
    plt.xlabel('number of thread')
    plt.ylabel('time(ms)')
    data = np.loadtxt(name+"_block.log", dtype=int)
    plt.plot(data[:, 0], data[:, 1], label= name,
                 color=color, linewidth=2)
    plt.legend()
    plt.savefig("test3.png")

def Speedup_ratio(name,color):
    plt.title('Speedup ratio')
    plt.xlabel('number of line')
    plt.ylabel('time(ms)')
    data1 = np.loadtxt(name+"_single.log", dtype=int)
    data2 = np.loadtxt(name+"_multi.log", dtype=int)
    y = data1[:,1] / data2[:,1]
    plt.plot(data1[:, 0], y, label= name,
                 color=color, linewidth=2)
    plt.legend()
    plt.savefig("Speedup_ratio.png")

def multi_png_test2():
    fig = plt.figure()
    for i in range(4):
        test2(name[i],color[i])
    plt.close()


def multi_png_test3():
    fig = plt.figure()
    for i in range(4):
        test3(name[i],color[i])
    plt.close()

def multi_png_Speedup_ratio():
    fig = plt.figure()
    for i in range(5):
        Speedup_ratio(name[i],color[i])
    plt.close()

if __name__ == '__main__':
    multi_png_test2()
    multi_png_test3()
    multi_png_Speedup_ratio()
    # name = sys.argv[1]

    # test1(name,sys.argv[2])
    # test2(name)
    # test3(name)
    # Speedup_ratio(name)
© 2021 GitHub, Inc.
Terms
Privacy
Security
Status
Docs
Contact GitHub
Pricing
API
Training
Blog
About
