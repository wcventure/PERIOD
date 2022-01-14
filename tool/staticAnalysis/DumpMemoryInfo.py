#!/usr/bin/env python3

import sys, subprocess, time, os, getopt, copy, signal
from datetime import datetime
from ast import literal_eval
from ctypes import *
import numpy as np

# =================================================================================

reproduceFile = ""
memInfoFile = ""
TaskReproduce = 0
LoadMemInfoFile = 0
opts = []
args = []
try:
    opts, args = getopt.getopt(sys.argv[1:], "hm:", ["help", "memInfoFile="])
except:
    pass
print("opts:", opts)
print("args:", args)

for opt, arg in opts:
    if opt in ("-h", "--help"):
        print('./run_DBDS.py -m <memInfoFile>')
        sys.exit()
    elif opt in ("-m", "--memInfoFile"):
        memInfoFile = arg
        print("memInfoFile:", memInfoFile)

# load npz data for memory Info
pureSoList = []
memGroupList = []
memPairList = []


if not os.path.exists(memInfoFile):
    print("memInfoFile " + memInfoFile + " not exists.")
    sys.exit()
NPZ = np.load(memInfoFile,allow_pickle=True)
pureSoList = NPZ["arr_0"].tolist()
memGroupList = NPZ["arr_1"].tolist()
memPairList =  NPZ["arr_2"].tolist()

print("\033[34mpureSoList: \033[0m")
for each in pureSoList:
    print(each)
print()

print("\033[34mmemGroupList: \033[0m")
for each in memGroupList:
    print(each)
print()

print("\033[34mmemPairList: \033[0m")
for each in memPairList:
    print(each)
print()