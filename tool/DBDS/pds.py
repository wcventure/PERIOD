'''
This module is an abstrct class (interface)
for periodical deterministic scheduling.
'''

from abc import ABC, abstractmethod

class Pds(ABC):
    # periodical deteministatic scheduling
    @abstractmethod
    def get_yield_pattern(self):
        pass
