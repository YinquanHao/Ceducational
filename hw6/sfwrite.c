#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "sfwrite.h"

pthread_mutex_t count_mutex;
void sfwrite(pthread_mutex_t *lock, FILE* stream, const char *fmt,...);



void sfwrite(pthread_mutex_t *lock, FILE* stream, const char *fmt,...)
{
    va_list args;
    va_start(args,fmt);
    pthread_mutex_lock(lock);
    vfprintf(stream,fmt,args);
    pthread_mutex_unlock(lock);
    va_end(args);

}
