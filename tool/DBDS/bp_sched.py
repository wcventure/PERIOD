'''
This module represents the Bounded-Periods scheduling method. By using this
method, every generated scheduling has exictly the specified number of periods.
Within each period, key points from different threads may be executed in
parallel, while a sequential order is enforced for key points from different
periods.
'''
from sched import Sched
from pds import Pds

class BPSched(Sched, Pds):

    def __init__(self, n = 1, kps = [0], p = 1, sched = None):
        super()._init_helper(n, kps, p, sched)

        for i in self._sched:
            if not i:
                raise ValueError("Invalid period(s) in the given sched!")
        self._iter = self._first_iter

        if len(self._sched) >= self._p and sum(self._kp_pool) != 0:
            raise ValueError("The given sched can't host all key points!")

        if self._sched:
            self._pattern = list(map(len, self._sched))
            nkp = sum(self._kp_pool)     # num of kps

            if nkp == 0:
                self._norm_sched()
                return

            np = self._p - len(self._sched)   # num of peridos
        else:
            self._pattern = []
            nkp = sum(self._kps)
            np = self._p

        sub_pattern = self._init_pattern(nkp, np)
        sub_sched = self._init_sched(sub_pattern, self._kp_pool)
        self._sched.extend(sub_sched)
        self._pattern.extend(sub_pattern)
        self._norm_sched()

    def _init_pattern(self, nkp, np):
        rlt = [1] * (np - 1)
        rlt.append(nkp - np + 1)
        return rlt

    def _next_pattern(self, pattern):
        np = len(pattern)  # number of periods
        if np == 1:
            return None

        rlt = [pattern[0]]

        sub_pattern = self._next_pattern(pattern[1:])
        if sub_pattern:
            rlt.extend(sub_pattern)
            return rlt

        nkp = sum(pattern[1:])
        if nkp == np - 1:
            return None

        rlt[0] += 1
        rlt.extend(self._init_pattern(nkp - 1, np - 1))
        return rlt

    def _init_sched(self, pattern, kp_pool):
        rlt = []
        n = 0   # thread ID to traverse the kp_pool
        for p in range(len(pattern)):
            rlt.append([])
            while len(rlt[-1]) < pattern[p]:
                while kp_pool[n] <= 0:
                    n += 1
                rlt[-1].append(n)
                kp_pool[n] -= 1
        return rlt

    def _next_sched(self, sched, pattern, kp_pool):
        if len(pattern) <= 1:
             return None

        rlt = [sched[0]]

        sub_sched = self._next_sched(sched[1:], pattern[1:], self._kp_pool)
        if sub_sched:
            rlt.extend(sub_sched)
            return rlt

        for i in sched[1]:
            kp_pool[i] += 1

        found = False
        for i in range(pattern[0] - 1, -1, -1):
            a = rlt[0][i]
            for b in range(a + 1, self._n):
                if kp_pool[b] > 0:
                    rlt[0][i] = b
                    kp_pool[a] += 1
                    kp_pool[b] -= 1
                    found = True
                    break
            if found:
                break

        if found:
            rlt.extend(self._init_sched(pattern[1:], self._kp_pool))
            return rlt

        return None

    def __str__(self):
        return "Number of periods: {}\n".format(self._p) + super().__str__()

    def _first_iter(self):
        self._iter = self._next_iter
        rlt  = [x.copy() for x in self._sched]
        return rlt

    def _next_iter(self):
        self._sched = self._next_sched(self._sched, self._pattern,
                                       self._kp_pool)
        if self._sched == None:
            self._pattern = self._next_pattern(self._pattern)
            if self._pattern == None:
                raise StopIteration()
            else:
                self._kp_pool = self._kps.copy()
                self._sched = self._init_sched(self._pattern,
                                               self._kp_pool)

        rlt  = [x.copy() for x in self._sched]
        return rlt

    def next(self):
        rlt  = self._iter()
        return rlt

    def get_yield_pattern(self):
        if self._sched == None:
            return None
        pattern = []
        idx = [0] * self._n
        for i in range(self._n):
            pattern.append([0] * self._kps[i])
        for p in self._sched:
            for n in p:
                idx[n] += 1
            for i in range(self._n):
                try:
                    pattern[i][idx[i]] += 1
                except:
                    pass
        return pattern
