#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys
import getopt
import os
import time
import datetime

def main(argv):
    #通过 getopt模块 来识别参数demo

    TargetFile = ""

    try:

        opts, args = getopt.getopt(argv, "hf:", ["help", "TargetFile="])


    except getopt.GetoptError:
        print('Error: ExtractLine.py -f <TargetFile>')
        sys.exit(2)

    # 处理 返回值options是以元组为元素的列表。
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
    
    print('Target file = ', TargetFile)
    print('')        


    # 打印 返回值args列表，即其中的元素是那些不含'-'或'--'的参数。
    for i in range(0, len(args)):
        print('参数 %s 为：%s' % (i + 1, args[i]))


    # start

    TargetPath = os.path.abspath(TargetFile)
    TargetDIR = os.path.dirname(TargetPath)
    print(TargetPath)
    print(TargetDIR)

    LineSplitSet = set()
    with open (TargetFile, 'r') as f:
        for line in f.readlines():
            list = line.split()
            first = list[0].replace(':0', '')
            second = list[1].replace(':0', '')
            LineSplitSet.add(first)
            LineSplitSet.add(second)
    
    writeList = []
    for each in LineSplitSet:
        writeList.append(each)
        
    with open (TargetDIR+'/ConConfig', 'w+') as fi:
        if not writeList == []:
            for each in writeList:
                fi.write(each+'\n')
            fi.write('\n')
            
    # end

	
if __name__ == "__main__":
    # sys.argv[1:]为要处理的参数列表，sys.argv[0]为脚本名，所以用sys.argv[1:]过滤掉脚本名。
    starttime = datetime.datetime.now()

    main(sys.argv[1:])
    
    endtime = datetime.datetime.now()
    
    print ("start time: ", starttime)
    print ("end time: ", endtime)
    print ("The output can be seen in ConConfig")
    print ("@@@ Finished @@@")
