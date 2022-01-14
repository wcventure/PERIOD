/* GNU C Compiler Detection */
#ifdef __clang_version__
  #if __clang_major__ == 4
    #define LLVM_4
  #endif
  #if __clang_major__ == 11 || __clang_major__ == 12
    #define LLVM_11
  #endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono> 
#include <algorithm>
#include <cassert>
#include <dlfcn.h>
#include <cxxabi.h>
#include "llvm/Pass.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include <llvm/IR/User.h>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/LegacyPassManager.h"
#include <llvm/IR/Module.h>			// for Module
#include <llvm/IR/Function.h>		// for Function
#include <llvm/IR/BasicBlock.h>		// for BasicBlock
#include <llvm/IR/GlobalVariable.h>	// for GlobalVariable
#include <llvm/IR/InstrTypes.h>		// for TerminatorInst
#include <llvm/IR/IntrinsicInst.h>	// for intrinsic instruction
#include <llvm/IR/InstIterator.h>	// for inst iteration
#include <llvm/IR/CFG.h>			// for CFG
#include "llvm/IR/Constant.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/CallGraph.h"
#include <llvm/Analysis/CFG.h>		// for CFG
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/Local.h"	// for FindDbgAddrUses
#include <llvm/Support/CommandLine.h>
#include "llvm/Support/Debug.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/IRReader/IRReader.h>			// for isIRFiler isBitcode
#include <sys/resource.h>					// increase stack size
#include <boost/algorithm/string.hpp>

#ifdef LLVM_11
  #include <llvm/Pass.h>
#else
  #include "llvm/IR/CallSite.h"
  #include <llvm/PassAnalysisSupport.h>
#endif

using namespace llvm;
using namespace llvm;
using namespace std::chrono;
using namespace std;
using std::vector;

//Lock analysis
#define LockFunctionNum 6
char LockFunctionString[][25] = {
  "Lock",
  "mutex_lock",
  "task_lock",
  "PR_RWLock_Rlock",
  "PR_RWLock_Wlock",
  "PR_Lock",
};
char UnlockFunctionString[][25] = {
  "Unlock",
  "mutex_unlock",
  "task_unlock",
  "PR_RWLock_Unlock",
  "PR_Unlock",
  "UK"
};

//Manually Marked Interesting Functions
#define MarkedInterestingNum 1
char MarkedInterestingString[][30] = {
  "js_ClearContextThread",
  "UK"
};

#define DEBUG_TYPE "SensitiveOperations"

//#define WC_DEBUG
//#define WC_DEBUG_2

namespace {
	struct SensitiveOperations :public ModulePass {

		static char ID; // Pass ID, replacement for typeid

		SensitiveOperations():ModulePass(ID){}

		bool runOnModule(Module &M) override;
        
        virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
          AU.addRequiredTransitive<CallGraphWrapperPass>();
          AU.setPreservesAll();
        }

		StringRef getPassName() const override {
			return "SensitiveOperations Instrumentation";
      	}
	};
}

inline const char *get_funcname(const char *src) {
	int status = 99;
	const char *f = abi::__cxa_demangle(src, nullptr, nullptr, &status);
	return f == nullptr ? src : f;
}

int CountLines(char *filename)
{
    ifstream ReadFile;
    int n=0;
    string line;
    ReadFile.open(filename,ios::in);//ios::in 表示以只读的方式读取文件
    if(ReadFile.fail())//文件打开失败:返回0
    {
        return 0;
    }
    else//文件存在
    {
        while(getline(ReadFile,line,'\n'))
        {
            if (line.size() == 0 || line.empty())
              continue;
            else
              n++;
        }
        ReadFile.close();
        return n;
    }
}

std::string getSourceLoc(const Value* val) {
  if(val==NULL)  return "empty val";

  std::string str;
  raw_string_ostream rawstr(str);
  if (const Instruction *inst = dyn_cast<Instruction>(val)) {
    if (isa<AllocaInst>(inst)) {
#ifdef LLVM_4 /* LLVM 4.0 */
	  DbgDeclareInst* DDI = llvm::FindAllocaDbgDeclare(const_cast<Instruction*>(inst));
      if (DDI) {
        DIVariable *DIVar = cast<DIVariable>(DDI->getVariable());
        rawstr << DIVar->getLine();
      }
#else /* LLVM 6.0+ */
      for (DbgInfoIntrinsic *DII : FindDbgAddrUses(const_cast<Instruction*>(inst))) {
        if (DbgDeclareInst *DDI = dyn_cast<DbgDeclareInst>(DII)) {
          DIVariable *DIVar = cast<DIVariable>(DDI->getVariable());
          rawstr << DIVar->getLine();
          break;
        }
      }
#endif
	  
    }
    else if (MDNode *N = inst->getMetadata("dbg")) { // Here I is an LLVM instruction
      DILocation* Loc = cast<DILocation>(N); // DILocation is in DebugInfo.h
      unsigned Line = Loc->getLine();
      StringRef File = Loc->getFilename();
      rawstr << File << ":" << Line;
      //StringRef Dir = Loc->getDirectory();
      //rawstr << Dir << "/" << File << ":" << Line;
    }
  }
  else {
    //Can only get source location for instruction, argument, global var or function.
    rawstr << "N.A.";
  }
  return rawstr.str();
}

uint16_t string_hash_encoding(const char* str){
	uint16_t encodingNum = 26;
	for(int k=0; k<strlen(str); k++){
		encodingNum *= int(str[k]);
		encodingNum = encodingNum + int(str[k]);
	}
	return encodingNum;
}

uint16_t string_hash_encoding(std::string str){
	uint16_t encodingNum = 26;
	for(int k = 0; k < str.length(); k++){
		encodingNum *= int(str[k]);
		encodingNum = encodingNum + int(str[k]);
	}
	return encodingNum;
}

bool SensitiveOperations::runOnModule(Module &M) {

  int linesNum = 0;
  int lockNum = 0;
  int SoNum = 0;
  
  //Function指针到bool的映射表
  std::map<llvm::Function*, bool> pthreadFunctionMap;
  std::set<llvm::Function*> pthreadAllCalleeSet;
  std::set<llvm::Function*> pthreadCallFunction;

  //包装了一层Lock的函数的集合
  std::set<llvm::Function*> LockFunctionSet;
  //包装了一层Unlock的函数的集合
  std::set<llvm::Function*> UnlockFunctionSet;

  //pthread_mutex_Lock的集合
  std::set<string> mutexLockSet;
  //SoFunc的集合
  std::set<string> SoFuncSet;

  //初始化functionMap和controlDependenceMap
  for (Module::iterator function = M.begin(), FEnd = M.end(); function != FEnd; function++) {
    Function* curF = &*function;
    if (pthreadFunctionMap.find(curF) == pthreadFunctionMap.end())
    {
      pthreadFunctionMap[curF] = false;
    }
  }

  //找pthread_create的参数，找pthread_mutex_lock, atomic_fetch_add, atomic_fetch_sub, atomic_exchange
  //refers to: https://www.nuomiphp.com/eplan/85333.html
  for (Module::iterator function = M.begin(), FEnd = M.end(); function != FEnd; function++) {
      if (function->isDeclaration() || function->size() == 0){
          continue;
      }
      //判断是否是MarkedInterestingNum
      for(int i = 0; i < MarkedInterestingNum; i++){
          if (MarkedInterestingString[i] == function->getName()){
              pthreadFunctionMap[&*function]=true;
              break;
          }
      }
      for (Function::iterator basicblock = function->begin(), BBEnd = function->end(); basicblock != BBEnd; basicblock++) { //遍历每一个Basic Block

          for(BasicBlock::iterator instruction = basicblock->begin(), IEnd = basicblock->end(); instruction!=IEnd; instruction++) {    //遍历每一条instruction
        
              if(Instruction *inst = dyn_cast<Instruction>(instruction)) {
            
                  if(inst->getOpcode() == Instruction::Call) {
                      
                      std::string instr_create = "pthread_create";
                      std::string instr_create2 = "PR_CreateThread";
                      if(inst->getNumOperands() >= 5 ){ //操作数大于5
                          
                          int find_thread_flag = 0;
                          int arg_id = 0;

                          if (inst->getNumOperands() == 5 && instr_create == std::string(inst->getOperand(4)->getName())){//找pthread_create
                              find_thread_flag = 1;
                              arg_id = 2;
                          }
                          else if (inst->getNumOperands() == 8 && instr_create2 == std::string(inst->getOperand(7)->getName())){
                              find_thread_flag = 1;
                              arg_id = 1;
                          }

                          if (find_thread_flag==1){
                              Function *pthread_task = dyn_cast<Function>(inst->getOperand(arg_id)->stripPointerCasts());
                              pthreadCallFunction.insert(pthread_task);
                              pthreadAllCalleeSet.insert(pthread_task);
                              pthreadFunctionMap[pthread_task]=true;
                          }
                      } 

                      std::string str_free = "free";
                      if(inst->getNumOperands() >= 2 ){ //操作数大于2
                          if (str_free == std::string(inst->getOperand(1)->getName())){ //free
                              std::string SourceLoc = getSourceLoc(inst);
                              if (SourceLoc.size() == 0) //SourceLoc为空则不用比较了，直接continue
                                continue;
                              if (SourceLoc.find(":")== std::string::npos) //SourceLoc可能取得有问题
                                continue;
                              std::string tmpstring("./");
                              if (SourceLoc.compare(0, tmpstring.length(), tmpstring) == 0) {//如果SourceLoc前两个字符为"./", 则删除掉
                                SourceLoc.erase(0,2);
                              }

                              //freeFuncSet
                              SoFuncSet.insert(SourceLoc);
                              SoNum ++;
                          }
                      }

                      std::string str_mutex_lock = "pthread_mutex_lock";
                      std::string str_mutex_unlock = "pthread_mutex_unlock";
                      if(inst->getNumOperands() >= 2 ){ //操作数大于2
                          if (str_mutex_lock == std::string(inst->getOperand(1)->getName())){ //pthread_mutex_lock
                              std::string SourceLoc = getSourceLoc(inst);
                              if (SourceLoc.size() == 0) //SourceLoc为空则不用比较了，直接continue
                                continue;
                              if (SourceLoc.find(":")== std::string::npos) //SourceLoc可能取得有问题
                                continue;
                              std::string tmpstring("./");
                              if (SourceLoc.compare(0, tmpstring.length(), tmpstring) == 0) {//如果SourceLoc前两个字符为"./", 则删除掉
                                SourceLoc.erase(0,2);
                              }

                              // C++函数名解码
#ifdef LLVM_11
                              string strfname = function->getName().str();
#else
                              string strfname = function->getName();
#endif
                              string strfnameDemangled = get_funcname(strfname.c_str());
                              size_t iIndex = strfnameDemangled.rfind("(");
                              if(iIndex != std::string::npos)
                                strfnameDemangled = strfnameDemangled.substr(0,iIndex);
                              //outs() << strfnameDemangled << "\n";

                              //判断是否是包装的Lock函数
                              int isLockFunction = 0;
                              for(int i = 0; i < LockFunctionNum; i++){
                                if (LockFunctionString[i] == function->getName() || LockFunctionString[i] == strfnameDemangled){
                                  LockFunctionSet.insert(&*function);
                                  isLockFunction = 1;
                                  break;
                                }
                              }

                              if (isLockFunction == 0) {
                                  if (function->size() == 1 && function->arg_size() == 1 && function->getInstructionCount() <= 10 && function->isVarArg() == false && function->hasAddressTaken() == false) { //Heuristic judgment
                                      LockFunctionSet.insert(&*function);
                                      isLockFunction = 1;
                                  }
                              }

                              // 就是pthread_mutex_lock
                              if (isLockFunction == 0) {
                                mutexLockSet.insert(SourceLoc); 
                                lockNum ++;
                                
                              }

                          } else if (str_mutex_unlock == std::string(inst->getOperand(1)->getName())){ //pthread_mutex_unlock
                              std::string SourceLoc = getSourceLoc(inst);
                              if (SourceLoc.size() == 0) //SourceLoc为空则不用比较了，直接continue
                                continue;
                              if (SourceLoc.find(":")== std::string::npos) //SourceLoc可能取得有问题
                                continue;
                              std::string tmpstring("./");
                              if (SourceLoc.compare(0, tmpstring.length(), tmpstring) == 0) {//如果SourceLoc前两个字符为"./", 则删除掉
                                SourceLoc.erase(0,2);
                              }

                              // C++函数名解码
#ifdef LLVM_11
                              string strfname = function->getName().str();
#else
                              string strfname = function->getName();
#endif
                              string strfnameDemangled = get_funcname(strfname.c_str());
                              size_t iIndex = strfnameDemangled.rfind("(");
                              if(iIndex != std::string::npos)
                                strfnameDemangled = strfnameDemangled.substr(0,iIndex);
                              //outs() << strfnameDemangled << "\n";

                              //判断是否是包装的unLock函数
                              int isUnLockFunction = 0;
                              for(int i = 0; i < LockFunctionNum; i++){
                                if (UnlockFunctionString[i] == function->getName() || UnlockFunctionString[i] == strfnameDemangled){
                                  UnlockFunctionSet.insert(&*function);
                                  isUnLockFunction = 1;
                                  break;
                                }
                              }

                              if (isUnLockFunction == 0) {
                                  if (function->size() == 1 && function->arg_size() == 1 && function->getInstructionCount() <= 10 && function->isVarArg() == false && function->hasAddressTaken() == false) { //Heuristic judgment
                                      UnlockFunctionSet.insert(&*function);
                                      isUnLockFunction = 1;
                                  }
                              }
                              // 就是pthread_mutex_unlock则可以忽略

                          } else {

                              // C++函数名解码
#ifdef LLVM_11
                              string strfname = inst->getOperand(1)->getName().str();
#else
                              string strfname = inst->getOperand(1)->getName();
#endif
                              string strfnameDemangled = get_funcname(strfname.c_str());
                              size_t iIndex = strfnameDemangled.rfind("(");
                              if(iIndex != std::string::npos)
                                strfnameDemangled = strfnameDemangled.substr(0,iIndex);
                              
                              //判断是否是interesting的Lock函数
                              for(int i = 0; i < LockFunctionNum; i++){
                                if (LockFunctionString[i] == std::string(inst->getOperand(1)->getName()) || LockFunctionString[i] == strfnameDemangled){
                                  Function *function_of_lock = dyn_cast<Function>(inst->getOperand(1)->stripPointerCasts());
                                  LockFunctionSet.insert(&*function_of_lock);
                                  pthreadFunctionMap[&*function]=true; //Open a back door for interested lock functions, regardless of thread creation and function calls
                                  break;
                                }
                              }
                              //判断是否是interesting的unLock函数
                              for(int i = 0; i < LockFunctionNum; i++){
                                if (UnlockFunctionString[i] == std::string(inst->getOperand(1)->getName()) || UnlockFunctionString[i] == strfnameDemangled){
                                  Function *function_of_lock = dyn_cast<Function>(inst->getOperand(1)->stripPointerCasts());
                                  UnlockFunctionSet.insert(&*function_of_lock);
                                  break;
                                }
                              }
                          }

                      }
                      
                      std::string str_atomic_fetch_add = "long std::atomic_fetch_add<long>";
                      std::string str_atomic_fetch_sub = "long std::atomic_fetch_sub<long>";
                      std::string str_atomic_exchange1 = "std::atomic<void*>::exchange";
                      std::string str_atomic_exchange2 = "webrtc::SSRCDatabase* std::atomic_exchange<webrtc::SSRCDatabase*>";
                      if(inst->getNumOperands() == 3 || inst->getNumOperands() == 4){ //操作数大于3, 有类对象是操作数为4，因为还有this
                          if(CallInst *callfunInst = dyn_cast<CallInst>(instruction)) {
                            string strfname;
                            Function *callfun = callfunInst->getCalledFunction();
                            if (callfun){
#ifdef LLVM_11
                              strfname = callfun->getName().str();
#else
                              strfname = callfun->getName();
#endif
                              
                            } else {
                              if (inst->getNumOperands() == 3) {
#ifdef LLVM_11
                                strfname = inst->getOperand(2)->getName().str();
#else
                                strfname = inst->getOperand(2)->getName();
#endif
                              }
                              else if (inst->getNumOperands() == 4){
#ifdef LLVM_11
                                strfname = inst->getOperand(3)->getName().str();
#else
                                strfname = inst->getOperand(3)->getName();
#endif                          
                              }
                            }
                            // C++函数名解码
                            string strfnameDemangled = get_funcname(strfname.c_str());
                            size_t iIndex = strfnameDemangled.rfind("(");
                            if(iIndex != std::string::npos)
                                strfnameDemangled = strfnameDemangled.substr(0,iIndex);
                            //outs() << "[DEBUG] " << getSourceLoc(inst) << ", " << strfnameDemangled << "\n";

                            if (str_atomic_fetch_add == strfnameDemangled ||
                              str_atomic_fetch_sub == strfnameDemangled ||
                              str_atomic_exchange1 == strfnameDemangled ||
                              str_atomic_exchange2 == strfnameDemangled){

                                std::string SourceLoc = getSourceLoc(inst);
                                if (SourceLoc.size() == 0) //SourceLoc为空则不用比较了，直接continue
                                  continue;
                                if (SourceLoc.find(":")== std::string::npos) //SourceLoc可能取得有问题
                                  continue;
                                std::string tmpstring("./");
                                if (SourceLoc.compare(0, tmpstring.length(), tmpstring) == 0) {//如果SourceLoc前两个字符为"./", 则删除掉
                                  SourceLoc.erase(0,2);
                                }

                                SoFuncSet.insert(SourceLoc);
                                SoNum ++;
                            }
                          }
                      }

                      std::string str_std_Map_size = "std::map<unsigned int, unsigned int, std::less<unsigned int>, std::allocator<std::pair<unsigned int const, unsigned int> > >::size";
                      if(inst->getNumOperands() == 2 ){ //操作数等于2
                          if(CallInst *callfunInst = dyn_cast<CallInst>(instruction)) {
                            string strfname;
                            Function *callfun = callfunInst->getCalledFunction();
                            if (callfun){
#ifdef LLVM_11
                              strfname = callfun->getName().str();
#else
                              strfname = callfun->getName();
#endif                        
                            } else {
#ifdef LLVM_11
                              strfname = inst->getOperand(1)->getName().str();
#else
                              strfname = inst->getOperand(1)->getName();
#endif
                            }
                            // C++函数名解码
                            string strfnameDemangled = get_funcname(strfname.c_str());
                            size_t iIndex = strfnameDemangled.rfind("(");
                            if(iIndex != std::string::npos)
                              strfnameDemangled = strfnameDemangled.substr(0,iIndex);
                            
                            if (str_std_Map_size == strfnameDemangled){
                              std::string SourceLoc = getSourceLoc(inst);
                              if (SourceLoc.size() == 0) //SourceLoc为空则不用比较了，直接continue
                                continue;
                              if (SourceLoc.find(":")== std::string::npos) //SourceLoc可能取得有问题
                                continue;
                              std::string tmpstring("./");
                              if (SourceLoc.compare(0, tmpstring.length(), tmpstring) == 0) {//如果SourceLoc前两个字符为"./", 则删除掉
                                SourceLoc.erase(0,2);
                              }

                              SoFuncSet.insert(SourceLoc);
                              SoNum ++;
                            }
                          }
                      }

                      //memset函数
                      std::string memsetstring = "llvm.memset.";
                      if(inst->getNumOperands() == 5){ //操作数等于5
                          if (boost::algorithm::starts_with(std::string(inst->getOperand(4)->getName()), memsetstring)){
                              std::string SourceLoc = getSourceLoc(inst);
                              if (SourceLoc.size() == 0) //SourceLoc为空则不用比较了，直接continue
                                continue;
                              if (SourceLoc.find(":")== std::string::npos) //SourceLoc可能取得有问题
                                continue;
                              std::string tmpstring("./");
                              if (SourceLoc.compare(0, tmpstring.length(), tmpstring) == 0) {//如果SourceLoc前两个字符为"./", 则删除掉
                                SourceLoc.erase(0,2);
                              }

                              SoFuncSet.insert(SourceLoc);
                              SoNum ++;
                          }
                      }
  
                  } 
                  /*else if (inst->getOpcode() == Instruction::ICmp) {
                      // cmp null statement. i.e., if (xxx==NULL)
                      if(inst->getNumOperands() == 2){ //操作数等于5
                          if (ConstantPointerNull *CI = dyn_cast<ConstantPointerNull>(inst->getOperand(1))){
                              std::string SourceLoc = getSourceLoc(inst);
                              if (SourceLoc.size() == 0) //SourceLoc为空则不用比较了，直接continue
                                continue;
                              if (SourceLoc.find(":")== std::string::npos) //SourceLoc可能取得有问题
                                continue;
                              std::string tmpstring("./");
                              if (SourceLoc.compare(0, tmpstring.length(), tmpstring) == 0) {//如果SourceLoc前两个字符为"./", 则删除掉
                                SourceLoc.erase(0,2);
                              }

                              SoFuncSet.insert(SourceLoc);
                              SoNum ++;
                          }
                      }
                  }*/
                  else if(inst->getOpcode() == Instruction::Invoke){
                      std::string VecResizeStart = "std::vector<";
                      std::string VecResizeEnd = ">::resize";
                      if(inst->getNumOperands() == 5){ //找std::vector<...>::resize
#ifdef LLVM_11
                        string strfname = inst->getOperand(inst->getNumOperands()-1)->getName().str();
#else
                        string strfname = inst->getOperand(inst->getNumOperands()-1)->getName();
#endif                              
                        string strfnameDemangled = get_funcname(strfname.c_str());// C++函数名解码
                        size_t iIndex = strfnameDemangled.rfind("(");
                        if(iIndex != std::string::npos)
                          strfnameDemangled = strfnameDemangled.substr(0,iIndex);
                        if (boost::algorithm::istarts_with(strfnameDemangled, VecResizeStart)
                          && boost::algorithm::ends_with(strfnameDemangled, VecResizeEnd) ) {
                            std::string SourceLoc = getSourceLoc(inst);
                            if (SourceLoc.size() == 0) //SourceLoc为空则不用比较了，直接continue
                              continue;
                            if (SourceLoc.find(":")== std::string::npos) //SourceLoc可能取得有问题
                              continue;
                            std::string tmpstring("./");
                            if (SourceLoc.compare(0, tmpstring.length(), tmpstring) == 0) {//如果SourceLoc前两个字符为"./", 则删除掉
                              SourceLoc.erase(0,2);
                            }

                            SoFuncSet.insert(SourceLoc);
                            SoNum ++;
                        }
                      }
                  }

              }
          }
      }
  }

  //找包装过的Lock函数
  for (Module::iterator function = M.begin(), FEnd = M.end(); function != FEnd; function++) {
      if (function->isDeclaration() || function->size() == 0){
          continue;
      }
      for (Function::iterator basicblock = function->begin(), BBEnd = function->end(); basicblock != BBEnd; basicblock++) { //遍历每一个Basic Block

          for(BasicBlock::iterator instruction = basicblock->begin(), IEnd = basicblock->end(); instruction!=IEnd; instruction++) {    //遍历每一条instruction
        
              if(Instruction *inst = dyn_cast<Instruction>(instruction)) {

                  if(inst->getOpcode() == Instruction::Call){
                      
                      //lock function
                      if(inst->getNumOperands() >= 2) { //操作数大于二
                          std::set<llvm::Function*>::iterator LockFunctionSetiter; //定义前向集合迭代器
                          for(LockFunctionSetiter = LockFunctionSet.begin(); LockFunctionSetiter != LockFunctionSet.end(); LockFunctionSetiter++){
#ifdef LLVM_11
                              std::string LockFunctionstring = (*LockFunctionSetiter)->getName().str();
#else
                              std::string LockFunctionstring = (*LockFunctionSetiter)->getName();
#endif   
                              
                              if ( LockFunctionstring == std::string(inst->getOperand(1)->getName()) ){
                                  std::string SourceLoc = getSourceLoc(inst);
                                  if (SourceLoc.size() == 0) //SourceLoc为空则不用比较了，直接continue
                                    continue;
                                  if (SourceLoc.find(":")== std::string::npos) //SourceLoc可能取得有问题
                                    continue;
                                  std::string tmpstring("./");
                                  if (SourceLoc.compare(0, tmpstring.length(), tmpstring) == 0) {//如果SourceLoc前两个字符为"./", 则删除掉
                                    SourceLoc.erase(0,2);
                                  }

                                  //outs() << SourceLoc << "\n";
                                  //outs() << LockFunctionstring << "\n";
                                  //outs() << *inst << "\n";

                                  mutexLockSet.insert(SourceLoc); 
                                  lockNum ++;
                              }
                          }
                      }

                  }
              }
          }
      }
  }

  CallGraph &CG = Pass::getAnalysis<CallGraphWrapperPass>().getCallGraph(); //获取Call graph
    
  //根据pthreadAllCalleeSet（初始化为pthreadCallFunction）, 标记所有Callee
  
  int isFixPoint = 0;
  std::set<llvm::Function*> SearchCalleeFinishSet; //搜索过Callee函数的集合
  bool canSeekCallee_global_flag = false;
  while(isFixPoint == 0) {//不动点算法
    isFixPoint = 1;
    std::set<llvm::Function*>::iterator functionInterestingiter; //定义前向集合迭代器
    for(functionInterestingiter = pthreadAllCalleeSet.begin(); functionInterestingiter != pthreadAllCalleeSet.end(); functionInterestingiter++) {
      if (SearchCalleeFinishSet.find(*functionInterestingiter) == SearchCalleeFinishSet.end()) { //没有被找过Callee
        SearchCalleeFinishSet.insert(*functionInterestingiter); //加入集合保存，用于不动点迭代不重复搜索，后面遇到这个函数就不用再找了
        CallGraphNode *CallerNode = CG.getOrInsertFunction(*functionInterestingiter);
#ifdef WC_DEBUG_2
        errs () << "[DEBUG] Calculating " << (*functionInterestingiter)->getName() << "'s Callee. " << "  #uses=" << CallerNode->getNumReferences() << '\n';
#endif
        // Seeking Callee (use callgraph, it makes mistakes sometimes)
        bool canSeekCallee = false;
        for (const auto &I : *CallerNode) {
          if (Function *FI = I.second->getFunction()){
            if (FI->size()!=0){
#ifdef LLVM_11
              Value *callvalue = I.first.getValue();
#else
              Value *callvalue = (Value*) I.first;
#endif   
              if (Instruction *callinst = dyn_cast<Instruction>(callvalue)){
#ifdef WC_DEBUG_2
                  errs() << "[DEBUG]   "<< callinst->getParent()->getName() << " calls function '" << FI->getName() << "'\n";
#endif
                  outs() << "[DEBUGDEBUG]   "<< callinst->getParent()->getName() << " calls function '" << FI->getName() << "'\n";
                  pthreadFunctionMap[FI] = true;
                  pthreadAllCalleeSet.insert(FI);
                  isFixPoint = 0;
                  canSeekCallee = true;
                  canSeekCallee_global_flag = true;
              }
            } else {
              //errs() << "  CS<" << I.first << "> calls " << "external node\n";
              continue;
            }
          }
        }

        // The missing Callee is supplemented by traditional method
        for (Function::iterator basicblock = (*functionInterestingiter)->begin(), BBEnd = (*functionInterestingiter)->end(); basicblock != BBEnd; basicblock++) { //遍历每一个Basic Block
          for(BasicBlock::iterator instruction = basicblock->begin(), IEnd = basicblock->end(); instruction!=IEnd; instruction++) {    //遍历每一条instruction
            if(Instruction *inst = dyn_cast<Instruction>(instruction)) {
              if(inst->getOpcode() == Instruction::Call){
                if(CallInst *callfunInst = dyn_cast<CallInst>(instruction)) {
                  Function *callfun = callfunInst->getCalledFunction();
                  if (callfun){
                    ; // the same as else branch, but is done above (see Seeking Callee)
                  } else {
                    callfun = dyn_cast<Function>(inst->getOperand(inst->getNumOperands()-1)->stripPointerCasts());
                    pthreadFunctionMap[callfun] = true;
                  }
                }
              }
            }
          }
        }

#ifdef WC_DEBUG_2
        if (canSeekCallee == false) {
          errs() << "[DEBUG]   None" << "\n";
        }
#endif
      }
    }
  }

  /*wcventure 打开ConConfig文件*/

  char *Con_PATH = NULL;
  char *ConFileName;
  int ConcurrencyInsteresting = 0;
  Con_PATH = getenv("Con_PATH");

  if (Con_PATH == NULL || strcmp(Con_PATH, "") == 0){
    ConcurrencyInsteresting = 0;
    outs() << "[DEBUG] Will use defult instru\n";
  }

  if (Con_PATH != NULL && Con_PATH[0] != '.' && Con_PATH[0] != ' ') {
    ConFileName = (char*)malloc(strlen(Con_PATH) + 1);
      if (ConFileName == NULL) {
        exit(1);
        return false;
      }
      strcpy(ConFileName, Con_PATH);
      linesNum = CountLines(ConFileName);
  } else {
    outs () << "[DEBUG] Please export Con_PATH" << "\n";
  }

  ifstream read_file;
  read_file.open(ConFileName, ios::binary);

  if (read_file.fail()) {
    outs() << "[DEBUG] Failed to open ConConfig file." << "\n";
    outs() << "[DEBUG] The program quit without execution." << "\n";
    ConcurrencyInsteresting = 0;
    exit(1);
    return false;
  } else {
    ConcurrencyInsteresting = 1;
    outs() << "[DEBUG] The ConConfig file was successfully opened.\n[DEBUG] The file is " << ConFileName << "\n[DEBUG]\n";
  }

  std::set<string>::iterator Coniter; //定义前向集合迭代器
  int lock_index;
  int free_index;

  if (ConcurrencyInsteresting == 1) {
    string* ConFirst = new string[linesNum];
    string line;
    for (int i=0; getline(read_file, line); i++){
      if (!line.empty() && line[line.size() - 1] == '\r'){
        if (line.size() == 1)
          ConFirst[i] = "N.A.";
        else {
          ConFirst[i] = line.erase(line.size() - 1);
		    }
      }
      else if (line.size() == 0 || line.empty())
      {
        continue;
      }
      else {
        ConFirst[i] = line;
      }

      //过滤掉重复的
      lock_index = 0;
      free_index = 0; 
      for(Coniter = mutexLockSet.begin(); Coniter != mutexLockSet.end() && lock_index < lockNum; lock_index++){
        if (ConFirst[i] ==  *Coniter){
          Coniter = mutexLockSet.erase(Coniter);
        }else{
	  Coniter++;
	}
      }
      for(Coniter = SoFuncSet.begin(); Coniter != SoFuncSet.end() && free_index < SoNum; free_index++){
        if (ConFirst[i] ==  *Coniter){
          Coniter = SoFuncSet.erase(Coniter);
        } else {
	  Coniter++;
	}
      }
    }

    //将key points和lock组合一下
    lockNum = mutexLockSet.size();
    SoNum = SoFuncSet.size();
    string* ConLine = new string[linesNum + lockNum + SoNum];
    for (int i=0; i < linesNum; i++)
    {
      ConLine[i] = ConFirst[i];
    }
    
    lock_index = 0;
    free_index = 0;
    for(Coniter = mutexLockSet.begin(); Coniter != mutexLockSet.end() && lock_index < lockNum; Coniter++, lock_index++){
      ConLine[linesNum + lock_index] =  *Coniter;
    }
    for(Coniter = SoFuncSet.begin(); Coniter != SoFuncSet.end() && free_index < SoNum; Coniter++, free_index++){
      ConLine[linesNum + lock_index + free_index] =  *Coniter;
    }
    
//#ifdef WC_DEBUG
    outs() << "[DEBUG] All the Concurent insteresting line are shown as follow:" << "\n";
    for (int i = 0; i < linesNum + lockNum + SoNum; i++){
      outs() << "[DEBUG]  " << i + 1 << ")\t" << ConLine[i].c_str() << "\n";
    }
//#endif
    free(ConFileName);
    read_file.close();

    /*wcventure 关闭ConConfig文件*/

    for (Module::iterator function = M.begin(), FEnd = M.end(); function != FEnd; function++) { //遍历每一个Function

      if (canSeekCallee_global_flag == false) {
        if (function->isDeclaration() || function->size() == 0){
          //outs() << "[DEBUG] " << function->getName() << " is filtered" << "\n";
          continue;
        }
      } else {
        if (function->isDeclaration() || function->size() == 0 || pthreadFunctionMap[&*function]==false){
          //outs() << "[DEBUG] " << function->getName() << " is filtered" << "\n";
          continue;
        }
      }

      for (Function::iterator basicblock = function->begin(), BBEnd = function->end(); basicblock != BBEnd; basicblock++) { //遍历每一个Basic Block

        for(BasicBlock::iterator instruction = basicblock->begin(), IEnd = basicblock->end(); instruction!=IEnd; instruction++) {	//遍历每一条instruction
          
          if(Instruction *inst = dyn_cast<Instruction>(instruction)) {
            
            //指令对应的代码行号
            std::string SourceLoc = getSourceLoc(inst);

            if (SourceLoc.size() == 0) //SourceLoc为空则不用比较了，直接continue
              continue;
			  
            if (SourceLoc.find(":")== std::string::npos) //SourceLoc可能取得有问题
              continue;
			  
            std::string tmpstring("./");
            if (SourceLoc.compare(0, tmpstring.length(), tmpstring) == 0) {//如果SourceLoc前两个字符为"./", 则删除掉
              SourceLoc.erase(0,2);
            }

            for (int i = 0; i < linesNum + lockNum + SoNum; i++){
              
			        int spaceIndex = ConLine[i].find(" ");
			        std::string splitConLine;
              if (spaceIndex>=0)
                splitConLine = ConLine[i].substr(0,spaceIndex);
              else
                splitConLine = ConLine[i];
#ifdef WC_DEBUG_2
              outs() << splitConLine << "--" << SourceLoc << "\n";
#endif
              if (boost::algorithm::ends_with(splitConLine, SourceLoc)) { //rather than ConLine[i] == SourceLoc
#ifdef WC_DEBUG_2
                outs() << "[DEBUG] Match to diff in function "<< function->getName() << ":" << SourceLoc << "\n";
                outs() << "[DEBUG]   " << *inst << " (in BasicBlock " << basicblock->getName() << ")" << "\n";
#endif
                /* start: Find Sensitive Operations */
				
                //outs() << SourceLoc << ": " << *inst << "\n";
                int FindFlag = 0;
				
                //在call指令中搜索
                if(inst->getOpcode() == Instruction::Call) {

                  //outs() << *inst << "\n";
                  
                  //malloc函数
                  std::string instr_malloc1 = "malloc";
                  std::string instr_malloc2 = "xmalloc";
                  std::string instr_malloc3 = "valloc";
                  std::string instr_malloc4 = "safe_malloc";
                  std::string instr_malloc5 = "safemalloc";
                  std::string instr_malloc6 = "safexmalloc";
                  if(inst->getNumOperands() >= 2 && !FindFlag){ //操作数大于二
                    if ( instr_malloc1 == std::string(inst->getOperand(1)->getName()) || 
                      instr_malloc2 == std::string(inst->getOperand(1)->getName()) ||
                      instr_malloc3 == std::string(inst->getOperand(1)->getName()) || 
                      instr_malloc4 == std::string(inst->getOperand(1)->getName()) || 
                      instr_malloc5 == std::string(inst->getOperand(1)->getName()) || 
                      instr_malloc6 == std::string(inst->getOperand(1)->getName()) ) {
                        outs() << ConLine[i] << " malloc " << function->getName() << " " << string_hash_encoding(splitConLine) << "\n";
                        FindFlag = 1;
                    }
                  }

                  //calloc函数
                  std::string instr_calloc1 = "calloc";
                  std::string instr_calloc2 = "xcalloc";
                  std::string instr_calloc3 = "memalign";
                  std::string instr_calloc4 = "aligned_alloc";
                  std::string instr_calloc5 = "safe_calloc";
                  std::string instr_calloc6 = "safecalloc";
                  std::string instr_calloc7 = "safexcalloc";
                  if(inst->getNumOperands() >= 3 && !FindFlag){ //操作数大于二
                    if ( instr_calloc1 == std::string(inst->getOperand(2)->getName()) || 
                    instr_calloc2 == std::string(inst->getOperand(2)->getName()) || 
                    instr_calloc3 == std::string(inst->getOperand(2)->getName()) ||
                    instr_calloc4 == std::string(inst->getOperand(2)->getName()) ||
                    instr_calloc5 == std::string(inst->getOperand(2)->getName()) ||
                    instr_calloc6 == std::string(inst->getOperand(2)->getName()) ||
                    instr_calloc7 == std::string(inst->getOperand(2)->getName()) ){
                      outs() << ConLine[i] << " malloc " << function->getName() << " " << string_hash_encoding(splitConLine) << "\n";
                      FindFlag = 1;
                    }
                  }

                  //realloc函数
                  std::string instr_realloc1 = "realloc"; 
                  if(inst->getNumOperands() >= 3 && !FindFlag){ //操作数大于二
                    if ( instr_realloc1 == std::string(inst->getOperand(2)->getName()) ){
                      outs() << ConLine[i] << " realloc " << function->getName() << " " << string_hash_encoding(splitConLine) << "\n";
                      FindFlag = 1;
                    }
                  }

                  //free函数
                  std::string instr_free1 = "free";
                  std::string instr_free2 = "xfree";
                  std::string instr_free3 = "cfree";
                  std::string instr_free4 = "safe_cfree";
                  std::string instr_free5 = "safe_free";
                  std::string instr_free6 = "safefree";
                  std::string instr_free7 = "safexfree";
                  if(inst->getNumOperands() >= 2 && !FindFlag){ //操作数大于二
                    if ( instr_free1 == std::string(inst->getOperand(1)->getName()) || 
                      instr_free2 == std::string(inst->getOperand(1)->getName()) ||
                    instr_free3 == std::string(inst->getOperand(1)->getName()) ||
                    instr_free4 == std::string(inst->getOperand(1)->getName()) ||
                    instr_free5 == std::string(inst->getOperand(1)->getName()) ||
                    instr_free6 == std::string(inst->getOperand(1)->getName()) ||
                    instr_free7 == std::string(inst->getOperand(1)->getName()) ) {
                      outs() << ConLine[i] << " free " << function->getName() << " " << string_hash_encoding(splitConLine) << "\n";
                      FindFlag = 1;
                    }
                  }

                  //new函数
                  std::string instr_new1 = "_Znwm";
                  std::string instr_new2 = "_Znam";
                  std::string instr_new3 = "_Znaj";
                  std::string instr_new4 = "_Znwj";
                  if(inst->getNumOperands() >= 2 && !FindFlag){ //操作数大于二
                    if ( instr_new1 == std::string(inst->getOperand(1)->getName()) || instr_new2 == std::string(inst->getOperand(1)->getName()) || instr_new3 == std::string(inst->getOperand(1)->getName()) || instr_new4 == std::string(inst->getOperand(1)->getName()) ){
                      outs() << ConLine[i] << " new " << function->getName() << " " << string_hash_encoding(splitConLine) << "\n";
                      FindFlag = 1;
                    }
                  }

                  //delete函数
                  std::string instr_delete1 = "_ZdaPv"; 
                  std::string instr_delete2 = "_ZdlPv"; 
                  if(inst->getNumOperands() >= 2 && !FindFlag){ //操作数大于二
                    if ( instr_delete1 == std::string(inst->getOperand(1)->getName()) || instr_delete2 == std::string(inst->getOperand(1)->getName()) ){
                      outs() << ConLine[i] << " delete " << function->getName() << " " << string_hash_encoding(splitConLine) << "\n";
                      FindFlag = 1;
                    }
                  }
				  
				          //memset函数
				          std::string memsetstring = "llvm.memset.";
				          if(inst->getNumOperands() == 5 && !FindFlag){ //操作数等于5
                    if (boost::algorithm::starts_with(std::string(inst->getOperand(4)->getName()), memsetstring)){
                      outs() << ConLine[i] << " memset " << function->getName() << " " << string_hash_encoding(splitConLine) << "\n";
                      FindFlag = 1;
                    }
                  }
				  
				          //memcpy函数
				          std::string memcpystring = "llvm.memcpy.";
				          if(inst->getNumOperands() == 5 && !FindFlag){ //操作数等于5
                    if (boost::algorithm::starts_with(std::string(inst->getOperand(4)->getName()), memcpystring)){
                      outs() << ConLine[i] << " memcpy " << function->getName() << " " << string_hash_encoding(splitConLine) << "\n";
					            FindFlag = 1;
                    }
                  }
                  
                  //pthread_mutex_lock
				          std::string mutexLockstring = "pthread_mutex_lock";
                  if(inst->getNumOperands() >= 2 && !FindFlag){ //操作数大于二
                    if ( mutexLockstring == std::string(inst->getOperand(1)->getName()) ){
                      //outs() << *inst << "\n";
                      //outs() << *inst->getOperand(0) << "\n";      
                      outs() << ConLine[i] << " lock " << function->getName() << " " << string_hash_encoding(splitConLine) << "\n";
                      FindFlag = 1;
                    }
                  }

                  //lock function
                  if(inst->getNumOperands() >= 2 && !FindFlag){ //操作数大于二
                    std::set<llvm::Function*>::iterator LockFunctionSetiter; //定义前向集合迭代器
                    for(LockFunctionSetiter = LockFunctionSet.begin(); LockFunctionSetiter != LockFunctionSet.end(); LockFunctionSetiter++){
#ifdef LLVM_11
                      std::string LockFunctionstring = (*LockFunctionSetiter)->getName().str();
#else
                      std::string LockFunctionstring = (*LockFunctionSetiter)->getName();
#endif
                      //outs() << LockFunctionstring << "\n";
                      //outs() << *inst << "\n";
                      if ( LockFunctionstring == std::string(inst->getOperand(1)->getName()) ){
                        outs() << ConLine[i] << " lock " << function->getName() << " " << string_hash_encoding(splitConLine) << "\n";
                        FindFlag = 1;
                      }
                    }
                  }

                  //pthread_mutex_unlock
				          std::string mutexUnlockstring = "pthread_mutex_unlock";
                  if(inst->getNumOperands() >= 2 && !FindFlag){ //操作数大于二
                    if ( mutexUnlockstring == std::string(inst->getOperand(1)->getName()) ){
                      //outs() << *inst << "\n";
                      //outs() << *inst->getOperand(0) << "\n";
                      outs() << ConLine[i] << " unlock " << function->getName() << " " << string_hash_encoding(splitConLine) << "\n";
                      FindFlag = 1;
                    }
                  }

                  //unlock function
                  if(inst->getNumOperands() >= 2 && !FindFlag){ //操作数大于二
                    std::set<llvm::Function*>::iterator UnlockFunctionSetiter; //定义前向集合迭代器
                    for(UnlockFunctionSetiter = UnlockFunctionSet.begin(); UnlockFunctionSetiter != UnlockFunctionSet.end(); UnlockFunctionSetiter++){
#ifdef LLVM_11
                      std::string UnlockFunctionstring = (*UnlockFunctionSetiter)->getName().str();
#else
                      std::string UnlockFunctionstring = (*UnlockFunctionSetiter)->getName();
#endif
                      if ( UnlockFunctionstring == std::string(inst->getOperand(1)->getName()) ){
                        outs() << ConLine[i] << " unlock " << function->getName() << " " << string_hash_encoding(splitConLine) << "\n";
                        FindFlag = 1;
                      }
                    }
                  }

                  
                  std::string str_atomic_fetch_add = "long std::atomic_fetch_add<long>";
                  std::string str_atomic_fetch_sub = "long std::atomic_fetch_sub<long>";
                  std::string str_atomic_exchange1 = "std::atomic<void*>::exchange";
                  std::string str_atomic_exchange2 = "webrtc::SSRCDatabase* std::atomic_exchange<webrtc::SSRCDatabase*>";
				          if((inst->getNumOperands() == 3 || inst->getNumOperands() == 4) && !FindFlag){ //操作数等于5
                    if(CallInst *callfunInst = dyn_cast<CallInst>(instruction)) {
                      string strfname;
                      Function *callfun = callfunInst->getCalledFunction();
                      if (callfun){
#ifdef LLVM_11
                        strfname = callfun->getName().str();
#else
                        strfname = callfun->getName();
#endif
                      } else {
                        if (inst->getNumOperands() == 3) {
#ifdef LLVM_11
                          strfname = inst->getOperand(2)->getName().str();
#else
                          strfname = inst->getOperand(2)->getName();
#endif
                        }
                        else if (inst->getNumOperands() == 4) {
#ifdef LLVM_11
                          strfname = inst->getOperand(3)->getName().str();
#else
                          strfname = inst->getOperand(3)->getName();
#endif
                        }
                      }
                      // C++函数名解码
                      string strfnameDemangled = get_funcname(strfname.c_str());
                      size_t iIndex = strfnameDemangled.rfind("(");
                      if(iIndex != std::string::npos)
                          strfnameDemangled = strfnameDemangled.substr(0,iIndex);
                      //outs() << "[DEBUG] " << strfnameDemangled << "\n";
                
                      if (boost::algorithm::starts_with(strfnameDemangled, str_atomic_fetch_add) ||
                          boost::algorithm::starts_with(strfnameDemangled, str_atomic_fetch_sub) ||
                          boost::algorithm::starts_with(strfnameDemangled, str_atomic_exchange1) ||
                          boost::algorithm::starts_with(strfnameDemangled, str_atomic_exchange2)
                          ){
                        outs() << ConLine[i] << " std_atomic " << function->getName() << " " << string_hash_encoding(splitConLine) << "\n";
                        FindFlag = 1;
                      }
                    }
                  }

                  //std::Map.size函数
                  std::string str_std_Map_size = "std::map<unsigned int, unsigned int, std::less<unsigned int>, std::allocator<std::pair<unsigned int const, unsigned int> > >::size";
                  if(inst->getNumOperands() == 2 && !FindFlag){ //操作数等于2
                    if(CallInst *callfunInst = dyn_cast<CallInst>(instruction)) {
                      string strfname;
                      Function *callfun = callfunInst->getCalledFunction();
                      if (callfun){
#ifdef LLVM_11
                        strfname = callfun->getName().str();
#else
                        strfname = callfun->getName();
#endif
                      } else {
#ifdef LLVM_11
                        strfname = inst->getOperand(1)->getName().str();
#else
                        strfname = inst->getOperand(1)->getName();
#endif
                      }
                      // C++函数名解码
                      string strfnameDemangled = get_funcname(strfname.c_str());
                      size_t iIndex = strfnameDemangled.rfind("(");
                      if(iIndex != std::string::npos)
                        strfnameDemangled = strfnameDemangled.substr(0,iIndex);
                      if (boost::algorithm::starts_with(strfnameDemangled, str_std_Map_size)
                          ){
                        outs() << ConLine[i] << " std_size " << function->getName() << " " << string_hash_encoding(splitConLine) << "\n";
                        FindFlag = 1;
                      }
                    }
                  }
                }

                //针对有些new在invoke指令中搜索
                if(inst->getOpcode() == Instruction::Invoke) {
                  std::string instr_malloc1 = "malloc";
                  std::string instr_new1 = "_Znwm";
                  std::string instr_new2 = "_Znam";
                  std::string instr_new3 = "_Znaj";
                  std::string instr_new4 = "_Znwj";
                  if(inst->getNumOperands() >= 2 && !FindFlag){ //操作数大于二
                    if (instr_malloc1 == std::string(inst->getOperand(1)->getName()) || instr_new1 == std::string(inst->getOperand(1)->getName()) || instr_new2 == std::string(inst->getOperand(1)->getName()) || instr_new3 == std::string(inst->getOperand(1)->getName()) || instr_new4 == std::string(inst->getOperand(1)->getName()) ){
                      outs() << ConLine[i] << " new " << function->getName() << " " << string_hash_encoding(splitConLine) << "\n";
                      FindFlag = 1;
                    }
                  }

                  std::string VecResizeStart = "std::vector<";
                  std::string VecResizeEnd = ">::resize";
                  if(inst->getNumOperands() == 5  && !FindFlag){ //找std::vector<...>::resize
#ifdef LLVM_11
                    string strfname = inst->getOperand(inst->getNumOperands()-1)->getName().str();
#else
                    string strfname = inst->getOperand(inst->getNumOperands()-1)->getName();
#endif
                    string strfnameDemangled = get_funcname(strfname.c_str());// C++函数名解码
                    size_t iIndex = strfnameDemangled.rfind("(");
                    if(iIndex != std::string::npos)
                      strfnameDemangled = strfnameDemangled.substr(0,iIndex);
                    if (boost::algorithm::istarts_with(strfnameDemangled, VecResizeStart)
                      && boost::algorithm::ends_with(strfnameDemangled, VecResizeEnd) ) {
                      outs() << ConLine[i] << " std::vector<...>::resize " << function->getName() << " " << string_hash_encoding(splitConLine) << "\n";
                      FindFlag = 1;
                    }
                  }
                }

                //在store指令中搜索X=NULL;
                if(inst->getOpcode() == Instruction::Store) {
                  if(inst->getNumOperands() >= 2 && !FindFlag){ //操作数大于二
					          if (ConstantPointerNull *CI = dyn_cast<ConstantPointerNull>(inst->getOperand(0))){
						          outs() << ConLine[i] << " nullptr " << function->getName() << " " << string_hash_encoding(splitConLine) << "\n";
						          FindFlag = 1;
					          }
                  }
                }

                // 再icmp指令中搜索 cmp null statement. i.e., if (xxx==NULL)
                // Too much redundancy
                /*if (inst->getOpcode() == Instruction::ICmp) {
                  if(inst->getNumOperands() == 2  && !FindFlag){ //操作数等于5
                    if (ConstantPointerNull *CI = dyn_cast<ConstantPointerNull>(inst->getOperand(1))){
                      outs() << ConLine[i] << " cmpnull " << function->getName() << " " << string_hash_encoding(splitConLine) << "\n";
						          FindFlag = 1;
                    }
                  }
                }*/
				
				        if(!FindFlag){
					        outs() << ConLine[i] << " R/W " << function->getName() << " " << string_hash_encoding(splitConLine) << "\n";
				        }
                /* end: Find Sensitive Operations */

                break;
              }
            }
          }
        }

      }

    }

  }
  return true;
}

char SensitiveOperations::ID = 0;

static RegisterPass<SensitiveOperations> X("so", "SensitiveOperations Pass");
