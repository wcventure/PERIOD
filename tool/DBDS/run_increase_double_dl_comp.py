#!/usr/bin/env python3

import subprocess, time, interleaving
from datetime import datetime

N = 2   # number of threads
M = 3   # number of key points per thread

rlts = []
startTime = time.time()
lastTime  = time.time()
rounds = lastRound = 0

def formateTime(seconds):
    hours, rem = divmod(seconds, 3600)
    minutes, s = divmod(rem, 60)
    return '{:0>2}:{:0>2}:{:08.5f}'.format(int(hours), int(minutes), s)

inters = interleaving.interleaving_gen(N, M, True)
for i in inters:
    pattern = interleaving.yield_pattern_gen(N, i, True)
    cmd = ['sudo', './increase_double_dl_comp', str(N), str(M)]
    for t in pattern:   # for each thread
        for k in t:     # for each key point
            cmd.append(str(k))

    process = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    stdout, _ = process.communicate()
    x = stdout.split()[-1]
    rounds += 1
    if(not x in rlts):
        rlts.append(x)
        lastRound = rounds
        lastTime = time.time()

print(len(rlts), 'results found:')
print(rlts)
print('-' * 50)
print('\tLast New Find\t\tTotal')
print('Round\t{0}\t\t\t{1}'.format(lastRound, rounds))
print('Time\t' + formateTime(lastTime - startTime), end = "")
print('\t\t' + formateTime(time.time() - startTime))
