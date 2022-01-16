import itertools

def interleaving_count(n, M, to_print = False):
    '''
    Parameters: n is the number of threads. M is a list, holding the number
    of key points for each thread. If len(M) is smaller than n, M[-1] will
    be used as the default. If the to_print is set to True, the calculations
    result will be printed on the screen.
    Returns: The function returns the total number of interleaving for the
    given specifications.
    '''
    total_m = 0
    rlt = 1

    for i in range(n):
        if i < len(M):
            total_m += M[i]
        else:
            total_m += M[-1]

    for i in range(1, total_m + 1):
        rlt *= i

    for i in range(n):
        if i < len(M):
            for j in range(1, M[i] + 1):
                rlt = rlt // j
        else:
            for j in range(1, M[-1] + 1):
                rlt = rlt // j

    if to_print:
        print("Number of threads:", n)
        print("Number of key points in each thread:", end = " ")
        for i in range(n):
            if i < len(M):
                print(M[i], end = " ")
            else:
                print(M[-1], end = " ")
    print("\nNumber of interleaving:", rlt)

    return rlt


# generate all possible interleaving for n threads,
# with at most m key points in each thread
def interleaving_gen(n, m, to_print_num = False, to_print_rlt = False):
    kp_pool = []

    # populate the key points to be scheduled
    for i in range(n):
        for _ in range(m):
            kp_pool.append(i)

    inters = [] # the list holes possible interleaving

    # generate 'raw' interleaving, with possible duplications
    for i in itertools.permutations(kp_pool):
        inters.append(list(i))

    # deduplication
    inters.sort()
    inters = list(i for i,_ in itertools.groupby(inters))

    # print the number of interleaving found
    if to_print_num:
        print('Number of interleaving: ', len(inters))

    # print the interleaving found
    if to_print_rlt:
        for i in inters:
            print(i)

    return inters


def interleaving_init(n, M):
    '''
    This function takes n as the number of threads, and a list of integers
    M that contains the number of key points for each thread. If len(M) is
    smaller than n, the last specified integer number in M will be used as
    the default.
    Returns: The function returns the first (the smallest, according to the
    dictionary order) interleaving for the given specification.
    '''
    rlt = []
    for i in range(n):
        if i < len(M):
            m = M[i]
        else:
            m = M[-1]
        for _ in range(m):
            rlt.append(i)
    return rlt


def interleaving_next(inter):
    '''
    The function takes an interleaving, represented by a list of thread IDs,
    then it calculates the NEXT interleaving in terms of the dictionary
    order.
    Returns: the new interleaving will be calculated in-place, that is, the
    argument itself is changed to represent the new interleaving (if there's
    any); meanwhile, the returned boolean value is used to indicate if the
    new interleaving exists.
    '''
    a = b = -1
    tmp = []
    for i in range(len(inter) - 1, 0, -1):
        tmp.append(inter[i])
        if inter[i] > inter[i - 1]:
            a = i - 1
            b = i
            break

    if a < 0:
        return False

    for i in tmp:
        inter[b] = i
        b += 1

    b = a + 1
    while inter[b] <= inter[a]:
        b += 1

    tmp = inter[a]
    inter[a] = inter[b]
    inter[b] = tmp

    return True
    # NOTE: array is called by reference, thus the return value is only
    # used to indicate if the next interleaving exists


# Deprecated
def strip_suffix(inter):
    '''
    This function takes an interleaving as input and
    (in-place) strips its longest dictionary order suffix
    '''
    while len(inter):
        if len(inter) > 1:
            if inter[-1] < inter[-2]:
                inter.pop()
                break
            inter.pop()
        else:
            inter.pop()


# in use
def get_prefix(inter):
    '''
    This function takes an interleaving as input and returns a copy
    of its prefix without the longest dictionary ordered suffix
    '''
    cs = 1   # the count of elements in the dictionary ordered suffix
    while cs < len(inter):
        if inter[-(cs + 1)] > inter[-cs]:
            break
        cs += 1
    return inter[:len(inter) - cs]


def interleaving_iter(n, M, parent = []):
    '''
    This function takes n as the number of threads, a list of integers M
    that contains the number of key points for each thread. If len(M) is
    smaller than n, the last specified integer number in M will be used as
    the default.
    If an optional argument ``parent'' is given, the function will only
    calculate interleaving with the same prefix as their parent's.

    Returns:
    The function is a ``generator'', therefore it creates an iterator.
    Every time the iterator is visited, it yields the smallest, according
    to the dictionary order, interleaving for the given specification.
    '''
    while len(M) < n:           # padding the M
        M.append(M[-1])

    prefix = get_prefix(parent) # keep only the prefix of parent interleaving

    for i in prefix:            # calculate the remaining kps to be scheduled
        try:
            M[i] -= 1
        except IndexError:
            print('Warning: Too many threads in parent interleaving!')


    suffix = interleaving_init(n, M)

    while True:
        yield prefix + suffix   # return an interleaving with parent's prefix
        if not interleaving_next(suffix):
            break


def yield_pattern_gen(n, inter, to_print = False):
    '''
    this function calculates which yield pattern should be used to
    get the desired interleaving. A yield pattern is a matrix, indicating
    how many periods should a thread skip, before reaching a certain
    key point with in that thread.
    n is the number of threads, inter is the desired interleaving
    '''
    pattern = []    # the yield pattern
    td_period = []  # record which period-block a thread lastly used
    lastT = 0       # last thread scheduled
    lastP = 0       # last period-block used
    # initially, all threads resident in period 0
    # and patterns for all threads are empty
    for _ in range(n):
        td_period.append(0)
        pattern.append(list())

    # set the last scheduled thread to be the
    # first thread in the desired interleaving
    lastT = inter[0]
    # for each key point instruction,
    for i in inter:
        # if it's NOT from the same thread with the last scheduled thread
        # kick it to the next period-block, to avoid uncertainty in execution
        if not i == lastT:
            lastP += 1  # mark that a new period-block is used
            pattern[i].append(lastP - td_period[i])
            td_period[i] = lastP
            lastT = i
        else:
            pattern[i].append(0)

    # print the interleaving and corresponding
    # yiled patterns should be used to get there
    if to_print:
        for i in pattern:
            print(i, end = "")
        print()

    return pattern

def yp_to_inter(n, M, YP):
    '''
    Calculate the actual interleaving based on the number of threads: n,
    (new) key points in all the threads: M, and (old) Yield Pattern: YP.

    If len(M) is smaller than n, the last specified integer number in M
    will be used as the default.

    If there're too many kps for the given yield patter, the kps come
    late will be assumed not to yield.

    A `well-formed` Yield Pattern should ensure no two threads fall into
    a same running period. But this function can also deal with ill-formed
    YPs, in which case, this function will arrange threads in dictionary
    order if they fall into a same running period.
    '''
    while len(M) < n:   # padding the M
        M.append(M[-1])
    while len(YP) < n:
        YP.append([0])  # padding the Yield Patterns
    for i in range(n):
        while len(YP[i]) < M[i]:
            YP[i].append(0)

    inter = []          # the interleaving calculated
    sched = [0] * n     # key points have been scheduled for each thread
    cont = True         # should we continue?

    while cont:         # each round, we consider one running period
        cont = False
        for t in range(n): # each round, we consider a thread
            # if there're kps left in current thread ..
            while sched[t] < M[t]:
                cont = True # we found something to do, we should continue
                # if it's time to yield ..
                if YP[t][sched[t]] > 0:
                    YP[t][sched[t]] -= 1
                    break
                sched[t] += 1
                inter.append(t)

    return inter
