#ifndef GLOBAL_H
#define GLOBAL_H

#include <mutex>
#include <queue>
#include "semaphore.h"
#include <pthread.h>
//每次接受的信息最大长度
#define MAXT 1024

extern int R;
extern int R1;
extern int W;
extern int O1;
extern int O2;
extern sem_t ROsem;

#endif



























