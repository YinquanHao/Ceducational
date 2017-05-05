#include "sfwrite.h"
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t count_mutex;
void sfwrite(pthread_mutex_t *lock, FILE* stream, const char *fmt,...);





/*
#include <stdarg.h>
#include <stdio.h>

int add_em_up (int count,...)
{
  va_list ap;
  int i, sum;

  va_start (ap, count);
  sum = 0;
  for (i = 0; i < count; i++)
    sum += va_arg (ap, int);
  va_end (ap);
  return sum;
}
*/
int main (void)
{
  FILE* fp = stdout;
  sfwrite(&count_mutex,fp,"argument1 %d argument 2 %d\n",1,2);
  return 0;
}



void sfwrite(pthread_mutex_t *lock, FILE* stream, const char *fmt,...)
{
    va_list args;
    va_start(args,fmt);
    pthread_mutex_lock(lock);
    vfprintf(stream,fmt,args);
    pthread_mutex_unlock(lock);
    va_end(args);

}
