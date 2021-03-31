#include <iostream>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include "sudoku.h"
#include "queue_pool.h"

using namespace  std;
#define B 10
mutex __mutex;
mutex __mutex2;
// mutex _input;
// condition_variable inputEmpty;
string questionBuffer[1000];//问题缓存，最高支持1000个问题输入
int questionIn=0;//指针表示有效问题个数，从0开始
int questionLocal=0;//当前处理的文件指针

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


void* LHZFUN(void* arg){
    argT* p=(argT*) arg;
    if (solve_sudoku_dancing_links(p->Tboard)){
        __mutex2.lock();
        for(int i=0;i<N;i++) {
            boardW[i]=p->Tboard[i];
        }
        putAns(p->num,boardW);
//        if(!solved()){
//            printf("work wrong!\n");
//            assert(0);
//        }
        __mutex2.unlock();
    }
    else{
        assert(0);
        printf("wrong!\n");
    }
    return 0;
}

void* getinput(void* arg){
    //锁suspend
    string fname;
    while(cin>>fname){
        // fpT = freopen(fname,"r",stdin);
        questionBuffer[questionIn]=fname;
        questionIn++;
        cout<<questionLocal<<" "<<questionIn<<"\n";
    }
    
    pthread_exit(NULL);
}

void creatInput(){
    if(pthread_create(&inputT,NULL,getinput,NULL)){
        printf("create thread wrong!\n");
    }
}







int main(int argc, char* argv[])
{
    // FILE* fp = fopen(argv[1], "r");
//    FILE* fp = freopen("D:/ProgramFiles (x86)/JetBrains/Code/test/test1000", "r",stdin);
    // if(fp==NULL) printf("wrong! get file fail.\n");
    creatInput();
    char puzzle[128];
    int total_solved = 0;
    long long total = 0;
    ThreadPool p(7);
    p.run();
    
    while(1){
        // while(questionLocal>=questionIn){
        // }
        if(questionLocal<questionIn)questionLocal++;
    /*    FILE* fp = fopen(questionBuffer[questionLocal].c_str(),"r");




        int PT=0;
        int64_t start = now();

        while (fgets(puzzle, sizeof puzzle, fp) != NULL) {
            if (strlen(puzzle) >= N) {
                PT=(PT+1)%B;
                __mutex.lock();
                T[PT].num=total;
                ++total;
                input(puzzle);
                init_cache();
                T[PT].getT();
                p.dispatch(LHZFUN, &T[PT]);
                if(total%B==0){
                    p.sync();//分步读入文件
                }
                __mutex.unlock();
            }
        }

        p.sync();
        int64_t end = now();
        delete T;
        double sec = (end-start)/1000000.0;
        printf("%f sec %f ms each %d\n", sec, 1000*sec/total, total_solved);*/

    }
    return 0;
}






























