from __future__ import absolute_import
from __future__ import print_function


import os
import sys
import tempfile
import hashlib

from shutil import copyfile
from .filetype import FileType
from .popenwrapper import Popen
from .arglistfilter import ArgumentListFilter

from .logconfig import logConfig

# Internal logger
_logger = logConfig(__name__)

def wcompile(mode):
    """ The workhorse, called from wllvm and wllvm++.
    """

    rc = 1

    legible_argstring = ' '.join(list(sys.argv)[1:])

    # for diffing with gclang
    _logger.info('Entering CC [%s]', legible_argstring)

    try:
        cmd = list(sys.argv)
        cmd = cmd[1:]
        builder = getBuilder(cmd, mode)

        af = builder.getBitcodeArglistFilter()

        rc = buildObjectKleer(builder)
        # phase one compile failed. no point continuing
        if rc != 0:
            _logger.error('Failed to compile using given arguments: [%s]', legible_argstring)
            return rc

        # phase two
        buildAndAttachBitcode(builder, af)

    except Exception as e:
        _logger.warning('%s: exception case: %s', mode, str(e))

    _logger.debug('Calling %s returned %d', list(sys.argv), rc)
    return rc




fullSelfPath = os.path.realpath(__file__)
prefix = os.path.dirname(fullSelfPath)
driverDir = prefix
asDir = os.path.abspath(os.path.join(driverDir, 'dragonegg_as'))


# Environmental variable for path to compiler tools (clang/llvm-link etc..)
llvmCompilerPathEnv = 'LLVM_COMPILER_PATH'

# This is the ELF section name inserted into binaries
elfSectionName = '.llvm_bc'

# (Fix: 2016/02/16: __LLVM is now used by MacOS's ld so we changed the segment name to __WLLVM).
#
# These are the MACH_O segment and section name
# The SegmentName was __LLVM. Changed to __WLLVM to avoid clashing
# with a segment that ld now uses (since MacOS X 10.11.3?)
#
darwinSegmentName = '__WLLVM'
darwinSectionName = '__llvm_bc'


# Same as an ArgumentListFilter, but DO NOT change the name of the output filename when
# building the bitcode file so that we don't clobber the object file.
class ClangBitcodeArgumentListFilter(ArgumentListFilter):
    def __init__(self, arglist):
        localCallbacks = {'-o' : (1, ClangBitcodeArgumentListFilter.outputFileCallback)}
        super(ClangBitcodeArgumentListFilter, self).__init__(arglist, exactMatches=localCallbacks)

    def outputFileCallback(self, flag, filename):
        self.outputFilename = filename


def getHashedPathName(path):
    return hashlib.sha256(path.encode('utf-8')).hexdigest() if path else None


class BuilderBase(object):
    def __init__(self, cmd, mode, prefixPath=None):
        self.af = None     #memoize the arglist filter
        self.cmd = cmd
        self.mode = mode

        # Used as prefix path for compiler
        if prefixPath:
            self.prefixPath = prefixPath
            # Ensure prefixPath has trailing slash
            if self.prefixPath[-1] != os.path.sep:
                self.prefixPath = self.prefixPath + os.path.sep
            # Check prefix path exists
            if not os.path.exists(self.prefixPath):
                errorMsg = 'Path to compiler "%s" does not exist'
                _logger.error(errorMsg, self.prefixPath)
                raise Exception(errorMsg)

        else:
            self.prefixPath = ''

class ClangBuilder(BuilderBase):
    def getBitcodeCompiler(self):
        cc = self.getCompiler()
        return cc + ['-emit-llvm']

    def getCompiler(self):
        if self.mode == "wllvm++":
            env, prog = 'LLVM_CXX_NAME', 'clang++'
        elif self.mode == "wllvm":
            env, prog = 'LLVM_CC_NAME', 'clang'
        elif self.mode == "wfortran":
            env, prog = 'LLVM_F77_NAME', 'flang'
        else:
            raise Exception("Unknown mode {0}".format(self.mode))
        return ['{0}{1}'.format(self.prefixPath, os.getenv(env) or prog)]

    def getBitcodeArglistFilter(self):
        if self.af is None:
            self.af = ClangBitcodeArgumentListFilter(self.cmd)
        return self.af

class DragoneggBuilder(BuilderBase):
    def getBitcodeCompiler(self):
        pth = os.getenv('LLVM_DRAGONEGG_PLUGIN')
        cc = self.getCompiler()
        # We use '-B' to tell gcc where to look for an assembler.
        # When we build LLVM bitcode we do not want to use the GNU assembler,
        # instead we want gcc to use our own assembler (see as.py).
        cmd = cc + ['-B', asDir, '-fplugin={0}'.format(pth), '-fplugin-arg-dragonegg-emit-ir']
        _logger.debug(cmd)
        return cmd

    def getCompiler(self):
        pfx = ''
        if os.getenv('LLVM_GCC_PREFIX') is not None:
            pfx = os.getenv('LLVM_GCC_PREFIX')

        if self.mode == "wllvm++":
            mode = 'g++'
        elif self.mode == "wllvm":
            mode = 'gcc'
        elif self.mode == "wfortran":
            mode = 'gfortran'
        else:
            raise Exception("Unknown mode {0}".format(self.mode))
        return ['{0}{1}{2}'.format(self.prefixPath, pfx, mode)]

    def getBitcodeArglistFilter(self):
        if self.af is None:
            self.af = ArgumentListFilter(self.cmd)
        return self.af

def getBuilder(cmd, mode):
    compilerEnv = 'LLVM_COMPILER'
    cstring = os.getenv(compilerEnv)
    if cstring is None:
        cstring = 'clang'
    pathPrefix = os.getenv(llvmCompilerPathEnv) # Optional
    _logger.debug('WLLVM compiler using %s', cstring)
    if pathPrefix:
        _logger.debug('WLLVM compiler path prefix "%s"', pathPrefix)

    if cstring == 'clang':
        return ClangBuilder(cmd, mode, pathPrefix)
    elif cstring == 'dragonegg':
        return DragoneggBuilder(cmd, mode, pathPrefix)
    elif cstring is None:
        errorMsg = ' No compiler set. Please set environment variable %s'
        _logger.critical(errorMsg, compilerEnv)
        raise Exception(errorMsg)
    else:
        errorMsg = '%s = %s : Invalid compiler type'
        _logger.critical(errorMsg, compilerEnv, str(cstring))
        raise Exception(errorMsg)

def buildObjectKleer(builder):
    objCompiler = builder.getCompiler()
    objCompiler.extend(builder.cmd)
    #proc = Popen(objCompiler)
    #rc = proc.wait()
    #_logger.debug('buildObject rc = %d', rc)
    #return rc
    return 0

# This command does not have the executable with it

def buildAndAttachBitcode(builder, af):

    #iam: when we have multiple input files we'll have to keep track of their object files.
    newObjectFiles = []
    if not af.inputFiles or af.isEmitLLVM or af.isAssembly or af.isAssembleOnly or (
            af.isDependencyOnly and not af.isCompileOnly) or af.isPreprocessOnly:
        buildObject(builder)
        _logger.debug('No work to do')
        return
    hidden = not af.isCompileOnly

    if  len(af.inputFiles) == 1 and af.isCompileOnly:
        _logger.debug('Compile only case: %s', af.inputFiles[0])
        # iam:
        # we could have
        # "... -c -o foo.o" or even "... -c -o foo.So" which is OK, but we could also have
        # "... -c -o crazy-assed.objectfile" which we wouldn't get right (yet)
        # so we need to be careful with the objFile and bcFile
        # maybe python-magic is in our future ...
        srcFile = af.inputFiles[0]
        (objFile, bcFile) = af.getArtifactNames(srcFile, hidden)
        if af.outputFilename is not None:
            objFile = af.outputFilename
            bcFile = af.getBitcodeFileName()
        buildBitcodeFile(builder, srcFile, bcFile)
        buildObjectFileKleer(builder, bcFile, objFile)
        attachBitcodePathToObject(bcFile, objFile)

    else:
        for srcFile in af.inputFiles:
            _logger.debug('Not compile only case: %s', srcFile)
            (objFile, bcFile) = af.getArtifactNames(srcFile, hidden)
            buildBitcodeFile(builder, srcFile, bcFile)
            buildObjectFileKleer(builder, bcFile, objFile)
            newObjectFiles.append(objFile)
            attachBitcodePathToObject(bcFile, objFile)
    if not af.isCompileOnly:
        linkFiles(builder, newObjectFiles)

    sys.exit(0)

def buildObject(builder):
    objCompiler = builder.getCompiler()
    objCompiler.extend(builder.cmd)
    # print(objCompiler)
    proc = Popen(objCompiler)
    rc = proc.wait()
    _logger.debug('buildObject rc = %d', rc)
    return rc
    #return 0

def buildBitcodeFile(builder, srcFile, bcFile):
    af = builder.getBitcodeArglistFilter()
    bcc = builder.getBitcodeCompiler()
    bcc.extend(af.compileArgs)
    bcc.extend(['-c', srcFile])
    bcc.extend(['-o', bcFile])
    _logger.debug('buildBitcodeFile: %s', bcc)
    #print(bcc)
    proc = Popen(bcc)
    rc = proc.wait()
    if rc != 0:
        _logger.warning('Failed to generate bitcode "%s" for "%s"', bcFile, srcFile)
        sys.exit(rc)

def buildObjectFileKleer(builder,  bcFile, objFile):
    af = builder.getBitcodeArglistFilter()
    bin = os.getenv('LLVM_LLC_NAME')
    if(bin == None):
        bin = 'llc'
    bcc = [bin]#['{0}{1}'.format(bin,'llc')]
    if any("-fPIC" in s for s in af.compileArgs):
        bcc.extend(['-relocation-model=pic'])
    # llc usually does not need args
    # bcc.extend(af.compileArgs)
    bcc.append(bcFile)
    bcc.extend(['-filetype=obj'])
    bcc.extend(['-o', objFile])
    _logger.debug('llcBitcodeFile: %s', bcc)
    proc = Popen(bcc)
    rc = proc.wait()
    if rc != 0:
        _logger.warning('Failed to llc bitcode "%s" for "%s"', bcFile, srcFile)
        sys.exit(rc)

def attachBitcodePathToObject(bcPath, outFileName):
    # Don't try to attach a bitcode path to a binary.  Unfortunately
    # that won't work.
    (_, ext) = os.path.splitext(outFileName)
    _logger.debug('attachBitcodePathToObject: %s  ===> %s [ext = %s]', bcPath, outFileName, ext)

    #iam: just object files, right?
    fileType = FileType.getFileType(outFileName)
    if fileType not in (FileType.MACH_OBJECT, FileType.ELF_OBJECT):
    #if fileType not in (FileType.MACH_OBJECT, FileType.MACH_SHARED, FileType.ELF_OBJECT, FileType.ELF_SHARED):
        _logger.warning('Cannot attach bitcode path to "%s of type %s"', outFileName, FileType.getFileTypeString(fileType))
        return

    #iam: this also looks very dodgey; we need a more reliable way to do this:
    #if ext not in ('.o', '.lo', '.os', '.So', '.po'):
    #    _logger.warning('Cannot attach bitcode path to "%s of type %s"', outFileName, FileType.getReadableFileType(outFileName))
    #    return

    # Now just build a temporary text file with the full path to the
    # bitcode file that we'll write into the object file.
    f = tempfile.NamedTemporaryFile(mode='w+b', delete=False)
    absBcPath = os.path.abspath(bcPath)
    f.write(absBcPath.encode())
    f.write('\n'.encode())
    _logger.debug('Wrote "%s" to file "%s"', absBcPath, f.name)

    # Ensure buffers are flushed so that objcopy doesn't read an empty
    # file
    f.flush()
    os.fsync(f.fileno())
    f.close()


    # Now write our bitcode section
    if sys.platform.startswith('darwin'):
        objcopyCmd = ['ld', '-r', '-keep_private_externs', outFileName, '-sectcreate', darwinSegmentName, darwinSectionName, f.name, '-o', outFileName]
    else:
        objcopyCmd = ['objcopy', '--add-section', '{0}={1}'.format(elfSectionName, f.name), outFileName]
    orc = 0

    # loicg: If the environment variable WLLVM_BC_STORE is set, copy the bitcode
    # file to that location, using a hash of the original bitcode path as a name
    storeEnv = os.getenv('WLLVM_BC_STORE')
    if storeEnv:
        hashName = getHashedPathName(absBcPath)
        copyfile(absBcPath, os.path.join(storeEnv, hashName))

    try:
        if os.path.getsize(outFileName) > 0:
            objProc = Popen(objcopyCmd)
            orc = objProc.wait()
    except OSError:
        # configure loves to immediately delete things, causing issues for
        # us here.  Just ignore it
        os.remove(f.name)
        sys.exit(0)

    os.remove(f.name)

    if orc != 0:
        _logger.error('objcopy failed with %s', orc)
        sys.exit(-1)

def linkFiles(builder, objectFiles):
    af = builder.getBitcodeArglistFilter()
    outputFile = af.getOutputFilename()
    cc = builder.getCompiler()
    cc.extend(objectFiles)
    cc.extend(af.objectFiles)
    cc.extend(af.linkArgs)
    # haijun: by default, we would not use llvm-link
    cc.extend(af.compileArgs)
    cc.extend(['-o', outputFile])
    proc = Popen(cc)
    rc = proc.wait()
    if rc != 0:
        _logger.warning('Failed to link "%s"', str(cc))
        sys.exit(rc)


# def buildObjectFile(builder, srcFile, objFile):
#     af = builder.getBitcodeArglistFilter()
#     cc = builder.getCompiler()
#     cc.extend(af.compileArgs)
#     cc.append(srcFile)
#     cc.extend(['-c', '-o', objFile])
#     _logger.debug('buildObjectFile: %s', cc)
#     proc = Popen(cc)
#     rc = proc.wait()
#     if rc != 0:
#         _logger.warning('Failed to generate object "%s" for "%s"', objFile, srcFile)
#         sys.exit(rc)

# bd & iam:
#
# case 1 (compileOnly):
#
# if the -c flag exists then so do all the .o files, and we need to
# locate them and produce and embed the bit code.
#
# locating them is easy:
#   either the .o is in the cmdline and we are in the simple case,
#   or else it was generated according to getObjectFilename
#
# we then produce and attach bitcode for each inputFile in the cmdline
#
#
# case 2 (compile and link)
#
#  af.inputFiles is not empty, and compileOnly is false.
#  in this case the .o's may not exist, we must regenerate
#  them in any case.
#
#
# case 3 (link only)
#
# in this case af.inputFiles is empty and we are done
#
#
