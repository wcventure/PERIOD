/*
 * Copyright (c)  Huawei Technologies Co., Ltd. 2021-2021. All rights reserved. 
 *
 * @description  Double Free for cross thread
 *
 * @bad main
 *
 * @secureCoding
 *
 * @tool
 *
 * @author x00441286
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <malloc.h>
#include <unistd.h>
#include <pthread.h>


char *g_pointer = NULL;

static void *MemDoubleFreeThread1(void *arg)
{
    //usleep(1000);
    g_pointer = malloc(100);
    if(g_pointer == NULL) {
        return NULL;
    }

    free(g_pointer);


    return NULL;
}

static void *MemDoubleFreeThread2(void *arg)
{
    if (g_pointer != NULL) {
        free(g_pointer);
    }

    return NULL;
}

void main()
{
    pthread_t tidp1, tidp2;

    if (pthread_create(&tidp2, NULL, MemDoubleFreeThread2, NULL) == -1) {
        printf("create thread 1 error!\n");
        return;
    }

    if (pthread_create(&tidp1, NULL, MemDoubleFreeThread1, NULL) == -1) {
        printf("create thread 2 error!\n");
        return;
    }

    pthread_join(tidp2, NULL);
    pthread_join(tidp1, NULL);
}
