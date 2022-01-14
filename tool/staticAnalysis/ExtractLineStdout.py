#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys
import getopt
import os
import time
import datetime

def main(argv):
    TargetFile = ""
    try:
        opts, args = getopt.getopt(argv, "hf:", ["help", "TargetFile="])


    except getopt.GetoptError:
        print('Error: ExtractLine.py -f <TargetFile>')
        sys.exit(2)

    for opt, arg in opts:
        if opt in ("-h", "--help"):
            print('ExtractLine.py -f <TargetFile>')
            sys.exit()
        elif opt in ("-f", "--file"):
            TargetFile = arg

    if TargetFile == "":
        print('Error: Command line is empty')
        print('Tips: Using -h to view help')
        sys.exit(2)

    # start

    TargetPath = os.path.abspath(TargetFile)
    TargetDIR = os.path.dirname(TargetPath)

    LineSplitSet = set()
    with open (TargetFile, 'r') as f:
        for line in f.readlines():
            list = line.split()
            first = list[0].replace(':0', '')
            second = list[1].replace(':0', '')
            third = list[4]
            LineSplitSet.add((first, third))
            LineSplitSet.add((second,third))
        writeList = []
        for each in sorted(LineSplitSet):
            writeList.append(each)

        # merge
        for i in reversed(range(1,len(writeList))):
            if writeList[i][0]==writeList[i-1][0]:
                writeList[i-1]=(writeList[i][0], writeList[i-1][1]+'|'+writeList[i][1])
                del writeList[i]

        #writeList.remove(('uaf.c:17', '18'))
        if not writeList == []:
            for each in writeList:
                print(each[0] + " " + each[1])
                pass
        print("")
        
        # end

if __name__ == "__main__":
    starttime = datetime.datetime.now()
    main(sys.argv[1:])
    endtime = datetime.datetime.now()

