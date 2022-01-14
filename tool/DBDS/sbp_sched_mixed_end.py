'''
This module represents a Serialised-Bounded-Periods scheduling method.
By using this method, every generated scheduling has exictly
the specified number of periods, p.
For the first p - 1 periods, ONLY kps from a same thread can be in a same
period, while the last period may contain key points from various threads
running in parallel.
'''

import itertools
from sched import Sched
from sbp_sched import SBPSched
from pds import Pds

class SBPMESched(SBPSched):

    def __init__(self, n = 0, kps = [0], p = 1,
                 sched = None, pattern = None, fix_prefix = False):
        Sched._init_helper(self, n, kps, p, sched)
        for i in self._sched:
            if not i:
                raise ValueError("Invalid period(s) in the given sched!")
        self._pattern = []
        self._iter = self._first_iter
        lastPattern = -1
        pre = 1

        if pattern or sched:
            self._init_helper(self._n, self._kps, self._p, sched, pattern)
            if self._sched:
                if len(self._sched) == self._p - 1:
                    if self._kp_pool[self._sched[-1][-1]] > 0:
                        raise ValueError("Can't finish initialising pattern!")
                if len(self._sched) == self._p:
                    if sum(self._kp_pool) > 0:
                        raise ValueError("Can't finish initialising!")
                lastPattern = self._pattern[-1]
                pre = len(self._sched)

        if len(self._pattern) != self._p:
            # actually we're saying self._pattern < self._p, which
            # means that there're some peridos need to be initialised
            self._chosen = []
            sub_pattern = []

            while True:
                self._chosen = self._next_chosen(self._n,
                        self._p - len(self._pattern), self._chosen)
                if not self._chosen:
                    raise ValueError("Can't finish initialising pattern!")

                # calculate working key points
                wkps = self._cal_ckps(self._n,self._kp_pool,self._chosen)
                for i in self._pattern[len(self._sched):]:
                    wkps[self._pattern[i]] -= 1

                sub_pattern = self._init_pattern(
                        self._n, wkps, self._p - len(self._pattern),
                        set(self._chosen) -
                            set(self._pattern[len(self._sched):]),
                        lastPattern)

                if sub_pattern:
                    break

            for i in self._pattern[len(self._sched):]:
                if i not in self._chosen:
                    self._chosen.append(self._pattern[i])
            self._chosen.sort()

            self._pattern += sub_pattern

        self._ckps = self._cal_ckps(self._n, self._kp_pool, self._chosen)
        self._sched_chosen, self._sched_other = self._init_sched_(
                self._n, self._kp_pool, self._ckps,
                self._p - len(self._sched),
                self._pattern[len(self._sched):])

        # indicating the starting period
        self._start = 0
        if fix_prefix:
            self._start = len(self._sched)
            self._last = self._sched[-1][-1]
            self._lastCount = len(self._sched[-1])
        else:
            self._kp_pool = self._kps.copy()
            self._ckps = self._cal_ckps(self._n, self._kp_pool, self._chosen)
            for i in self._pattern[:len(self._sched)]:
                if i not in self._chosen:
                    self._chosen.append(self._pattern[i])
            self._chosen.sort()

        self._sched += self._sched_chosen
        self._sched_chosen = [x.copy() for x in self._sched]
        self._sched[-1].extend(self._sched_other)

        self._prefix = [x.copy() for x in self._sched[:pre]]

    def _next_chosen(self, n, maxNum, chosen):
        num = len(chosen)   # number of threads chosen to interleave
        for i in range(num - 1, -1, -1):
            curr = chosen[i]
            if curr < n + i - num:
                curr += 1
                return chosen[:i] + list(range(curr, curr + num - i))

        num += 1
        # the num of threas chosen can't get bigger than a maxNum
        # (which is subject to the number of peridos), or the total
        # number of threads
        if num > maxNum or num > n:
            return None

        return list(range(num))

    ''' Update the chosen key points to match the new set of chosen threads'''
    def _cal_ckps(self, n, kps, chosen):
        ckps = [0] * n
        for i in chosen:
            ckps[i] = kps[i]
        return ckps

    # NOTE: the _ after the name is used to avoid closion
    # with the recusive method provided in the super class
    def _init_sched_(self, n, kps, ckps, p, pattern):
        rlt1 = super()._init_sched(n, ckps, p, pattern)
        rlt2 = []
        for i in range(n):
            if ckps[i] == 0:
                rlt2.extend([i] * kps[i])
        return rlt1, rlt2

    def _next_iter(self):
        if self._start > 0:
            ckps = self._ckps.copy()
            ckps[self._last] += self._lastCount
            p = self._p - self._start + 1
            sched = self._sched_chosen[self._start - 1:]
            pattern = self._pattern[self._start - 1:]
        else:
            ckps = self._ckps.copy()
            p = self._p
            sched = self._sched_chosen.copy()
            pattern = self._pattern.copy()
        sched_chosen = self._next_sched(self._n, ckps, p, sched, pattern)

        while sched_chosen == None:
            pattern = self._next_pattern(
                    self._n,
                    self._ckps,
                    self._p - self._start,
                    self._pattern[self._start:],
                    set(self._chosen),
                    -1 if self._start == 0 else self._pattern[self._start - 1])

            min_dumped = min([x for x in range(self._n) if self._kp_pool[x] > 0
                    and x not in self._chosen], default = self._n)
            while (pattern != None and pattern[-1] not in pattern[:-1]
                    and pattern[-1] > min_dumped):
                pattern = self._next_pattern(
                        self._n,
                        self._ckps,
                        self._p - self._start,
                        pattern,
                        set(self._chosen),
                        -1 if self._start == 0
                            else self._pattern[self._start - 1])

            while pattern == None:
                self._chosen = self._next_chosen(self._n,
                        self._p - self._start, self._chosen)
                if self._chosen == None:
                    raise StopIteration()
                self._ckps = self._cal_ckps(self._n,self._kp_pool,self._chosen)

                pattern = self._init_pattern(
                        self._n,
                        self._ckps,
                        self._p - self._start,
                        set(self._chosen),
                        -1 if self._start == 0
                           else self._pattern[self._start - 1])

                min_dumped = min([x for x in range(self._n)
                        if self._kp_pool[x] > 0 and x not in self._chosen],
                        default = self._n)
                while (pattern != None and pattern[-1] not in pattern[:-1]
                        and pattern[-1] > min_dumped):
                    pattern = self._next_pattern(
                            self._n,
                            self._ckps,
                            self._p - self._start,
                            pattern,
                            set(self._chosen),
                            -1 if self._start == 0
                                else self._pattern[self._start - 1])

            self._pattern = self._pattern[:self._start] + pattern

            sched_chosen, self._sched_other = self._init_sched_(
                    self._n, self._kp_pool, self._ckps,
                    self._p - self._start,
                    self._pattern[self._start:])
            if self._start > 0:
                sched_chosen = [[self._last] * self._lastCount] + sched_chosen

        self._sched_chosen = self._sched_chosen[:(self._start - 1 if
                self._start > 0 else 0)] + sched_chosen
        sched = [x.copy() for x in self._sched_chosen]
        sched[-1].extend(self._sched_other)
        self._prefix = []
        for i in range(len(sched)):
            self._prefix.append(sched[i])
            if sched[i] != self._sched[i]:
                break

        self._sched = sched
        self._norm_sched()
        rlt  = [x.copy() for x in self._sched]
        return rlt
