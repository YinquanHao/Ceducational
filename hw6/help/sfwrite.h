#ifndef SFWRITE_H
#define SFWRITE_H
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "sfwrite.h"
void sfwrite(pthread_mutex_t *lock, FILE* stream, const char *fmt,...);
#endif
