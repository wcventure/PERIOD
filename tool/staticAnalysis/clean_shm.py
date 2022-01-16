#!/usr/bin/env python3
# -*- coding: UTF-8 -*-

import sys
import getopt
import os
import time
import datetime

def main(argv):
    
    ipcsList = []

    result = os.popen('ipcs -m')
    res = result.read()
    findshmid = 0
    for line in res.splitlines():
        print(line)
        if findshmid == 1:
            line = line.strip()
            if len(line) == 0:
                continue
            tmplist = line.split(" ")
            ipcsList.append(tmplist[1])

        if "shmid" in line:
            findshmid = 1
        
    print("cleaning shm ...")
    print()

    for each in ipcsList:
        print("ipcrm -m " + each)
        os.popen("ipcrm -m" + each)


if __name__ == "__main__":
    starttime = datetime.datetime.now()
    main(sys.argv[1:])
    endtime = datetime.datetime.now()
