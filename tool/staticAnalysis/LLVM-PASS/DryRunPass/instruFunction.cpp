#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <map>
#include <dlfcn.h>
#include <cxxabi.h>
#include <ctime>
#include <chrono> 
#include <sys/syscall.h>
#include <iostream>

using namespace std;
using namespace std::chrono;

#if defined(__APPLE__) || defined(__FreeBSD__) || defined (__OpenBSD__)
#  include <sched.h>
#  include <stdint.h>
#  include <sys/types.h>
#endif /* __APPLE__ || __FreeBSD__ || __OpenBSD__ */

#ifdef __linux__
#  include <linux/sched.h>
#  include <linux/types.h>
#  define gettid() syscall(__NR_gettid)
#else
#  if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12
uint64_t gettid (){
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);
    return tid;
}
#  endif
#endif


static FILE *fp_trace;
static int mainTid;
static std::map<unsigned long int, long int> mapThread2Pid;
static std::map<unsigned long int, long int>::iterator T2Piter;
static std::map<unsigned int, unsigned int> mapTid;
static std::map<unsigned int, unsigned int>::iterator Tiditer;
static system_clock::time_point startDryRun;
static system_clock::time_point endDryRun;


inline const char *get_funcname(const char *src) {
	int status = 99;
	const char *f = abi::__cxa_demangle(src, nullptr, nullptr, &status);
	return f == nullptr ? src : f;
}

void __attribute__((constructor)) traceBegin(void) {
	  
	mainTid = gettid();
	fp_trace = fopen("func_trace.log", "w");
	
	startDryRun = system_clock::now();	//程序计时开始
}

void __attribute__((destructor)) traceEnd(void) {
	
	endDryRun = system_clock::now();	//程序结束用时
	
	//输出统计时间
	auto durationDryRun = duration_cast<microseconds>(endDryRun - startDryRun);
	double DryRunTime = double(durationDryRun.count()) * microseconds::period::num / microseconds::period::den; //s为单位
	
	if (fp_trace != NULL) {
		
		int pidNum, MaxKPNum=0;
		fprintf(fp_trace, "Summary(Time: %lf (s))\n", DryRunTime);
		fprintf(fp_trace, "Summary(");
		for(Tiditer = mapTid.begin(), pidNum = 0; Tiditer != mapTid.end(); Tiditer++, pidNum++)
		{
			if(Tiditer->second > MaxKPNum)
				MaxKPNum = Tiditer->second;
			if(Tiditer != mapTid.begin())
				fprintf(fp_trace, ", ");
			fprintf(fp_trace, "%d: %d", Tiditer->first, Tiditer->second);
		}
		fprintf(fp_trace, ")\n");
		fprintf(fp_trace, "Summary(ThraedNum:%d, KPNum:%d)\n", pidNum, MaxKPNum);
		
		fclose(fp_trace);
	}
}

void instr_Call (void *func) 
{
	Dl_info info;
    if (dladdr(func, &info) && fp_trace != NULL){
		fprintf(fp_trace, "Thread %ld (%lu) enter function: %s\n", gettid(), pthread_self(), get_funcname(info.dli_sname));
	}
}

void instr_PthreadCall (void *func) 
{
	/*Dl_info info;
    if (dladdr(func, &info) && fp_trace != NULL){
		fprintf(fp_trace, "Thread %ld (%lu) enter function: %s\n", gettid(), pthread_self(), get_funcname(info.dli_sname));
	}*/
	if(mapThread2Pid.find(pthread_self())==mapThread2Pid.end()){
		mapThread2Pid.insert(pair<unsigned long int, long int>(pthread_self(), gettid()));
	}
}

void instr_Return (void *func)
{
	Dl_info info;
    if (dladdr(func, &info) && fp_trace != NULL){
        fprintf(fp_trace, "Thread %ld (%lu) exit function: %s\n", gettid(), pthread_self(), get_funcname(info.dli_sname));
	}
}

void instr_LOC (void *func ,unsigned int a) 
{
	Dl_info info;
    if (dladdr(func, &info) && fp_trace != NULL){
        fprintf(fp_trace, "Tid: %ld, Function: %s, KeyPoint: %u\n", gettid(), get_funcname(info.dli_sname), a);
		
		if(mapTid.find(gettid())!=mapTid.end())
		{
			mapTid[gettid()]++;
		} else {
			mapTid.insert(pair<unsigned int, unsigned int>(gettid(), 1));;
		}
	}
}

void instr_pthread_create(unsigned long int *threadId, void *func){
	Dl_info info;
    if (dladdr(func, &info) && fp_trace != NULL){
		int SavePID;
		if(mapThread2Pid.find(*threadId)!=mapThread2Pid.end())
			SavePID = mapThread2Pid[*threadId];
		else
			SavePID = 0;
		fprintf(fp_trace, "Thread %u (%lu) execute pthread_create(%s)\n", SavePID, *threadId, get_funcname(info.dli_sname));
	}
}
void instr_pthread_join(unsigned long int threadId){
	
	//for(T2Piter = mapThread2Pid.begin(); T2Piter != mapThread2Pid.end(); T2Piter++)
	//{
		//printf("%lu: %u\n", T2Piter->first, T2Piter->second);
	//}
	
    if (fp_trace != NULL){
		fprintf(fp_trace, "Thread %ld (%lu) execute pthread_join()\n", mapThread2Pid[threadId], threadId);
	}
}

void instr_LOC_string (void *func ,void *str) 
{
	Dl_info info;
    if (dladdr(func, &info) && fp_trace != NULL){
        fprintf(fp_trace, "Tid: %ld, Function: %s, KeyPoint: %s\n", gettid(), get_funcname(info.dli_sname), str);
		
		if(mapTid.find(gettid())!=mapTid.end())
		{
			mapTid[gettid()]++;
		} else {
			mapTid.insert(pair<unsigned int, unsigned int>(gettid(), 1));;
		}
	}
}
