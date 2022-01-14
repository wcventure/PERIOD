#include "sched.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <map>
#include <dlfcn.h>
#include <cxxabi.h>
#include <ctime>
#include <chrono>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include <fstream>
#include <iostream>
#include <sys/syscall.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/syscall.h>
#ifdef __linux__
#  include <linux/kernel.h>
#  include <linux/sched.h>
#  include <linux/types.h>
#endif /* __linux__ */
//#define SAME_THREAD_CONSIDER

using namespace std;
using namespace std::chrono;

#define SHARE_MEMORY_THREAD_NUM 100
#define SHARE_MEMORY_ARRAY_SIZE 3964
#define SAME_THREAD_CONSIDER_NUM 4
int LOOP_UNWINDED_NUM = 4;

typedef struct Sched_Info {
	uint64_t time;										// the execution time of the program (nsec)
	unsigned int N;										// number of threads concerned
	unsigned int M;										// number of kep points per thread
	unsigned int TestedBounds;							// It took several period in testing engine
	//unsigned int tupleScheduling;						// now considering tuple scheduling :1, otherwise: 0; Then it save the hash ID of first touched key point
	uint16_t kpNumArray[SHARE_MEMORY_THREAD_NUM];		// the specific kpNum of each My_tid Thread, only consider almost 10 thread
	uint16_t kp[SHARE_MEMORY_ARRAY_SIZE];				// how many times should a thread yield before a kp
	uint16_t kpLoc[SHARE_MEMORY_ARRAY_SIZE];			// the hash ID of Key Point
	uint16_t kpOrder[SHARE_MEMORY_ARRAY_SIZE];			// recored the order of key points
} sched_info;

static bool NeedDryRun = false;
static bool DBDSRun = true;
static bool NoTrace = false;
//static bool TupleScheduling = false;
static int num_cpu = 0;						// number of available CPUs
static int num_td = 0;						// number of threads concerned
static int num_kp = 0;						// number of kep points per thread
#ifdef SAME_THREAD_CONSIDER
	static int tested_bounds = 0;				// It took several period in testing engine
#endif
static uint64_t DBDSruntime = 1000*1000;	// nsec
static unsigned int kpYield[SHARE_MEMORY_ARRAY_SIZE] = {0};	// how many times should a thread yield before a kp
static int c_tid = 0;						// child tid, initially 0
static uint16_t curSyncOrder = 1;				// recored the current order of key point, initially 1
static int joinable = 0;					// Determine whether pthread_join is encountered

static FILE *fp_trace;
static int mainTid;

static std::map<long int, unsigned int> mapTid;                 // the map form tid to dynamic number of Key Points
static std::map<long int, unsigned int>::iterator Tiditer;

// traceEnd函数执行之前std::map就会释放资源，使用map的时候可能由于iter++导致出错，谨慎用
// std::map would release resource without ASAN
static std::map<long int, int> my_tidMap;                       // the map from tid to My_Tid
static std::map<long int, int>::iterator my_tidMapiter;
static std::map<long int, long int> idx_kpMap;                  // the map from tid to idx_kp
static std::map<long int, long int> idx_original_kpMap;         // the map from tid to idx_kp (Initial Value)
static std::map<long int, long int>::iterator idx_kpMapiter;
static std::map<unsigned long int, long int> mapThread2Pid;     // the map from thread ID to tid
static std::map<unsigned long int, long int>::iterator T2Piter;

#ifdef SAME_THREAD_CONSIDER
	static std::map<unsigned long int, unsigned int> mapFunc2Num;
#endif

static system_clock::time_point startDryRun;
static system_clock::time_point endDryRun;

sched_info * si;                                                //the information used to guide the scheduling

inline const char *get_funcname(const char *src) {
	int status = 99;
	const char *f = abi::__cxa_demangle(src, nullptr, nullptr, &status);
	return f == nullptr ? src : f;
}

bool isBoundedLocHash(uint16_t intHash, int start, int end) {
	int hitCount = 0;
	for (int i = start; i < end; i++) {
		if (si->kpLoc[i] == intHash)
			hitCount++;
		if (hitCount >= LOOP_UNWINDED_NUM)
			return false;
	}
	return true;
}

double max(double a, double b){
	if (a >= b)
		return a;
	else 
		return b;
}

int DBDSdetectCPUs()
{
	int ncpu;
	
	// Set default to 1 in case there is no auto-detect
	ncpu = 1;

	// Autodetect the number of CPUs on a box, if available
	#if defined(__APPLE__)
		size_t len = sizeof(ncpu);
		int mib[2];
		mib[0] = CTL_HW;
		mib[1] = HW_NCPU;
		if (sysctl(mib, 2, &ncpu, &len, 0, 0) < 0 || len != sizeof(ncpu))
			ncpu = 1;
	#elif defined(_SC_NPROCESSORS_ONLN)
		ncpu = sysconf(_SC_NPROCESSORS_ONLN);
	#elif defined(WIN32)
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		ncpu = si.dwNumberOfProcessors;
	#endif

	// Ensure we have at least one processor to use
	if (ncpu < 1)
		ncpu = 1;

	// Ensure we have at most 16 processor to use
	//if (ncpu >= 16)
		//ncpu = 16;

	return ncpu;
}

void setDeadLineSched (uint64_t DBDSruntime) {

	double weight = 1;
	double single_core_baise = 1;
	if (num_td < 10)
		weight = 10;
	else {
		double weight_cpu = 1;
		if (num_cpu <= 1)
			weight_cpu=0.9;
		else if (num_cpu <= 2)
			weight_cpu=0.95;
		else if (num_cpu <= 4)
			weight_cpu = (double)num_cpu/4;
		else if (num_cpu <= 8)
			weight_cpu = 1 + (double)(num_cpu-4)/8;
		else if (num_cpu <= 12)
			weight_cpu = 1 + 0.5 + (double)(num_cpu-8)/16;
		else
			weight_cpu = 1.75;

		if (weight_cpu != 0) {
			if (num_td <= 2)
				weight = 9 + (double)(num_td)*10/(weight_cpu*5);
			else if (num_td <= 4)
				weight = 9 + (double)(num_td)*6/(weight_cpu*5);
			else if (num_td <= 8)
				weight = 9 + (double)(num_td)*4/(weight_cpu*5);
			else if (num_td <= 16)
				weight = 10 + (double)(num_td)*2/(weight_cpu*5);
			else if (num_td <= 32)
				weight = 10 + (double)(num_td)*1.5/(weight_cpu*5);
                        else if (num_td <= 65)
                                weight = 10 + (double)(num_td)*1.25/(weight_cpu*5);
			else if (num_td > 65)
				weight = 10 + (double)(num_td)/(weight_cpu*5);
		}

		//printf("[DEBUG] num_cpu = %d, weight_cpu = %f\n", num_cpu, weight_cpu);
	}

	// Adjust parameters to prevent "sched_setattr error: Device or resource busy" error
	if (num_cpu <= 1 && num_td <= 8)
		single_core_baise = 1.1;
	else if (num_cpu <= 1 && num_td <= 16)
                single_core_baise = 1.15;
	else if (num_cpu <= 1 && num_td <= 32)
                single_core_baise = 1.5;
	else if (num_cpu <= 1 && num_td <= 64)
                single_core_baise = 4;
	else if (num_cpu <= 1 && num_td > 65)
		single_core_baise = 11.5*((double)num_td/100);
	else if (num_cpu <= 2 && num_td <= 8)
		single_core_baise = 1.05;
	else if (num_cpu <=2 && num_td <= 16)
		single_core_baise = 1.15;
	else if (num_cpu <=2 && num_td <= 32)
                single_core_baise = 1.5;
	else if (num_cpu <= 2 && num_td <= 64)
                single_core_baise = 2.5;
	else if (num_cpu <= 2 && num_td > 65)
                single_core_baise = 3*((double)num_td/100);
        else if (num_cpu <= 1 && num_td <=8)
		single_core_baise = 1;
	else if (num_cpu <= 2 && num_td <= 16)
                single_core_baise = 1.025;
	else if (num_cpu <= 2 && num_td <= 32)
                single_core_baise = 1.05;
        else if (num_cpu <= 4 && num_td <= 65)
                single_core_baise = 1.1;
	else if (num_cpu <= 4 && num_td > 65)
		single_core_baise = 1.45*((double)num_td/100);
	
	if 	(single_core_baise < 1)
		single_core_baise = 1;
	//printf("[DEBUG] single_core_baise = %f\n", single_core_baise);

	if (DBDSruntime < 10000) { // DBDSruntime is the observed execution time in dry run stage
		set_sched_dl((uint64_t)(DBDSruntime * weight * 5), (uint64_t)(DBDSruntime * weight * 100 * single_core_baise), (uint64_t)(DBDSruntime * weight * 100 * single_core_baise));
		//set_sched_dl((uint64_t)(DBDSruntime * 100), (uint64_t)(DBDSruntime * 900), (uint64_t)(DBDSruntime * 1000));
	} else if (10000 <= DBDSruntime && DBDSruntime < 100000) {
		set_sched_dl((uint64_t)(DBDSruntime * weight * 4), (uint64_t)(DBDSruntime * weight * 75 * single_core_baise), (uint64_t)(DBDSruntime * weight * 75 * single_core_baise));
		//set_sched_dl((uint64_t)(DBDSruntime * 80), (uint64_t)(DBDSruntime * 700), (uint64_t)(DBDSruntime * 800));
	} else if (100000 <= DBDSruntime && DBDSruntime < 500000) {
		set_sched_dl((uint64_t)(DBDSruntime * weight * 3), (uint64_t)(DBDSruntime * weight * 50 * single_core_baise ), (uint64_t)(DBDSruntime * weight * 50 * single_core_baise));
		//set_sched_dl((uint64_t)(DBDSruntime * 60), (uint64_t)(DBDSruntime * 700), (uint64_t)(DBDSruntime * 800));
	} else if (500000 <= DBDSruntime && DBDSruntime < 1000000) {
                set_sched_dl((uint64_t)(DBDSruntime * weight * 2), (uint64_t)(DBDSruntime * weight * 25 * single_core_baise), (uint64_t)(DBDSruntime * weight * 25 * single_core_baise));
		//...
	} else if (1000000 <= DBDSruntime && DBDSruntime < 2000000) {
		set_sched_dl((uint64_t)(DBDSruntime * weight * 1.5), (uint64_t)(DBDSruntime * weight * 15 * single_core_baise), (uint64_t)(DBDSruntime * weight * 15 * single_core_baise));
		//set_sched_dl((uint64_t)(DBDSruntime * 40), (uint64_t)(DBDSruntime * 500), (uint64_t)(DBDSruntime * 600));
	} else {
		set_sched_dl((uint64_t)(DBDSruntime * weight * 1), (uint64_t)(DBDSruntime * weight * 7.5), (uint64_t)(DBDSruntime * weight * 7.5 * single_core_baise));
		//set_sched_dl((uint64_t)(DBDSruntime * 20), (uint64_t)(DBDSruntime * 100), (uint64_t)(DBDSruntime * 200));
	}
		
}

void __attribute__((constructor)) traceBegin(void) {
  
	mainTid = gettid();

	//获取环境变量，看是否需要DryRun
	char *Dry_Run = NULL;
	Dry_Run = getenv("Dry_Run");
	
	if (Dry_Run != NULL && strcmp(Dry_Run, "1") == 0){
		NeedDryRun = true;
	}
	
	//获取环境变量，看是否DBDS
	char *DBDS_Run = NULL;
	DBDS_Run = getenv("DBDS_Run");
	
	if (DBDS_Run != NULL && strcmp(DBDS_Run, "0") == 0){
		DBDSRun = false;
	}
	
	//获取环境变量，看是否DBDS
	char *NO_Trace = NULL;
	NO_Trace = getenv("NO_Trace");
	
	if (NO_Trace != NULL && strcmp(NO_Trace, "0") == 1){
		NoTrace = true;
	}
	
	//配置共享内存
	int shmid;
	key_t key = 1111;
	
	// Setup shared memory
	if ((shmid = shmget(key, sizeof(sched_info), IPC_CREAT | 0666)) < 0) {
		printf("Error: getting shared memory id. 1\n");
		DBDSRun = false;
	}
	// Attached shared memory
	if ((sched_info *) -1 == (si = (sched_info *)shmat(shmid, NULL, 0))) {
		printf("Error: attaching shared memory id. 2\n");
		DBDSRun = false;
	}

	// Initialize kpNumArray
	for (int i = 0; i < SHARE_MEMORY_THREAD_NUM; i++){
		si -> kpNumArray[i] = 0;
	}
	
	// Initialize kpLoc, kpOrder
	for (int i = 0; i < SHARE_MEMORY_ARRAY_SIZE; i++){
		si -> kpLoc[i] = 0;
		si -> kpOrder[i] = 0;
	}

#ifdef SAME_THREAD_CONSIDER
	tested_bounds = si -> TestedBounds;
#endif

	//获取环境变量，修改LOOP_UNWINDED_NUM
	char *LOOP_unwinded = NULL;
	LOOP_unwinded = getenv("LOOP_UNWINDED_NUM");
	if (LOOP_unwinded != NULL && strcmp(LOOP_unwinded, "1") == 0){
		LOOP_UNWINDED_NUM = 1;
	} else if (LOOP_unwinded != NULL && strcmp(LOOP_unwinded, "2") == 0) {
		LOOP_UNWINDED_NUM = 2;
	} else if (LOOP_unwinded != NULL && strcmp(LOOP_unwinded, "3") == 0) {
		LOOP_UNWINDED_NUM = 3;
	}

	// check whether in TupleScheduling
	//if (si -> tupleScheduling == 1)
		//TupleScheduling = true;
    
	set_tid_to_run(0);
	
	//start DBDSRun
	if (DBDSRun) {
		
		num_cpu = DBDSdetectCPUs();
		num_td = si -> N;
		num_kp = si -> M;
		DBDSruntime = si -> time;
		
		for (int i = 0; i < num_td*num_kp && i < SHARE_MEMORY_ARRAY_SIZE; i++)
			kpYield[i] = si -> kp[i];
	}
	
	//start NeedDryRun
	if (NeedDryRun) {
		if (NoTrace==true)			
			fp_trace = NULL;
		else
			fp_trace = fopen("func_trace.log", "w");
		startDryRun = system_clock::now();	//程序计时开始
	}

}

void __attribute__((destructor)) traceEnd(void) {
	
	//printf("[DEBUG] enter traceEnd\n");
	
	if (joinable == 0) {
		printf("[DEBUG] found potential thread leak\n");
		usleep(1000000);
	}

	unsigned int pidNum, MaxKPNum=0;
	for(Tiditer = mapTid.begin(), pidNum = 0; Tiditer != mapTid.end(); Tiditer++, pidNum++)
	{
		//printf("[DEBUG] %ld, %ld, %ld\n", Tiditer->first, Tiditer->second, mainTid);
		if (Tiditer->first == mainTid)
			continue;

		if(Tiditer->second > MaxKPNum)
			MaxKPNum = Tiditer->second;
		
		// std::map would release resource without ASAN, So here I use my_tidMap[Tiditer->first - 1] instead of my_tidMap[Tiditer->first]-1.
		int myThreadIndex = my_tidMap[Tiditer->first - 1]; // ignore main thread, here need to use (my_tid-1)
		if (myThreadIndex == 0) { // exception due to unconsidered thread
			myThreadIndex = my_tidMap[Tiditer->first] - 1;
		}
		
		if (myThreadIndex < SHARE_MEMORY_THREAD_NUM){
			si -> kpNumArray[myThreadIndex] = Tiditer->second;
		}
	}
	//printf("[DEBUG] leave traceEnd\n");
	
	si -> N = pidNum - 1;
	si -> M = MaxKPNum;

	if (NeedDryRun) {
		
		endDryRun = system_clock::now();	//程序结束用时
		
		auto durationDryRun = duration_cast<microseconds>(endDryRun - startDryRun);
		double DryRunTime = double(durationDryRun.count()) * microseconds::period::num / microseconds::period::den; //s为单位
		
		si -> time = (uint64_t)(DryRunTime*1000*1000);

		if (fp_trace != NULL) {
			
			fprintf(fp_trace, "Summary(Time: %lf (s))\n", DryRunTime);
			fprintf(fp_trace, "Summary(");
			for(Tiditer = mapTid.begin(); Tiditer != mapTid.end(); Tiditer++)
			{
				if(Tiditer != mapTid.begin())
					fprintf(fp_trace, ", ");
				fprintf(fp_trace, "%ld: %u", Tiditer->first, Tiditer->second);
			}
			fprintf(fp_trace, ")\n");
			fprintf(fp_trace, "Summary(ThraedNum:%d, KPNum:%d)\n", pidNum-1, MaxKPNum);
			
			fclose(fp_trace);
		}
	}

}

void instr_Call (void *func) 
{
	if (NeedDryRun) {
		Dl_info info;
		if (dladdr(func, &info) && fp_trace != NULL){
			fprintf(fp_trace, "Thread %ld (%lu) enter function: %s\n", gettid(), pthread_self(), get_funcname(info.dli_sname));
		}
	}
}

void instr_Return (void *func)
{
	if (NeedDryRun) {
		Dl_info info;
		if (dladdr(func, &info) && fp_trace != NULL){
			fprintf(fp_trace, "Thread %ld (%lu) exit function: %s\n", gettid(), pthread_self(), get_funcname(info.dli_sname));
		}
	}
}

void instr_LOC (void *func ,unsigned int a) 
{
    if(mapTid.find(gettid())!=mapTid.end())
    {
        mapTid[gettid()]++;
    } else {
        mapTid.insert(pair<long int, unsigned int>(gettid(), 1));
    }
    
    if (NeedDryRun) {
		Dl_info info;
		
		if (dladdr(func, &info) && fp_trace != NULL){
			fprintf(fp_trace, "Tid: %ld, Function: %s, KeyPoint: %u\n", gettid(), get_funcname(info.dli_sname), a);
		}
	}
}

void instr_PthreadCall (void *func) //pthread_create中调用的那个函数
{
	// check repeatation
	if (gettid() == mainTid) {
		return;
    }
	if(my_tidMap.find(gettid())!=my_tidMap.end()){
		return;
	}
	// check ending

#ifdef SAME_THREAD_CONSIDER
	if(mapFunc2Num.find((unsigned long int)func)==mapFunc2Num.end()){
		mapFunc2Num.insert(pair<unsigned long int, unsigned int>((unsigned long int)func, 1));
	} else {
		mapFunc2Num[(unsigned long int)func] ++;
	}
#endif

	while(get_tid_to_run() == 0);		// wait until its his turn
	int sched_tid = get_tid_to_run();	// acquire the tid for sechduling

#ifdef SAME_THREAD_CONSIDER
	if (mapFunc2Num[(unsigned long int)func] <= SAME_THREAD_CONSIDER_NUM) {
#endif
		if(mapThread2Pid.find(pthread_self())==mapThread2Pid.end()){
			mapThread2Pid.insert(pair<unsigned long int, long int>(pthread_self(), gettid()));
		} else {
			mapThread2Pid[pthread_self()] = gettid();
		}
		
		if(my_tidMap.find(gettid())==my_tidMap.end()){
			my_tidMap.insert(pair<long int, int>(gettid(), sched_tid));
		}
		
		if(idx_kpMap.find(gettid())==idx_kpMap.end()){
			long int tid2index;
			if (DBDSRun) {
				tid2index = (sched_tid-1) * num_kp;	
			} else {
				tid2index = (sched_tid-1) * (SHARE_MEMORY_ARRAY_SIZE/10); // the save space for dry run only consider 10 thread
				if (tid2index >= SHARE_MEMORY_ARRAY_SIZE) {// prevent buffer-overflow in Dry run
					tid2index = 0;
				}
			}
			idx_kpMap.insert(pair<long int, long int>(gettid(), tid2index));
			idx_original_kpMap.insert(pair<long int, long int>(gettid(), tid2index));
			if (DBDSRun) {
				setDeadLineSched(DBDSruntime);
			}
		}
#ifdef SAME_THREAD_CONSIDER
	} else {
		c_tid --;
		if (DBDSRun) {
			setDeadLineSched(DBDSruntime);
			// We have try to yield at this time, but no use. So we yield at first KP
		}
	}
#endif    
    set_tid_to_run(0);					// notify main thread to run

}

void before_pthread_create (){

	while(get_tid_to_run() != 0);		// wait until its his turn
	c_tid ++;							// get the next child tid
	set_tid_to_run(c_tid);				// notify thread 1 to get ready
}

void instr_pthread_create (unsigned long int *threadId, void *func){

	if (NeedDryRun) {

		Dl_info info;
		long int SavePID;

		if(mapThread2Pid.find(*threadId)!=mapThread2Pid.end())
			SavePID = mapThread2Pid[*threadId];
		else
			SavePID = 0;
	
		if (fp_trace != NULL && dladdr(func, &info)){
			fprintf(fp_trace, "Thread %ld (%lu) execute pthread_create(%s)\n", SavePID, *threadId, get_funcname(info.dli_sname));
		}
	}
}

void instr_pthread_join (unsigned long int threadId){
	joinable = 1;
	if (NeedDryRun) {
		if (fp_trace != NULL){
			fprintf(fp_trace, "Thread %ld (%lu) execute pthread_join()\n", mapThread2Pid[threadId], threadId);
		}
	}
}

void instr_LOC_string (void *func, uint16_t intHash) 
{
#ifdef SAME_THREAD_CONSIDER
	if (my_tidMap[gettid()] == 0) {							// (my_tidMap[gettid()] == 0) indicate that this tid didn't be added to map, thus no need to consider it
		static __thread int yieldFlag = 0;
		if (yieldFlag == 0){
			for (int i = 0; i < tested_bounds; i++)
				sched_yield();
			yieldFlag = 1;
		}
		return;
	}
#endif
	
	long int thisTid = gettid();
	bool isMainTid = false;
	uint16_t encodingNum = 0;								// hash encoding

	if (thisTid == mainTid) {
		isMainTid = true;
	}

	bool loopUnwinded = isBoundedLocHash(intHash, idx_original_kpMap[thisTid], idx_kpMap[thisTid]);
	
	if (loopUnwinded) {
		if(mapTid.find(thisTid)!=mapTid.end())				// record kp number
		{
			mapTid[thisTid]++;
		} else {
			mapTid.insert(pair<long int, unsigned int>(thisTid, 1));
		}

		if(!isMainTid){
			encodingNum = intHash;
		}
	}

	if (DBDSRun){
		if(loopUnwinded && !isMainTid){

			//printf("Thread %d (%lu): %d times to yield at kpYield[%lu] (%s).\n", my_tidMap[thisTid], thisTid, kpYield[idx_kpMap[thisTid]], idx_kpMap[thisTid], str);
			
			while(1) {										// perform DBDS yield pattern
				if(kpYield[idx_kpMap[thisTid]] <= 0) break;
				sched_yield();
				kpYield[idx_kpMap[thisTid]]--;
			}

			/*if (TupleScheduling){							// record the who first execute
				if (si -> tupleScheduling == 1)
					si -> tupleScheduling = encodingNum;
				else
					;
			}*/
		}
	}

	if(loopUnwinded && !isMainTid){
		si -> kpLoc[idx_kpMap[thisTid]] = encodingNum;		// save the hash ID
		uint16_t kp_order = __sync_fetch_and_add(&curSyncOrder, 1);
		si -> kpOrder[idx_kpMap[thisTid]] = kp_order;		// save the kp execution order
	}
    
    if(loopUnwinded && !isMainTid){
		if (DBDSRun) {
			if (idx_kpMap[thisTid] >= idx_original_kpMap[thisTid] + num_kp) {
				// if idx_kpMap[thisTid] reach the end of the bound of the thread, move idx_kpMap[thisTid] to the end.
				idx_kpMap[thisTid] = num_td*num_kp;	
			} else {
				// increase one
				idx_kpMap[thisTid]++;
			}
		} else {
			idx_kpMap[thisTid]++;
		}
    }
    
	if (NeedDryRun) {
		Dl_info info;
			
		if (dladdr(func, &info) && fp_trace != NULL){
			fprintf(fp_trace, "Tid: %ld, Function: %s, KeyPoint: %u\n", thisTid, get_funcname(info.dli_sname), intHash);
		}
	}
}
