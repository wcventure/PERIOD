#!/usr/bin/env python3

import io, re, sys
from ctypes import *
from sysv_ipc import * # Doc: http://semanchuk.com/philip/sysv_ipc
# to install: pip3 install sysv_ipc

def formateTime(seconds):
    '''
    Turn a time interval in seconds into the format of hh:mm:ss.sssss
    '''
    hours, rem = divmod(seconds, 3600)
    minutes, s = divmod(rem, 60)
    return '{:0>2}:{:0>2}:{:08.5f}'.format(int(hours), int(minutes), s)

# align pattern list (filled with zero)
def filled_with_zero(pattern, M):
    for eachSubList in pattern:
        if len(eachSubList) < M:
            eachSubList += [0 for i in range(M-len(eachSubList))]

def calculate_coverage_hashID(kpLocArray, Num, kpNumArray, N, SHARE_ARRAY_SIZE):
    hashID = 0
    if Num > SHARE_ARRAY_SIZE:
        Num = SHARE_ARRAY_SIZE
    for i in range(0, Num):
        hashID = hashID + kpLocArray[i]
    m = 1
    for i in range(0, N):
        m = (kpNumArray[i] * m) % 10000000
    hashID = hashID + m
    return hashID

def add_to_explore_dict(waited_to_explore, coverage_hashID, inter, pattern, N, M, kpNumList, prefix):
    if coverage_hashID in waited_to_explore:
        saved_prefix = [x for tup in waited_to_explore[coverage_hashID][5] for x in tup]
        new_prefix = [x for tup in prefix for x in tup]
        if len(saved_prefix) > len(new_prefix):
            tmpList = [inter, pattern, N, M, kpNumList, prefix]
            waited_to_explore[coverage_hashID] = tmpList
        else:
            return
    else:
        tmpList = [inter, pattern, N, M, kpNumList, prefix]
        waited_to_explore[coverage_hashID] = tmpList

def get_ASAN_error_type(context_bytes):
    try:
        context_strings = context_bytes.decode("utf-8")
        context_io = io.StringIO(context_strings)
        context = context_io.readlines()
    except:
        return '', ''
    CrashType = ''
    CrashDesc = ''
    for line in context:
        line = line.strip().strip(':') # delete the last ':'
        if "ERROR: AddressSanitizer: requested allocation size" in line or "WARNING: AddressSanitizer: requested allocation size" in line:
            CrashType = 'Failed-to-allocate'
        elif "ERROR: AddressSanitizer:" in line or "WARNING: ThreadSanitizer:" in line or "ERROR: ThreadSanitizer:" in line:
            rindex = line.rfind(':')
            tmp = line[rindex+1:].strip()
            left, right = tmp.split(' ',1)
            if left == "attempting":
                left, right = right.split(' ',1)
            if left == "data":
                left = "data-race"
            CrashType = left.strip()
        elif "WARNING: AddressSanitizer" in line or "ERROR: AddressSanitizer" in line:
            rindex = line.find(':')
            if "failed to allocate" in  line and 'WARNING' in line:
                CrashType = 'Failed-to-allocate'
            elif "failed to allocate" in  line and 'ERROR' in line:
                CrashType = 'Failed-to-allocate'
            else:
                tmp = line[rindex+1:].strip()
                left, right = tmp.split(' ',1)
                CrashType = left.strip()
        elif "SUMMARY: AddressSanitizer" in line or "SUMMARY: ThreadSanitizer" in line:
            CrashDesc = line.strip()
            if "leaked" in line and "allocation" in line:
                CrashType = "memory leaks"
                tupleLA = re.findall(r"\d+", line)
        elif "AddressSanitizer CHECK failed" in line:
            index = line.find('AddressSanitizer CHECK failed')
            tmp = line[index:].strip()
            CrashDesc = tmp.strip()
        elif "#" in line:
            tmplist = re.findall(r"\d+:\d+",line)
            if not tmplist == []:
                ll ,rr = tmplist[0].split(':',1)
        elif ("Assertion " in line and "failed" in line) or ("Assertion failure" in line):
            CrashType = "assertion failed"
            break
        if "Aborted (core dumped)" in line or "Aborted" in line or "SUMMARY: AddressSanitizer: " in line or "SUMMARY: ThreadSanitizer: " in line:
        #if "SUMMARY: AddressSanitizer: " in line:
            #print(CrashType, "---", CrashDesc)
            break
    if CrashType == '':
        CrashType = 'Exception'
    return CrashType, CrashDesc

def print_Info_from_shm(N, M, TupleFirst, kpNumArray, kpYieldArray, kpLocArray, kpOrderArray, prefix, coverage_hashID, zero_in_KP, Flag = False):
    if N != None:
        print("ThreadNum: ", N)
    if M != None:
        print("MaxKpNum: ", M)
    if TupleFirst != None:
        print("TupleScheduling:", TupleFirst)

    print("kpNumArray: ", end = '')
    if kpNumArray != None:
        for i in range(0, N + zero_in_KP):
            print(kpNumArray[i], end = " ")
        print()

    if Flag:
        if kpYieldArray != None and kpLocArray != None and kpOrderArray != None:
            for i in range(0, M * (N + zero_in_KP)):
                print(kpYieldArray[i], kpLocArray[i], kpOrderArray[i])

    print("prefix:", prefix)
    if coverage_hashID != None:
        print("coverage_hashID:", coverage_hashID)

def is_redundant(inter):
    if inter != [] and len(inter) >= 2:
        if type(inter[-1]) == list:
            if len(set(inter[-1])) == 1:
                if len(set(inter[-2])) == 1 and set(inter[-2]) != set(inter[-1]):
                    return False
                else:
                    return True
            else:
                return True
        else:
            return True
    else:
        return True

def is_redundant_with_prefix(inter, prefix):
    if inter != [] and len(inter) >= 2:
        if type(inter[0]) == list:
            if len(set(inter[0])) == 1 and set(prefix[0]) == set(inter[0]):
                ''' # outdated
                if len(prefix) == 1:
                    return False
                else:
                    for i in range(0, len(prefix)):
                        if i < len(prefix) - 1 and len(prefix) <= len(inter):
                            if prefix[i] != inter[i]:
                                return True
                        else:
                            if set(prefix[i]) == set(inter[i]):
                                return False
                            else:
                                return True
                '''
                if len(prefix[0]) <= len(inter[0]):
                    return False
                else:
                    return True
            else:
                return True
        else:
            return True
    else:
        return True

def doesZeroInList(kpNumArray, N):
    for i in range(0, N):
        print(N)
        print(i, kpNumArray[i])
        if kpNumArray[i] == 0:
            return False
    return True

def getMaxNuminPattern(pattern_list):
    max = 0
    for each in pattern_list:
        if sum(each) > max:
            max = sum(each)
    return max

def zero_num(kpNumArray, kpMaxNumArray, N):
    
    zero_num = -1
    zero_num_save = -1

    for i in range(0, N): # prevent all 0 in kpNumArray
        if not kpNumArray[i] == 0:
            zero_num = 0
            break

    if not zero_num == 0:
        return 0

    for limit in range(0, 3):
        if zero_num_save == zero_num:
            break
        else:
            zero_num_save = zero_num
        
        zero_num_save = zero_num
        zero_num_for_kpNumArray = 0
        zero_num_for_kpMaxNumArray = 0
        for i in range(0, N + zero_num):
            if kpNumArray[i] == 0:
                zero_num_for_kpNumArray = zero_num_for_kpNumArray + 1
            if kpMaxNumArray[i] == 0:
                zero_num_for_kpMaxNumArray = zero_num_for_kpMaxNumArray + 1
        
        if zero_num_for_kpNumArray <= zero_num_for_kpMaxNumArray:
            zero_num = zero_num_for_kpNumArray
        else:
            zero_num = zero_num_for_kpMaxNumArray
    
    return zero_num
