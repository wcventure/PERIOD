#!/usr/bin/env python3
# -*- coding: UTF-8 -*-

import sys
import getopt
import os
import time
import datetime

def main(argv):
    soFile = ""
    mempairFile = ""
    try:
        opts, args = getopt.getopt(argv, "hi:m:", ["help", "soFile=", "mempairFile"])


    except getopt.GetoptError:
        print('Error: updateMempairSo.py -i <soFile> -m <mempairFile>')
        sys.exit(2)

    for opt, arg in opts:
        if opt in ("-h", "--help"):
            print('updateMempairSo.py -i <soFile> -m <mempairFile>')
            sys.exit()
        elif opt in ("-i", "--soFile"):
            soFile = arg
        elif opt in ("-m", "--mempairFile"):
            mempairFile = arg

    if soFile == "" or mempairFile == "":
        print('Error: Command line is empty')
        print('Tips: Using -h to view help')
        sys.exit(2)

    # start

    soPath = os.path.abspath(soFile)
    soDIR = os.path.dirname(soPath)

    LineSplitList = []
    with open (mempairFile, 'r') as f:
        for line in f.readlines():
            list = line.split()
            first = list[0].replace(':0', '')
            second = list[1].replace(':0', '')
            LineSplitList.append([first,second,list[2],list[3],list[4],list[5],' ',' '])

    soList = []
    soSet = set()
    with open (soFile, 'r') as f:
        for line in f.readlines():
            if (line[0:7] == "[DEBUG]"):
                pass
            else:
                list = line.split()
                soList.append([list[0],list[1],list[2],list[4]])
                soSet.add(list[0])
    
    
    for eachSo in soList:
        for eachpair in LineSplitList:
            if eachSo[0] == eachpair[0]:
                pair0 = 1
                eachpair[6] = eachSo[3]
                if '|' in eachSo[1]:
                    eachpair[4] = eachSo[1]
                if eachSo[2] != "R/W":
                    eachpair[2] = eachSo[2]
                
            if eachSo[0] == eachpair[1]:
                eachpair[7] = eachSo[3]
                if '|' in eachSo[1]:
                    eachpair[5] = eachSo[1]
                if eachSo[2] != "R/W":
                    eachpair[3] = eachSo[2]
                    
    UpdatedList = []
    for eachPair in LineSplitList:
        matchPair0 = 0
        matchPair1 = 0
        for eachSo in soSet:
            if eachPair[0] == eachSo:
                matchPair0 = 1
            if eachPair[1] == eachSo:
                matchPair1 = 1
        if matchPair0 == 1 and matchPair1 == 1:
            UpdatedList.append(eachPair)
                
    for each in UpdatedList:
        print(each[0], each[1], each[2], each[3], each[4], each[5], each[6], each[7])
    
    # print(UpdatedList)
    # print("")
    # end

if __name__ == "__main__":
    starttime = datetime.datetime.now()
    main(sys.argv[1:])
    endtime = datetime.datetime.now()

