#include <pthread.h>
#include <stdlib.h>

#define XP_UNIX
#define JS_THREADSAFE
#include "jsapi.h"

#define THREADS 1

static JSClass global_class = {
    "global", JSCLASS_GLOBAL_FLAGS,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSRuntime *rt;

static void * testfunc(void *ignored) {

    JSContext *cx = JS_NewContext(rt, 0x1000);
    if (cx == NULL) {
        exit(1);
    }

    // Fastest way to cause a crash..
    JS_SetContextThread(cx);
    JS_ClearContextThread(cx);

    // Slower to crash, but more realistic use case?
//      JS_SetContextThread(cx);
//      JS_BeginRequest(cx);
//      JS_EndRequest(cx);
//      JS_ClearContextThread(cx);

    // Avoiding Set/ClearContextThread does not cause any crash..
//      JS_BeginRequest(cx);
//      JS_EndRequest(cx);
    
    return NULL;
}

void * jsApi(void *ignored) {
    JSContext *cx;
    JSObject *global;
    
    cx = JS_NewContext(rt, 0x1000);
    if (cx == NULL)
        return NULL;

    global = JS_NewObject(cx, &global_class, NULL, NULL);
    if (global == NULL)
        return NULL;

    if (! JS_InitStandardClasses(cx, global))
        return NULL;
    
    // Does an isolated JS_GC need to be wrapped in a request? The
    // API doesn't explicitly state so, but we get the same
    // behaviour regardless of whether or not we use
    // JS_{Begin,End}Request anyway..
    JS_BeginRequest(cx);
    JS_GC(cx);
    JS_EndRequest(cx);
    return NULL;
}

int main(void) {

    rt = JS_NewRuntime(0x100000);
    if (rt == NULL)
        return 1;

    pthread_t t[THREADS];
    pthread_t js;
    
    for (int i = 0; i < THREADS; i++) {
        pthread_create(&t[i], NULL, testfunc, NULL);
    }
    
    pthread_create(&js, NULL, jsApi, NULL);
    
    pthread_join(js, NULL);
    
    for (int i = 0; i < THREADS; i++) {
        pthread_join(t[i], NULL);
    }

}

