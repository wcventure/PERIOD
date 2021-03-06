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
#include "llvm/Pass.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include <llvm/IR/Type.h>
#include <llvm/IR/User.h>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/LegacyPassManager.h"
#include <llvm/IR/Module.h>					// for Module
#include <llvm/IR/Function.h>				// for Function
#include <llvm/IR/BasicBlock.h>				// for BasicBlock
#include <llvm/IR/Instructions.h>			// for Instructions
#include <llvm/IR/GlobalVariable.h>			// for GlobalVariable
#include <llvm/IR/InstrTypes.h>				// for TerminatorInst
#include <llvm/IR/IntrinsicInst.h>			// for intrinsic instruction
#include <llvm/IR/InstIterator.h>			// for inst iteration
#include <llvm/IR/CFG.h>					// for CFG
#include "llvm/IR/Constant.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/CallGraph.h"
#include <llvm/Analysis/CFG.h>				// for CFG
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

/* GNU C Compiler Detection */
#ifdef __clang_version__
  #if __clang_major__ == 4
    #define LLVM_4
  #endif
#endif

#define DEBUG_TYPE "InstrumentPass"

namespace {

	struct Instru :public ModulePass {
		
		static char ID;
		Instru():ModulePass(ID){ }

		bool runOnModule(Module &M) override;
		
		StringRef getPassName() const override {
			return "Instru Instrumentation";
      	}
	};
}

int CountLines(char *filename)
{
    ifstream ReadFile;
    int n=0;
    string line;
    ReadFile.open(filename,ios::in);//ios::in ????????????????????????????????????
    if(ReadFile.fail())//??????????????????:??????0
    {
        return 0;
    }
    else//????????????
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

bool Instru::runOnModule(Module &M) {

	LLVMContext &C = M.getContext();
	IntegerType *Int32Ty = IntegerType::getInt32Ty(C);
	
	// ???????????????????????????
	llvm::LLVMContext& context = M.getContext ();
	llvm::IRBuilder<> builder(context); 
	
	//Function instr_LOC
	std::vector<Type*> argTypesLOC;
	argTypesLOC.push_back(builder.getInt8PtrTy());
	argTypesLOC.push_back(builder.getInt32Ty());
	ArrayRef<Type*> argTypesRefLOC(argTypesLOC);
	llvm::FunctionType *funcLOCType = FunctionType::get(builder.getVoidTy(),argTypesRefLOC,false);
	llvm::Function *instr_LOCFunc = Function::Create(funcLOCType, llvm::Function::ExternalLinkage, "_Z9instr_LOCPvj", &M);

	// Function instr_SLOC
	std::vector<Type*> argTypesSLOC;
	argTypesSLOC.push_back(builder.getInt8PtrTy());
	argTypesSLOC.push_back(builder.getInt8PtrTy());
	ArrayRef<Type*> argTypesRefSLOC(argTypesSLOC);
	llvm::FunctionType *funcSLOCType = FunctionType::get(builder.getVoidTy(),argTypesRefSLOC,false);
	llvm::Function *instr_SLOCFunc = Function::Create(funcSLOCType, llvm::Function::ExternalLinkage, "_Z16instr_LOC_stringPvS_", &M);


	// Function instr_Call()
	std::vector<Type*> argTypesCall;
	argTypesCall.push_back(builder.getInt8PtrTy());
	ArrayRef<Type*> argTypesRefCall(argTypesCall);
	llvm::FunctionType *funcCallType = llvm::FunctionType::get(builder.getVoidTy(), argTypesRefCall, false);
	llvm::Function *instr_CallFunc = llvm::Function::Create(funcCallType, llvm::Function::ExternalLinkage, "_Z10instr_CallPv", &M);

	// Function instr_PthreadCall()
	std::vector<Type*> argTypesPthreadCall;
	argTypesPthreadCall.push_back(builder.getInt8PtrTy());
	ArrayRef<Type*> argTypesRefPthreadCall(argTypesPthreadCall);
	llvm::FunctionType *funcPthreadCallType = llvm::FunctionType::get(builder.getVoidTy(), argTypesRefPthreadCall, false);
	llvm::Function *instr_PthreadCallFunc = llvm::Function::Create(funcPthreadCallType, llvm::Function::ExternalLinkage, "_Z17instr_PthreadCallPv", &M);

	// Function instr_Return()
	std::vector<Type*> argTypesRet;
	argTypesRet.push_back(builder.getInt8PtrTy());
	ArrayRef<Type*> argTypesRefRet(argTypesRet);
	llvm::FunctionType *funcReturnType = llvm::FunctionType::get(builder.getVoidTy(), argTypesRefRet, false);
	llvm::Function *instr_ReturnFunc = llvm::Function::Create(funcReturnType, llvm::Function::ExternalLinkage, "_Z12instr_ReturnPv", &M);

	// Function instr_create()
	std::vector<Type*> argTypesCreate;
	argTypesCreate.push_back(Type::getInt64PtrTy(C));
	argTypesCreate.push_back(builder.getInt8PtrTy());
	ArrayRef<Type*> argTypesRefCreate(argTypesCreate);
	llvm::FunctionType *funcCreateType = llvm::FunctionType::get(builder.getVoidTy(), argTypesRefCreate, false);
	llvm::Function *instr_CreateFunc = llvm::Function::Create(funcCreateType, llvm::Function::ExternalLinkage, "_Z20instr_pthread_createPmPv", &M);
	
	// Function instr_Join
	std::vector<Type*> argTypesJoin;
	argTypesJoin.push_back(builder.getInt64Ty());
	ArrayRef<Type*> argTypesRefJoin(argTypesJoin);
	llvm::FunctionType *funcJoinType = FunctionType::get(builder.getVoidTy(),argTypesRefJoin,false);
	llvm::Function *instr_JoinFunc = Function::Create(funcJoinType, llvm::Function::ExternalLinkage, "_Z18instr_pthread_joinm", &M);

	// Function Demo
	// llvm::FunctionType *funcNextType = llvm::FunctionType::get(builder.getVoidTy(), false);
	// llvm::Function *instr_NextFunc = llvm::Function::Create(funcNextType, llvm::Function::ExternalLinkage, "_Z18instr_Nextm", &M);
	
	/*wcventure ??????ConConfig??????*/

	char *Con_PATH = NULL;
	char *ConFileName;
	int linesNum = 0;
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

	if (ConcurrencyInsteresting == 1) {
		string* ConLine = new string[linesNum];
		string line;
		for (int i=0; getline(read_file, line); i++){
			if (!line.empty() && line[line.size() - 1] == '\r'){
				if (line.size() == 1)
					ConLine[i] = "N.A.";
				else
					ConLine[i] = line.erase(line.size() - 1);
				}
			else if (line.size() == 0 || line.empty())
			{
				continue;
			}
			else {
				ConLine[i] = line;
			}
		}
#ifdef WC_DEBUG
		outs() << "[DEBUG] All the Concurent insteresting line are shown as follow:" << "\n";
		for (int i = 0; i < linesNum; i++){
		outs() << "[DEBUG]  " << i + 1 << ")\t" << ConLine[i].c_str() << "\n";
		}
#endif
		free(ConFileName);
		read_file.close();
		
		std::set<llvm::Function*> pthreadCallFunction; //???pthread_create?????????
		std::set<llvm::Function*>::iterator setit; //???????????????????????????  
		for (Module::iterator function = M.begin(), FEnd = M.end(); function != FEnd; function++) { 
			if (function->isDeclaration() || function->size() == 0){
				continue;
			}
			for (Function::iterator basicblock = function->begin(), BBEnd = function->end(); basicblock != BBEnd; basicblock++) { //???????????????Basic Block

				for(BasicBlock::iterator instruction = basicblock->begin(), IEnd = basicblock->end(); instruction!=IEnd; instruction++) {	//???????????????instruction
			  
					if(Instruction *inst = dyn_cast<Instruction>(instruction)) {
						if(inst->getOpcode() == Instruction::Call){
							std::string instr_create = "pthread_create";
							if(inst->getNumOperands() >= 5 ){ //???????????????5
								if (instr_create == std::string(inst->getOperand(4)->getName())){ //???pthread_create
									Function *pthread_task = dyn_cast<Function>(inst->getOperand(2)->stripPointerCasts());
									pthreadCallFunction.insert(pthread_task);
								}
							}
						}
					}
				}
			}
		}
			
		// ??????????????????
		for (Module::iterator function = M.begin(), FEnd = M.end(); function != FEnd; function++) { //???????????????Function

			if (function->isDeclaration() || function->size() == 0){
				continue;
			}
			
			BasicBlock::iterator FIP = function->begin()->getFirstInsertionPt(); // ???????????????
			SmallVector<Value *, 1> retArg;//?????????Vector????????????
			Value *CurFuncVa = ConstantExpr::getBitCast(&(*function), Type::getInt8PtrTy(C));
			retArg.push_back(CurFuncVa);
			IRBuilder<> IRB(&(*FIP));
			IRB.CreateCall(instr_CallFunc, retArg);
			
			if (function->getName() == "main") {//main??????????????????????????????,?????????????????????Pass??????mian??????
				SmallVector<Value *, 2> mlocArg;
				Value *conInte = ConstantInt::get(Int32Ty, 0);
				Value *CurFuncVal = ConstantExpr::getBitCast(&(*function), Type::getInt8PtrTy(C));
				mlocArg.push_back(CurFuncVal);
				mlocArg.push_back(conInte);
				IRB.CreateCall(instr_LOCFunc, mlocArg);
				//continue;
			} else {//???pthread_create?????????????????????
				Function* curF = &*function;
				for(setit = pthreadCallFunction.begin(); setit != pthreadCallFunction.end(); setit++) {  
					if (*setit == curF){
						SmallVector<Value *, 1> PCFArg;
						Value *CurFuncVal = ConstantExpr::getBitCast(&(*function), Type::getInt8PtrTy(C));
						PCFArg.push_back(CurFuncVal);
						IRB.CreateCall(instr_PthreadCallFunc, PCFArg);
						//outs() << curF->getName() << "\n";
						continue;
					}
				}
			}

			for (Function::iterator basicblock = function->begin(), BBEnd = function->end(); basicblock != BBEnd; basicblock++) { //???????????????Basic Block

				std::string LastLocString = "";
				
				for(BasicBlock::iterator instruction = basicblock->begin(), IEnd = basicblock->end(); instruction!=IEnd; instruction++) {	//???????????????instruction
			  
					if(Instruction *inst = dyn_cast<Instruction>(instruction)) {
						
						IRBuilder<> IRBuild(&(*(instruction)));//??????????????????????????????????????????
						
						if(inst->getOpcode() == Instruction::Ret) // Ret????????????
						{
							SmallVector<Value *, 1> retArg;//?????????Vector????????????
							Value *CurFuncVa = ConstantExpr::getBitCast(&(*function), Type::getInt8PtrTy(C));
							retArg.push_back(CurFuncVa);
							IRBuild.CreateCall(instr_ReturnFunc, retArg);
						}
						
						if(inst->getOpcode() == Instruction::Call){
							std::string instr_create = "pthread_create";
							std::string instr_join = "pthread_join";
							if(inst->getNumOperands() >= 5 ){ //???????????????5
								if (instr_create == std::string(inst->getOperand(4)->getName())){ //???pthread_create
									SmallVector<Value *, 2> createArg;//?????????Vector????????????
									Function *pthread_task = dyn_cast<Function>(inst->getOperand(2)->stripPointerCasts());
									Value *OpFuncV = ConstantExpr::getBitCast(pthread_task, Type::getInt8PtrTy(C));
									createArg.push_back(inst->getOperand(0));
									createArg.push_back(OpFuncV);
									BasicBlock::iterator tmptier = instruction;
									tmptier++;
									IRBuilder<> IRNEXT_Build(&(*(tmptier)));
									IRNEXT_Build.CreateCall(instr_CreateFunc, createArg);
								}
							}
							
							else if(inst->getNumOperands() >= 3 ){ //???????????????3
								if (instr_join == std::string(inst->getOperand(2)->getName())){ //???pthread_join
									SmallVector<Value *, 1> JoinArg;//?????????Vector????????????
									JoinArg.push_back(inst->getOperand(0));
									BasicBlock::iterator tmptier = instruction;
									tmptier++;
									IRBuilder<> IRNEXT_Build(&(*(tmptier)));
									IRNEXT_Build.CreateCall(instr_JoinFunc, JoinArg);
								}
							}
						}
						
						//???????????????????????????
						std::string SourceLoc = getSourceLoc(inst);
						
						if (LastLocString == SourceLoc) //??????????????????????????????????????????
							continue;
						LastLocString = SourceLoc;

						if (SourceLoc.size() == 0) //SourceLoc?????????????????????????????????continue
							continue;
						
						if (SourceLoc.find(":")== std::string::npos) //SourceLoc?????????????????????
							continue;
						
						std::string tmpstring("./");
						if (SourceLoc.compare(0, tmpstring.length(), tmpstring) == 0) {//??????SourceLoc??????????????????"./", ????????????
							SourceLoc.erase(0,2);
						}
												
						for (int i = 0; i < linesNum; i++){
							if (boost::algorithm::ends_with(ConLine[i], SourceLoc)) { //rather than ConLine[i] == SourceLoc
								
								outs() << "[DEBUG] Dry Run Instrumentation for " << SourceLoc << "\n";
											
								/*std::string strLocNum  = SourceLoc.substr(SourceLoc.find(":")+1);
								unsigned int LocNum = std::stoi(strLocNum); //??????????????????
								
								SmallVector<Value *, 2> locArg;//?????????Vector????????????
								Value *conInt = ConstantInt::get(Int32Ty, LocNum);
								Value *CurFuncV = ConstantExpr::getBitCast(&(*function), Type::getInt8PtrTy(C));
								locArg.push_back(CurFuncV);
								locArg.push_back(conInt);
								IRBuild.CreateCall(instr_LOCFunc, locArg);*/
								
								SmallVector<Value *, 2> slocArg;
								Value *argvStr = IRBuild.CreateGlobalStringPtr(SourceLoc);
								Value *CurFuncVa = ConstantExpr::getBitCast(&(*function), Type::getInt8PtrTy(C));
								slocArg.push_back(CurFuncVa);
								slocArg.push_back(argvStr);
								IRBuild.CreateCall(instr_SLOCFunc, slocArg);
							}
						}
					}
				}
			}
		}
	}
		
	return true;
	
}

//-----------------------------------------------------------------------------------------------------------------------------------

char Instru::ID = 0;

static RegisterPass<Instru> X("Instru", "Instru Pass");