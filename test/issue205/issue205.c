/*
 * Copyright (c） Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @description 没有正确同步的多线程任务
 *
 * @bad DataRace
 *
 * @secureCoding https://codehub-y.huawei.com/MapleC/EnhanceC/issues/205?issueTitle=%E6%95%B0%E9%80%9A%E9%97%AE%E9%A2%98-%E9%87%8E%E6%8C%87%E9%92%88-%E8%87%AA%E5%8A%A8trace%E4%B8%9A%E5%8A%A1%EF%BC%8C%E5%9B%A0trace%E6%B5%81%E7%A8%8B%E4%B8%AD%E5%AD%98%E5%9C%A8%E6%94%BE%E6%9D%83%EF%BC%8C%E5%AF%BC%E8%87%B4%E8%AE%BF%E9%97%AE%E7%A9%BA%E6%8C%87%E9%92%88
 *
 * @tool
 *
 * @author y00538860
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>



// a simple pthread example
// compile with -lpthreads
int *g_global = NULL;
void *Thread1Func(void *ptr)
{
    usleep(1000);
    free(g_global);
    g_global = NULL;
    return  ptr;
}
void *Thread2Func(void *ptr)
{
    //usleep(1000);
    /* POTENTIAL FLAW: g_global可能已经被其他线程释放并置空 */
    printf ("%d\n", *g_global);
    return  ptr;
}

int datarace_issue205()
{
    // create the thread objs
    pthread_t thread1, thread2;
    int thr = 1;
    int thr2 = 2;
    g_global = (int *)malloc(sizeof(int));
    *g_global = 24;
    // start the threads
    pthread_create(&thread2, NULL, *Thread2Func, (void *) NULL);
	pthread_create(&thread1, NULL, *Thread1Func, (void *) NULL);
    // wait for threads to finish
    pthread_join(thread1,NULL);
    pthread_join(thread2,NULL);
    return 0;
}

int main(int argc, char* argv[]) {
    datarace_issue205();
}
