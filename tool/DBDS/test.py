#!/usr/bin/env python3

'''
For testing only.
To be deleted before the branch is merged.
'''

from external_def import *
from ipc import *
from interleaving import *
from bp_sched import *
from sbp_sched_mixed_end import *
from ctypes import *
import sys, struct, time

# from shm_kit import *
# This is the structrue of Share Memory class
# typedef struct Sched_Info {
#	uint64_t time;										// the execution time of the program (nsec)
#	unsigned int N;										// number of threads concerned
#	unsigned int M;										// number of kep points per thread
#	unsigned int TestedPeriod;							// It took several period in testing engine
#	unsigned int kpNumArray[SHARE_MEMORY_THREAD_NUM];	// the specific kpNum of each My_tid Thread, only consider almost 10 thread
#	unsigned int kp[SHARE_MEMORY_ARRAY_SIZE];			// how many times should a thread yield before a kp
#	unsigned int kpLoc[SHARE_MEMORY_ARRAY_SIZE];		// the hash ID of Key Point
#	unsigned int kpOrder[SHARE_MEMORY_ARRAY_SIZE];		// recored the order of key points
# } sched_info;

# shm = setup_shm(0xff, 8)
#
# # print(int(shm.read(8), 2))
# shm.write(b'1') # 37
# print(shm.read(8))
# print(int.from_bytes(shm.read(4), sys.byteorder))
# # print(int(shm.read(8), 2))
# print(int.from_bytes(shm.read(8), sys.byteorder))
# shm.write(c_int(42))
# print(c_int(42))
# print(int.from_bytes(shm.read(4), sys.byteorder))
# print(int.from_bytes(shm.read(4), 'big'))
#
# shm.write(b'\xfe\x00\x00\x00') # 9
# print(int.from_bytes(shm.read(4), sys.byteorder))
#
# SI = struct.Struct('I2I')
# info = SI.pack(1, 2, 3)
# print(info)

# class Sched_Info(Structure):
#     _fields_ = [
#         ('N', c_char),
#         ('m', c_char),
#         ('kp', c_uint * 2)
#     ]

#############################################################

def test(scheduler, verbose = True):
    start = time.perf_counter()
    scheds = []
    count = 0
    for i in scheduler:
        if i in scheds:
            print("Duplicated!")
        scheds.append(i)
        if verbose:
            try:
                chosen = scheduler._chosen
            except:
                chosen = []
            print(i, chosen, [set(x) for x in i])
        count += 1
    print("Num of scheds: ", count)
    end = time.perf_counter()
    print("Time cost: ", end - start)

print("-----Test 1---------")
'''
[[0, 0], [1, 1]]
[[1, 1], [0, 0]]
'''
s = SBPSched(2, [2], 2)
test(s)

print("-----Test 2---------")
'''
[[0, 0, 0, 0, 0, 0, 0, 0], [1]]
[[1], [0, 0, 0, 0, 0, 0, 0, 0]]
'''
s = SBPMESched(2, [8,1], 2)
test(s)

print("-----Test 2.5--------")
'''
[[0], [1], [0, 0, 0, 0, 0, 0, 0]]
[[0, 0], [1], [0, 0, 0, 0, 0, 0]]
[[0, 0, 0], [1], [0, 0, 0, 0, 0]]
[[0, 0, 0, 0], [1], [0, 0, 0, 0]]
[[0, 0, 0, 0, 0], [1], [0, 0, 0]]
[[0, 0, 0, 0, 0, 0], [1], [0, 0]]
[[0, 0, 0, 0, 0, 0, 0], [1], [0]]
'''
s = SBPMESched(2, [8,1], 3)
test(s)

print("-----Test 3---------")
'''
[[0, 0], [1, 1, 2, 2, 3, 3]]
[[1, 1], [0, 0, 2, 2, 3, 3]]
[[2, 2], [0, 0, 1, 1, 3, 3]]
[[3, 3], [0, 0, 1, 1, 2, 2]]
'''
s = SBPMESched(4, [2], 2)
test(s)

print("-----Test 4---------")
'''
[[0, 0], [1, 1], [2, 2, 3, 3]]
[[1, 1], [0, 0], [2, 2, 3, 3]]
[[0, 0], [2, 2], [1, 1, 3, 3]]
[[2, 2], [0, 0], [1, 1, 3, 3]]
[[0, 0], [3, 3], [1, 1, 2, 2]]
[[3, 3], [0, 0], [1, 1, 2, 2]]
[[1, 1], [2, 2], [0, 0, 3, 3]]
[[2, 2], [1, 1], [0, 0, 3, 3]]
[[1, 1], [3, 3], [0, 0, 2, 2]]
[[3, 3], [1, 1], [0, 0, 2, 2]]
[[2, 2], [3, 3], [0, 0, 1, 1]]
[[3, 3], [2, 2], [0, 0, 1, 1]]
[[0], [1, 1], [0, 2, 2, 3, 3]]
[[1], [0, 0], [1, 2, 2, 3, 3]]
[[0], [2, 2], [0, 1, 1, 3, 3]]
[[2], [0, 0], [2, 1, 1, 3, 3]]
[[0], [3, 3], [0, 1, 1, 2, 2]]
[[3], [0, 0], [3, 1, 1, 2, 2]]
[[1], [2, 2], [1, 0, 0, 3, 3]]
[[2], [1, 1], [2, 0, 0, 3, 3]]
[[1], [3, 3], [1, 0, 0, 2, 2]]
[[3], [1, 1], [3, 0, 0, 2, 2]]
[[2], [3, 3], [2, 0, 0, 1, 1]]
[[3], [2, 2], [3, 0, 0, 1, 1]]
'''
s = SBPMESched(4, [2], 3)
test(s)

print("-----Test 5---------")
'''
[[0], [1], [0, 0, 0, 0, 0, 0, 0], [1]]
[[0, 0], [1], [0, 0, 0, 0, 0, 0], [1]]
[[0, 0, 0], [1], [0, 0, 0, 0, 0], [1]]
[[0, 0, 0, 0], [1], [0, 0, 0, 0], [1]]
[[0, 0, 0, 0, 0], [1], [0, 0, 0], [1]]
[[0, 0, 0, 0, 0, 0], [1], [0, 0], [1]]
[[0, 0, 0, 0, 0, 0, 0], [1], [0], [1]]
[[1], [0], [1], [0, 0, 0, 0, 0, 0, 0]]
[[1], [0, 0], [1], [0, 0, 0, 0, 0, 0]]
[[1], [0, 0, 0], [1], [0, 0, 0, 0, 0]]
[[1], [0, 0, 0, 0], [1], [0, 0, 0, 0]]
[[1], [0, 0, 0, 0, 0], [1], [0, 0, 0]]
[[1], [0, 0, 0, 0, 0, 0], [1], [0, 0]]
[[1], [0, 0, 0, 0, 0, 0, 0], [1], [0]]
'''
s = SBPMESched(2, [8,2], 4)
test(s)

print("-----Test 6---------")
'''
[[0, 0, 0], [1], [0, 0, 0, 0, 0]]
[[0, 0, 0, 0], [1, 1], [0, 0, 0, 0]]
[[0, 0, 0, 0, 0], [1, 1], [0, 0, 0]]
[[0, 0, 0, 0, 0, 0], [1, 1], [0, 0]]
[[0, 0, 0, 0, 0, 0, 0], [1, 1], [0]]
[[1], [0, 0, 0, 0, 0, 0, 0, 0], [1]]
'''
try:
    s = SBPSched(2, [8,2], 3, [[0, 0, 0], [1]])
    test(s)
except Exception as e:
    print(e,'\n')

print("-----Test 6.5--------")
'''
[[0, 0, 0], [1, 1], [0, 0, 0, 0, 0]]
[[0, 0, 0, 0], [1, 1], [0, 0, 0, 0]]
[[0, 0, 0, 0, 0], [1, 1], [0, 0, 0]]
[[0, 0, 0, 0, 0, 0], [1, 1], [0, 0]]
[[0, 0, 0, 0, 0, 0, 0], [1, 1], [0]]
[[1], [0, 0, 0, 0, 0, 0, 0, 0], [1]]
'''
try:
    s = SBPMESched(2, [8,2], 3, [[0, 0, 0], [1]], fix_prefix = False)
    test(s)
except Exception as e:
    print(e,'\n')

print("-----Test 6.6--------")
'''
[[0, 0, 0], [1], [0, 0, 0, 0, 0]]
[[0, 0, 0, 0], [1, 1], [0, 0, 0, 0]]
[[0, 0, 0, 0, 0], [1, 1], [0, 0, 0]]
[[0, 0, 0, 0, 0, 0], [1, 1], [0, 0]]
[[0, 0, 0, 0, 0, 0, 0], [1, 1], [0]]
[[1], [0, 0, 0, 0, 0, 0, 0, 0], [1]]
'''
try:
    s = SBPMESched(2, [8,2], 4, [[0, 0, 0], [1]], fix_prefix = False)
    test(s)
except Exception as e:
    print(e,'\n')

print("-----Test 6.7--------")
'''
[[0, 0, 0], [1], [0, 0, 0, 0, 0]]
[[0, 0, 0, 0], [1, 1], [0, 0, 0, 0]]
[[0, 0, 0, 0, 0], [1, 1], [0, 0, 0]]
[[0, 0, 0, 0, 0, 0], [1, 1], [0, 0]]
[[0, 0, 0, 0, 0, 0, 0], [1, 1], [0]]
[[1], [0, 0, 0, 0, 0, 0, 0, 0], [1]]
'''
try:
    s = SBPMESched(2, [8,3], 7, [[0, 0, 0], [1]], fix_prefix = True)
    test(s)
except Exception as e:
    print(e,'\n')

print("-----Test 6.8--------")
'''
[[0, 0, 0], [1], [0, 0, 0, 0, 0]]
[[0, 0, 0, 0], [1, 1], [0, 0, 0, 0]]
[[0, 0, 0, 0, 0], [1, 1], [0, 0, 0]]
[[0, 0, 0, 0, 0, 0], [1, 1], [0, 0]]
[[0, 0, 0, 0, 0, 0, 0], [1, 1], [0]]
[[1], [0, 0, 0, 0, 0, 0, 0, 0], [1]]
'''
try:
    s = SBPMESched(0, [3, 2, 1, 2], 3)
    test(s)
except Exception as e:
    print(e,'\n')

print("-----Test 7---------")
'''
[[0, 0], [1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19]]
[[1, 1], [0, 0, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19]]
[[2, 2], [0, 0, 1, 1, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19]]
[[3, 3], [0, 0, 1, 1, 2, 2, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19]]
[[4, 4], [0, 0, 1, 1, 2, 2, 3, 3, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19]]
[[5, 5], [0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19]]
[[6, 6], [0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 7, 7, 8, 8, 9, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19]]
[[7, 7], [0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 8, 8, 9, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19]]
[[8, 8], [0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 9, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19]]
[[9, 9], [0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19]]
[[10], [0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19]]
[[11], [0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 12, 13, 14, 15, 16, 17, 18, 19]]
[[12], [0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 13, 14, 15, 16, 17, 18, 19]]
[[13], [0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12, 14, 15, 16, 17, 18, 19]]
[[14], [0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12, 13, 15, 16, 17, 18, 19]]
[[15], [0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12, 13, 14, 16, 17, 18, 19]]
[[16], [0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12, 13, 14, 15, 17, 18, 19]]
[[17], [0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12, 13, 14, 15, 16, 18, 19]]
[[18], [0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12, 13, 14, 15, 16, 17, 19]]
[[19], [0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18]]
'''
s = SBPMESched(20, [2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], 2)
test(s)

print("-----Test 8---------")
'''
SBPSched will make progress; however, the number of schedules is too large!
'''
s = SBPSched(20, [2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], 20)
# test(s, False)

print("-----Test 8.1---------")
'''
SBPMESched with smaller number of peridos seems to work
'''
s = SBPMESched(20, [2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], 3)
test(s, False)

print("-----Test 9---------")
'''
Test 9
'''
s = SBPMESched(4, [2, 2, 2, 1], 4)
test(s)

print("-----Test 10---------")
'''
Test 10
'''
s = SBPMESched(2, [9, 5], 5, [[0, 0], [1]], fix_prefix=True)
for i in s:
    print(i)

print("-----Test 11---------")
'''
Test 10
'''
s = SBPMESched(4, [18, 6, 5, 8], 3, [[0]], fix_prefix=True)
for i in s:
    print(i)