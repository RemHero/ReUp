// 利用 pthread 实现了 work queue + thread pool 模型
// work queue : 任务队列, 用户将需要执行任务放进队列后退出
// thread pool : 线程池, 包含若干个等待着的工作线程, 只要任务队列中有东西, 就取出来执行
//
// 要点:
//  * 只能由一个线程来管理 ThreadPool (这个类本身不是 MT-safe 的)
//  * 任务的添加是异步的, dispatch 后立刻返回, 返回后不能保证任务已经开始执行
//  * 用 work_wrapper 来包装线程函数
// 	* 通过 condition variable 在进行线程等待与线程唤醒
// 	* 通过 clean up handler 来解锁并且释放资源
//  * 使用 semaphore 构造的 barrier 对当前所有任务进行同步

// 返回实际建立的线程数

#include "pthread.h"
#include "semaphore.h"
#include <queue>
#include <stack>
#define EAGAIN 11
typedef void* (*WorkFunc)(void*);
typedef void* Argument;

// 若创建的线程超过系统允许的范围则抛出异常
struct OutOfThreadError {};
// 若线程池的创建线程数为 0 则抛出异常
struct IllegalThreadNumber{};

class Thread;
class WorkQueue;

// 线程池
class ThreadPool {
	friend class Thread;
public:
	// 初始化一次后线程数就固定了
	ThreadPool(size_t thread_num);
	~ThreadPool();
	// 开始扫描任务队列线程
	void run();
	// 往线程池的任务队列里添加任务
	void dispatch(WorkFunc func, Argument arg);
	// 同步目前队列里的所有任务, 返回后线程池空闲
	bool sync();
private:
	// 不允许默认初始化
	ThreadPool() {};
	static void cleanqueue(void* arg);
	static void cleanup(void* arg);
	static void* runner(void* arg);

	int unfinished;
	int total;
	bool running;

	sem_t* barrier;
	pthread_t* run_tid;
	pthread_cond_t* th_cond;
	pthread_mutex_t* th_lock;
	std::stack<Thread*> threads;
	
	WorkQueue* job_queue;
};
