'''
This module is the abstrct class (interface) for a scheduling method.
A shcedule is a description of how threads should interleaving/parallelised.
A scheduler is also an iteratable object, i.e., by involking the next
method, a new shceduling will be returned (if it exists).
'''

from abc import ABC, abstractmethod

class Sched(ABC):
    '''
    Class variables:
    * self._n:       the number of threads
    * self._kps:     numbers of kps in each thread
    * self._p:       the number of periods
    * self._sched:   the scheduling of kps
    * self._kp_pool: where all unscheduled kps should go
    '''

    def _init_helper(self, n = 1, kps = [0], p = 1, sched = None, lazy_kps = True):
        self._n = max(n, len(kps))
        self._kps = kps.copy()
        if sched:
            # update self._n if there're more threads appeared in sched
            self._n = max(self._n,
                             max(map(max,
                                     [x for x in sched if x]),
                                     default = 0) + 1)
            self._p = max(p, len(sched))
            self._sched = [x.copy() for x in sched]
        else:
            self._p = p
            self._sched = []

        # padding self._kps
        while len(self._kps) < self._n:
            if lazy_kps:
                self._kps.append(self._kps[-1])
            else:
                self._kps.append(0)

        self._kp_pool = self._kps.copy()


        # update kp_pool and self._kps based on the sched
        for i in self._sched:
            for j in i:
                self._kp_pool[j] -= 1

        for i in range(self._n):
            if self._kp_pool[i] < 0:
                self._kps[i] -= self._kp_pool[i]
                self._kp_pool[i] = 0

    def __str__(self):
        rlt = "Number of threads: {}\n".format(self._n)
        rlt += "Number of key points: {}\n".format(self._kps)
        self._norm_sched()
        rlt += "Current schedule: {}".format(self._sched)
        return rlt

    def get_num_of_threads(self):
        return self._n

    def get_key_points(self):
        return self._kps.copy()

    def get_num_of_periods(self):
        return self._p

    def get_schedule(self):
        return [x.copy() for x in self._sched]

    def __iter__(self):
        return self

    def __next__(self):
        return self.next()

    def _norm_sched(self):
        for i in self._sched:
            i.sort()

    @abstractmethod
    def next(self):
        pass
