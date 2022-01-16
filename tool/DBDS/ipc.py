'''
This module is for inter-process communication between Python
script and the (instrumented) C programs to be tested.
'''

from sysv_ipc import *  # Doc: http://semanchuk.com/philip/sysv_ipc
                        # INSTALL: pip3 install sysv_ipc
from ctypes import *
from abc import ABC, abstractmethod

class Shm(ABC):
    '''
    This is an Abstract Base Class (ABC) that represents a
    shared Memory used for communication between Python and C
    '''

    def __init__(self, data_type, size, key, flag = IPC_CREAT, mode = 0o666):
        '''
        It's a wrapper of the sysv_ipc's shm creation function. It setups a
        piece of UNIX IPC shared memory, attaches the current process to it,
        and returns the shm object. The data_type parameter is the type of the
        data residents in the shared memory. For the information on the other
        parameters, check the OS manual:
        man shmget
        man shmat
        '''
        self.data_type = data_type
        try:
            self.shm = SharedMemory(key, flag, mode, size, b'\x00')
        except Exception as e:
            print("Something is wrong. Check the specified 'size'; or manually"
                + "\ndelete the shared memory by using: ipcrm -M IPC_KEY ;"
                + "\nor use a different IPC_KEY instead.")
            raise e

    @abstractmethod
    def store(self):
        pass

    @abstractmethod
    def load(self):
        pass

    def __del__(self):
        self.shm.remove()

'''
Global variables for scheduling information
'''
SHM_THREAD_NUM = 10
SHM_ARRAY_SIZE = 200

class Sched_Info(Structure):
    '''
    This class represents the following C Sched_Info structure:
    # typedef struct Sched_Info {
    #	uint64_t time;										// the execution time of the program (nsec)
    #	unsigned int N;										// number of threads concerned
    #	unsigned int M;										// number of kep points per thread
    #	unsigned int tupleScheduling;						// now considering tuple scheduling :1, otherwise: 0; Then it save the hash ID of first touched key point
    #	unsigned int kpNumArray[SHARE_MEMORY_THREAD_NUM];	// the specific kpNum of each My_tid Thread, only consider almost 10 thread
    #	unsigned int kp[SHARE_MEMORY_ARRAY_SIZE];			// how many times should a thread yield before a kp
    #	unsigned int kpLoc[SHARE_MEMORY_ARRAY_SIZE];		// the hash ID of Key Point
    #	unsigned int kpOrder[SHARE_MEMORY_ARRAY_SIZE];		// recored the order of key points
    # } sched_info;
    '''
    _fields_ = [
        ('time', c_ulonglong),
        ('N', c_uint),
        ('M', c_uint),
        ('tupleScheduling', c_uint),
        ('kpNumArray', c_uint * SHM_THREAD_NUM),
        ('kp', c_uint * SHM_ARRAY_SIZE),
        ('kpLoc', c_uint * SHM_ARRAY_SIZE),
        ('kpOrder', c_uint * SHM_ARRAY_SIZE)
    ]

    def set(self, **fields):
        '''
        Change the values in a field or fields of the data structure
        '''
        for i in fields:
            if i == 'time':
                setattr(self, i, (c_ulonglong)(fields[i]))
            elif i in ('N', 'M', 'tupleScheduling'):
                setattr(self, i, (c_uint)(fields[i]))
            elif i == 'kpNumArray':
                setattr(self, i, (c_uint * SHM_THREAD_NUM)(fields[i]))
            elif i == ('kp', 'kpLoc', 'kpOrder'):
                setattr(self, i, (c_uint * SHM_ARRAY_SIZE)(fields[i]))
            else:
                raise Exception("Unknown field:" + i)

class Shm_Sched_Info(Shm):
    '''
    This class encapsulates the functions from PIC shared memory and the data
    structure information about Sched_Info struct. Therefore, Python script can
    use it to talk with the DBDS instrumented C programs.
    '''
    def __init__(self, key):
        self.data_size = sizeof(Sched_Info)
        Shm.__init__(self, Sched_Info, self.data_size, key)

    # Deprecated: No longer in need as padding will be done automatically
    #             when converting the data to the structure formate.
    def __pad_array(self, array, length, val = 0):
        '''
        Pad the array to the specified length, by appending the value val
        '''
        array += [val] * (length - len(array))

    def store(self, data : Sched_Info):
        '''
        Store the Sched_Info data into shared memory
        '''
        if not isinstance(data, Sched_Info):
            error = TypeError("The type of data must be Sched_Info.")
            raise error
        self.shm.write(data)

    def store_fileds(self, **fields):
        '''
        Store the specified fields of Sched_Info data into shared memory
        '''
        tmp = Sched_Info()
        tmp.set(fields)

        for i in fields:
            if i == 'time':
                self.shm.write((c_ulonglong)(fields[i]), Sched_Info.time.offset)
            elif i == 'N':
                self.shm.write((c_uint)(fields[i]), Sched_Info.N.offset)
            elif i == 'M':
                self.shm.write((c_uint)(fields[i]), Sched_Info.M.offset)
            elif i == 'tupleScheduling':
                self.shm.write((c_uint)(fields[i]), Sched_Info.tupleScheduling.offset)
            elif i == 'kpNumArray':
                self.shm.write((c_uint * SHM_THREAD_NUM)(fields[i]), Sched_Info.kpNumArray.offset)
            elif i == 'kp':
                self.shm.write((c_uint * SHM_ARRAY_SIZE)(fields[i]), Sched_Info.kp.offset)
            elif i == 'kpLoc':
                self.shm.write((c_uint * SHM_ARRAY_SIZE)(fields[i]), Sched_Info.kpLoc.offset)
            elif i == 'kpOrder':
                self.shm.write((c_uint * SHM_ARRAY_SIZE)(fields[i]), Sched_Info.kpOrder.offset)

    def load(self):
        '''
        Load the Sched_Info data from the shared memory, and return
        it as a NEW python struct object of the type Sched_Info
        '''
        b = bytearray()
        b[:] = self.shm.read(self.data_size)
        return Sched_Info.from_buffer(b)

    def load_field(self, field, convert_array = False):
        '''
        Load the specified filed of Sched_Info data from the shared memory,
        and return it. When 'convert_array' is set to be True, a C array
        will be converted into python's list object.
        '''
        rlt = getattr(self.load(), field)
        if not convert_array:
            return rlt
        if field in ('kpNumArray', 'kp', 'kpLoc', 'kpOrder'):
            tmp = rlt
            rlt = []
            for i in tmp:
                rlt.append(i)
        return rlt
