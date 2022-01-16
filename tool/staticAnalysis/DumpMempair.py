#!/usr/bin/env python3

import os
import sys
import argparse
import getopt
import re
import itertools
import numpy as np

PREFIX = ""

mssa = 1
aset = 0

# get argv
outputNpzFile = ""
try:
    opts, args = getopt.getopt(sys.argv[1:], "ho:", ["help", "outputNpzFile="])

except getopt.GetoptError:
    print('Error: DumpMempair.py -o <outputNpzFile>')
    sys.exit(2)

for opt, arg in opts:
    if opt in ("-h", "--help"):
        print('DumpMempair.py -o <outputNpzFile>')
        sys.exit()
    elif opt in ("-o", "--output"):
        outputNpzFile = arg

if outputNpzFile == "":
    print('Error: Command line is empty')
    print('Tips: Using -h to view help')
    sys.exit(2)
    

def remove_column(text):
    if aset:
        return text
    toks = text.split(':')
    toks[2] = '0'
    return ':'.join(toks)

def strip_start(text, prefix):
    if not text.startswith(prefix):
        return text
    ret = text[-(len(text) - len(prefix)):]
    if ret.startswith("./"):
        return ret[2:]
    return ret

class Instruction:
    def __init__(self):
        self.load_from = set()
        self.store_to = set()
        self.source_loc = None
        self.poitner_type = None

    def is_integer(self):
        return self.poitner_type in ['i8', 'i16', 'i32', 'i64']

    def is_general_pointer(self):
        return self.poitner_type in ['i8*', 'i16*', 'i32*', 'i64*']

    def extract_type(self, line):
        typ = line[line.find("({") + 2 : line.find("})")]
        for regex in re.findall('struct\.[^\ ]*\.[0-9]+[^\ \*]*', typ):
            newregex = re.sub(r'\.[0-9]+$', '', regex)
            typ = typ.replace(regex, newregex)
        for regex in re.findall('\.[0-9]+:', typ):
            newregex = re.sub(r'\.[0-9]+:', ':', regex)
            typ = typ.replace(regex, newregex)
        self.poitner_type = typ

    def extract_source_location(self, line):
        loc = line.strip().split("[[")[1]
        if loc.find("@[") != -1:
            # It is inlined at somewhere, but I don't care where it is inlined at
            delim = "@["
        else:
            # No inlined
            delim = "]]"
        self.source_loc = loc.split(delim)[0].strip()

    def __parse_pts(line):
        line = line.strip()
        line = line[line.index("{") + 1 : len(line) - 1]
        return set(map(int, line.split()))

    def feed_line(self, line, is_write):
        pts = Instruction.__parse_pts(line)
        if is_write:
            self.store_to |= pts
        else:
            self.load_from |= pts

    def get_accessed_memory_location(self):
        return list(zip(self.store_to, [True]*len(self.store_to))) + \
                list(zip(self.load_from, [False]*len(self.load_from)))

    def get_source_location(self):
        return remove_column(strip_start(self.source_loc, PREFIX))

    def get_pointer_type(self):
        return self.poitner_type

class MemoryLocation:
    def __init__(self, id):
        self.id = id
        self.load_insn = set()
        self.store_insn = set()
        self.so_insn = set()

    def add_instruction(self, insn, is_write):
        source_loc = insn.get_source_location()[:-2]
        if is_write:
            self.store_insn.add(source_loc)
        else:
            self.load_insn.add(source_loc)
            
    # wcventure added
    def add_source_loc(self, source_loc, flag):
        if flag == 1:
            self.store_insn.add(source_loc)
        elif flag == 2:
            self.load_insn.add(source_loc)
        elif flag ==3:
            self.so_insn.add(source_loc)
        else:
            pass

    def generate_result(self):
        if aset:
            return self.__generate_aliased_set()
        else:
            return self.__generate_mempair()

    def __generate_aliased_set(self):
        return self.id, \
                list(self.load_insn) + list(self.store_insn), \
                ['R']*len(self.load_insn) + ['W']*len(self.store_insn)

    def __generate_mempair(self):
        st_st = list(itertools.product(self.store_insn, self.store_insn))
        st_ld = list(itertools.product(self.store_insn, self.load_insn))
        
        st_so = list(itertools.product(self.so_insn, self.so_insn))
        st_sost = list(itertools.product(self.so_insn, self.store_insn))
        st_sold = list(itertools.product(self.so_insn, self.load_insn))
        
        return self.id, \
                (st_st + st_ld + st_so + st_sost + st_sold), \
                [('W', 'W')]*len(st_st) + [('W', 'R')]*len(st_ld) + [('S', 'S')]*len(st_so) + [('S', 'W')]*len(st_sost) + [('S', 'R')]*len(st_sold)

class MempairResult:
    def __init__(self):
        self.deduped_mempair = {}

    def __sort(mempair, typ):
        if mempair[0] > mempair[1]:
            return (mempair[1], mempair[0]), (typ[1], typ[0])
        return mempair, typ

    def add(self, locid, mempairs, types):
        for mempair,typ in zip(mempairs, types):
            sorted_mempair, sorted_typ = MempairResult.__sort(mempair, typ)
            sorted_typ = sorted_typ
            self.deduped_mempair[sorted_mempair] = sorted_typ

    def print_all(self):
        for mempair, typ in sorted(self.deduped_mempair.items()):
            print(mempair[0], mempair[1], typ[0], typ[1])

class AliasedSetResult:
    def __init__(self):
        self.aliased_set_per_memloc = {}

    class AliasedSet:
        def __init__(self, aliased_set, typ):
            self.set = set()
            for source_loc, typ in zip(aliased_set, types):
                self.set.add((source_loc, typ))

        def __iter__(self):
            return self.set.__iter__()

    def __is_subset(self, sub_memlocid, super_memlocid):
        for insn in self.aliased_set_per_memloc[sub_memlocid]:
            if not insn in self.aliased_set_per_memloc[super_memlocid]:
                return False
        return True

    def __remove_duplicate(self, memlocid):
        for other_memlocid in list(self.aliased_set_per_memloc):
            if other_memlocid != memlocid and self.__is_subset(memlocid, other_memlocid):
                del self.aliased_set_per_memloc[memlocid]
                return
        for other_memlocid in list(self.aliased_set_per_memloc):
            if other_memlocid != memlocid and self.__is_subset(other_memlocid, memlocid):
                del self.aliased_set_per_memloc[other_memlocid]

    def add(self, memlocid, aliased_set, types):
        if len(aliased_set) == 1:
            return
        self.aliased_set_per_memloc[memlocid] = AliasedSetResult.AliasedSet(aliased_set, types)
        self.__remove_duplicate(memlocid)

    def print_all(self):
        for memlocid, aliased_set in sorted(self.aliased_set_per_memloc.items()):
            print("[Memory location ID: %d]" % memlocid[0])
            for insn in sorted(aliased_set, key = lambda x: (x[1], x[0])):
                print("\tType: ", insn[1], "\t", insn[0])

if __name__ == '__main__':
    
    # load npz data
    NPZ = np.load("soAndGroup.npz",allow_pickle=True)
    pureSoList = NPZ["arr_0"].tolist()
    memGroupList = NPZ["arr_1"].tolist()
    
    # ransform memory_locations
    memory_locations = {}
    for key in memGroupList:
        if not key[0] in memory_locations:
            memory_locations[key[0]] = MemoryLocation(key[0])
        for each in key[1]:
            memory_locations[key[0]].add_source_loc(each,1)
        for each in key[2]:
            memory_locations[key[0]].add_source_loc(each,2)
        for each in key[3]:
            memory_locations[key[0]].add_source_loc(each,3)
    
    # generate mempairs
    if aset:
        result = AliasedSetResult()
    else:
        result = MempairResult()
        
    for key in memory_locations:
        #print(memory_locations[key].generate_result())
        locid, results, types = memory_locations[key].generate_result()
        result.add(locid, results, types)
        
    # get Prune mempair
    Lines = []
    mempairList = []
    for mempair, typ in sorted(result.deduped_mempair.items()):
        Lines.append(mempair[0] +' '+ mempair[1] +' '+ typ[0] +' '+ typ[1])
    
    for line in Lines:
        line = line.strip()
        toks = line.split()

        if len(toks) != 2 and len(toks) != 4 and len(toks) != 6:
            sys.stderr.write("\t [ERROR] Wrong mempair format\n")
            exit(1)

        # functions in header files might be inlined many times.
        # Ignore it for now
        # TODO: handle it correctly
        if toks[0].find('.h') != -1 or toks[1].find('.h') != -1:
            continue

        dirs = [os.path.dirname(x) for x in toks]

        # TODO: we may need to handle "include" dir.
        if (dirs[0] in dirs[1]) or (dirs[1] in dirs[0]):
            mempairList.append(toks)
        else:
            # skip if two paths are in a different sub-directory.
            pass

    # update mempairList due to pureSoList, and add hash value for source_loc into mempairList
    for each in mempairList:
        each.append('')
        each.append('')
        for eachSo in pureSoList:
            if each[0] == eachSo[0]:
                each[4] = eachSo[3]
                if each[2] == 'S':
                    each[2] = eachSo[1]
            if each[1] == eachSo[0]:
                each[5] = eachSo[3]
                if each[3] == 'S':
                    each[3] = eachSo[1]
    
    # save npz
    npPureSoList = np.array(pureSoList)
    npMemGroupList = np.array(memGroupList)
    npmemPairList = np.array(mempairList)
    np.savez(outputNpzFile,npPureSoList,npMemGroupList,npmemPairList)

    # print
    for each in mempairList:
        print(each[0],each[1],each[2],each[3],each[4],each[5])
