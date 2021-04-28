#include <iostream>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <semaphore.h>
#include <mutex>
#include <sched.h>
#include <pthread.h>
#include <unistd.h>
#include "algorithm/sudoku.h"
#include "thread_pool/queue_pool.h"
#include "fileReader/fileReader.hpp"

using namespace  std;
#define B 10000
//int cpus;
//cpu_set_t set[20];
sem_t IOsem;

mutex __mutex;
mutex __mutex2;
mutex __mutex3;
bool buffer[B]={false};//标志位
int AnsBuffer[B][81]={0};//输出缓存，大小肯定要大于线程个数
long long posi=0;
pthread_t inputT;
FILE* fpT;

int64_t now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

struct argT{
    int num;
    int Tboard[N];
    void getT(){
        for(int i=0;i<N;i++)
            Tboard[i]=board[i];
    }
};
argT* T=new argT[B];

void outputAns(int a[]){
    for(int i=0;i<N;i++) {
        printf("%d",a[i]);
    }
    printf("\n");
}

void putAns(int n,int ans[]){
    int tk=n%B;
//    printf("%d\n",tk);
    buffer[tk]=true;
    for(int i=0;i<81;i++) AnsBuffer[tk][i]=ans[i];//这里的常数开销能否减小？？
    while(buffer[posi]){
        buffer[posi]=false;//基本运算
        outputAns(AnsBuffer[posi]);
        posi=(posi+1)%B;
    }
}

int count=0;
void* LHZFUN(void* arg){
//    cout << "1:"<<count++ << endl;
    argT* p=(argT*) arg;
    //pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &set[(p->num)%cpus]);
    if (solve_sudoku_dancing_links(p->Tboard)){
//        cout << "1.1:"<<count++ << endl;
        __mutex2.lock();
        for(int i=0;i<N;i++) {
            boardW[i]=p->Tboard[i];
        }
        putAns(p->num,boardW);
        /*
        if(!solved()){
            printf("work wrong!\n");
            assert(0);
        }
        */
        __mutex2.unlock();
    }
    else{
        assert(0);
        printf("wrong!\n");
    }
    return 0;
}

queue<string> filenameBuffer;
exclusiveLock filenameLock;
void* getinput(void* arg){
    //锁suspend
    string file_path;
//    __mutex3.lock();
    while(cin >> file_path)
    {
//        __mutex3.unlock();
        //cout << file_path << "------------\n";
        filenameLock.get();
        filenameBuffer.push(file_path);
        filenameLock.release();
        sem_post(&IOsem);
    }

    return 0;
}

void creatInput(){
    if(pthread_create(&inputT,NULL,getinput,NULL)){
        printf("create thread wrong!\n");
    }
}



int main(int argc,char* argv[]){
    FILE* fp=NULL;
//    FILE* fp = fopen(argv[1], "r");
//    fp = freopen("D:/ProgramFiles (x86)/JetBrains/Code/test/test1000", "r",stdin);
//    if(fp==NULL) printf("wrong! get file fail.\n");
    /*
    cpus = sysconf(_SC_NPROCESSORS_CONF);//get the num of the CPU
    for (int i=0; i<cpus; i++)
    {
        CPU_SET(i, &set[i]);
    }
    cout << "cpu num " << cpus << endl; 
*/
    if (sem_init(&IOsem,0,0)) {
        printf("Semaphore initialization failed!!\n");
        exit(EXIT_FAILURE);
    }

    char puzzle[128];
    int total_solved = 0;
    long long total = 0;
    creatInput();
    ThreadPool p(7);
    p.run();
    int PT=0;
    while(1) {
        sem_wait(&IOsem);
        string file_path;
        if (!filenameBuffer.empty()) {
            filenameLock.get();
            file_path = filenameBuffer.front();
            filenameBuffer.pop();
            fp = fopen(file_path.c_str(), "r");
            filenameLock.release();
        }
        if (fp != NULL) {
            int64_t start = now();
            while (fgets(puzzle, sizeof puzzle, fp) != NULL) {
                if (strlen(puzzle) >= N) {
                    PT = (PT + 1) % B;
                    __mutex.lock();
                    T[PT].num = total;
                    ++total;
                    input(puzzle);
                    init_cache();
                    T[PT].getT();
                    p.dispatch(LHZFUN, &T[PT]);
                    if (total % B == 0) {
                        p.sync();//分步读入文件
                    }
                    __mutex.unlock();
                }
            }
            p.sync();
            fclose(fp);
            //int64_t end = now();
            //double sec = (end - start) / 1000000.0;
            //printf("%f sec %f ms each %d\n", sec, 1000 * sec / total, total_solved);
            total=0;
            count=0;
            posi=0;
        }else{
            printf("access of file failed!");
            delete T;
            exit(0);
        }
    }
    delete T;
    return 0;
}
//D:/Desktop/test1000




