#!/usr/bin/env python3
# -*- coding: UTF-8 -*-

import sys
import getopt
import os
import time
import datetime
import numpy as np

def main(argv):
    soFile = ""
    memGroupFile = ""
    try:
        opts, args = getopt.getopt(argv, "hi:m:", ["help", "soFile=", "memGroupFile"])


    except getopt.GetoptError:
        print('Error: updateMempairSo.py -i <soFile> -m <memGroupFile>')
        sys.exit(2)

    for opt, arg in opts:
        if opt in ("-h", "--help"):
            print('updateMempairSo.py -i <soFile> -m <memGroupFile>')
            sys.exit()
        elif opt in ("-i", "--soFile"):
            soFile = arg
        elif opt in ("-m", "--memgroupFile"):
            memGroupFile = arg

    if soFile == "" or memGroupFile == "":
        print('Error: Command line is empty')
        print('Tips: Using -h to view help')
        sys.exit(2)

    # start
    soPath = os.path.abspath(soFile)
    soDIR = os.path.dirname(soPath)

    # load memgroup from npy file
    loadedList=np.load(memGroupFile, allow_pickle=True)
    memGroupList=loadedList.tolist()
    
    # load so file
    soList = []
    with open (soFile, 'r') as f:
        for line in f.readlines():
            if (line[0:7] == "[DEBUG]"):
                pass
            else:
                list = line.split()
                soList.append([list[0],list[1],list[2],list[3]])
    soList = sorted(soList)
                
    # Get Pure SoList
    pureSoList = []
    for each in soList:
        if len(pureSoList) == 0:
            pureSoList.append(each)
        else:
            if each[0]!=pureSoList[-1][0]:
                pureSoList.append(each)
            else:
                if each[1]!="R/W":
                    pureSoList[-1][1]=each[1]
    
    # remove unlock first
    CopySoList = pureSoList.copy()
    for each in CopySoList:
        if each[1] == "unlock":
            pureSoList.remove(each)
    
    # add those pthread_mutex_lock and free into memgroup
    hasUkLockSet = 0
    hasUkFreeSet = 0
    for each in pureSoList:
        if each[1] == "lock": # find all lock
            doesThisLockInMemGroup = 0
            for eachfloor in memGroupList:
                if each[0] in eachfloor[1] or each[0] in eachfloor[2]:
                    doesThisLockInMemGroup = 1
                    break
            if doesThisLockInMemGroup == 0:
                if hasUkLockSet == 0:
                    hasUkLockSet = 1
                    memGroupList.append([99999, set(), set()])
                    memGroupList[-1][1].add(each[0])
                else:
                    memGroupList[-1][1].add(each[0])
        if each[1] == "free": # find all free
            doesThisFreeInMemGroup = 0
            for eachfloor in memGroupList:
                if each[0] in eachfloor[1] or each[0] in eachfloor[2]:
                    doesThisFreeInMemGroup = 1
                    break
            if doesThisFreeInMemGroup == 0:
                if hasUkFreeSet == 0:
                    hasUkFreeSet = 1
                    memGroupList.append([99998, set(), set()])
                    memGroupList[-1][1].add(each[0])
                else:
                    memGroupList[-1][1].add(each[0])
        if each[1] == "std_atomic": # find all std_atomic
            doesThisFreeInMemGroup = 0
            for eachfloor in memGroupList:
                if each[0] in eachfloor[1] or each[0] in eachfloor[2]:
                    doesThisFreeInMemGroup = 1
                    break
            if doesThisFreeInMemGroup == 0:
                if hasUkFreeSet == 0:
                    hasUkFreeSet = 1
                    memGroupList.append([99997, set(), set()])
                    memGroupList[-1][1].add(each[0])
                else:
                    memGroupList[-1][1].add(each[0])
        if each[1] == "std_size": # find all std_atomic
            doesThisFreeInMemGroup = 0
            for eachfloor in memGroupList:
                if each[0] in eachfloor[1] or each[0] in eachfloor[2]:
                    doesThisFreeInMemGroup = 1
                    break
            if doesThisFreeInMemGroup == 0:
                if hasUkFreeSet == 0:
                    hasUkFreeSet = 1
                    memGroupList.append([99996, set(), set()])
                    memGroupList[-1][1].add(each[0])
                else:
                    memGroupList[-1][1].add(each[0])
        if each[1] == "memset": # find all memset
            doesThisFreeInMemGroup = 0
            for eachfloor in memGroupList:
                if each[0] in eachfloor[1] or each[0] in eachfloor[2]:
                    doesThisFreeInMemGroup = 1
                    break
            if doesThisFreeInMemGroup == 0:
                if hasUkFreeSet == 0:
                    hasUkFreeSet = 1
                    memGroupList.append([99995, set(), set()])
                    memGroupList[-1][1].add(each[0])
                else:
                    memGroupList[-1][1].add(each[0])
        if each[1] == "std::vector<...>::resize": # find all std::vector<...>::resize
            doesThisFreeInMemGroup = 0
            for eachfloor in memGroupList:
                if each[0] in eachfloor[1] or each[0] in eachfloor[2]:
                    doesThisFreeInMemGroup = 1
                    break
            if doesThisFreeInMemGroup == 0:
                if hasUkFreeSet == 0:
                    hasUkFreeSet = 1
                    memGroupList.append([99994, set(), set()])
                    memGroupList[-1][1].add(each[0])
                else:
                    memGroupList[-1][1].add(each[0])
        if each[1] == "cmpnull": # find all cmpnull
            doesThisFreeInMemGroup = 0
            for eachfloor in memGroupList:
                if each[0] in eachfloor[1] or each[0] in eachfloor[2]:
                    doesThisFreeInMemGroup = 1
                    break
            if doesThisFreeInMemGroup == 0:
                if hasUkFreeSet == 0:
                    hasUkFreeSet = 1
                    memGroupList.append([99993, set(), set()])
                    memGroupList[-1][1].add(each[0])
                else:
                    memGroupList[-1][1].add(each[0])
    
    # clean those keypoint that not in pthread_create's callee
    for eachfloor in memGroupList:
        eachfloor.append(set())
        copySet = eachfloor[2].copy()
        for eachload in copySet:
            pthreadRelated = 0
            for eachSo in pureSoList:
                if eachload == eachSo[0]:
                    if eachSo[1] != 'R/W' and eachSo[1] != 'R' and eachSo[1] != 'W':
                        eachfloor[3].add(eachSo[0])
                        eachfloor[2].remove(eachload)
                    else:
                        eachSo[1] = 'R'
                    pthreadRelated = 1
                    break
            if pthreadRelated == 0:
                eachfloor[2].remove(eachload)
        copySet = eachfloor[1].copy()
        for eachstore in copySet:
            pthreadRelated = 0
            for eachSo in pureSoList:
                if eachstore == eachSo[0]:
                    if eachSo[1] != 'R/W' and eachSo[1] != 'R' and eachSo[1] != 'W':
                        eachfloor[3].add(eachSo[0])
                        eachfloor[1].remove(eachstore)
                    else:
                        eachSo[1] = 'W'
                    pthreadRelated = 1
                    break
            if pthreadRelated == 0:
                eachfloor[1].remove(eachstore)
    
    # print(memGroupList)
    # print()
    # print(pureSoList)

    # remove only read
    tempList = memGroupList.copy()
    for each in tempList:
        if each[1]==set() and each[3]==set():
            memGroupList.remove(each)
            
    # update ConConfig
    ConConfigSet = set()
    for each in memGroupList:
        for e in each[1]:
            ConConfigSet.add(e)
        for e in each[2]:
            ConConfigSet.add(e)
        for e in each[3]:
            ConConfigSet.add(e)
    ConConfigList = sorted(ConConfigSet)
    
    # update So list
    cpyPureSoList = pureSoList.copy()
    for each in cpyPureSoList:
        if each[0] not in ConConfigList:
            pureSoList.remove(each)
    
    # save list (memGroupList and pureSoList)
    npPureSoList = np.array(pureSoList)
    npMemGroupList = np.array(memGroupList)
    np.savez("soAndGroup.npz",npPureSoList,npMemGroupList)
    
    # print ConConfig & list
    for each in ConConfigList:
        print(each)
    #print(memGroupList)


if __name__ == "__main__":
    starttime = datetime.datetime.now()
    main(sys.argv[1:])
    endtime = datetime.datetime.now()

