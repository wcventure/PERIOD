#!/usr/bin/env python3

import subprocess, time
from datetime import datetime

N = 20

rlts = []
startTime = time.time()
lastTime  = time.time()
rounds = lastRound = 0

def formateTime(seconds):
    hours, rem = divmod(seconds, 3600)
    minutes, s = divmod(rem, 60)
    return '{:0>2}:{:0>2}:{:08.5f}'.format(int(hours), int(minutes), s)

for i in range(0, N):
    for j in range(0, N):
        process = subprocess.Popen(
            ['./increase_double_sleep', str(i), str(j)],
            stdout=subprocess.PIPE)
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
