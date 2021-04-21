#include "queue_pool.h"
#include <assert.h>

struct Job {
	WorkFunc w_func;
	Argument arg;
};

// 任务队列
// 能够安全地 deferred cancel
class WorkQueue {
public:
	WorkQueue();
	~WorkQueue();
	void add(Job j);
	// 若队列为空则线程挂起
	Job get();
private:
	static void cleanup(void* arg);
	std::queue<Job> jobs;
	pthread_cond_t cond;
	pthread_mutex_t lock;
};

// 线程池中工作线程的封装
class Thread {
public:
	// 创建后马上进入等待工作阶段
	Thread(ThreadPool* p);
	~Thread() {};
	// 给线程分派工作
	void create();
	void give_job(Job j);
	void stop();
private:
	// 不允许默认初始化
	Thread() {};
	static void* work_wrapper(void* arg);
	static void cleanup(void* arg);

	// 线程的当前工作
	Job job;
	// 通过它来通知线程池任务已结束
	ThreadPool* pool;
	// 通过 is_idle + cond + lock 实现任务等待
	bool is_idle;
	pthread_mutex_t lock;
	pthread_cond_t cond;
	pthread_t tid;
};

// 等待任务时被 cancel 的清理函数, 解开锁
void WorkQueue::cleanup(void* arg) { 
	WorkQueue* self = (WorkQueue*)arg;
	pthread_mutex_unlock(&self->lock);
}

WorkQueue::WorkQueue() {
	pthread_mutex_init(&lock, 0);
	pthread_cond_init(&cond, 0);
}

WorkQueue::~WorkQueue() {
	pthread_mutex_unlock(&lock);
	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&cond);
}

void WorkQueue::add(Job j) {
	// 对工作数目没有上限
	pthread_mutex_lock(&lock);
	jobs.push(j);
	pthread_mutex_unlock(&lock);
	// 加入任务后通知等待取任务的线程
	pthread_cond_signal(&cond);
}

Job WorkQueue::get() {
	Job fetched;
	pthread_mutex_lock(&lock);
	pthread_cleanup_push(cleanup, this);
	while (jobs.empty()) {
		pthread_cond_wait(&cond, &lock);
	}
	fetched = jobs.front();
	jobs.pop();
	pthread_cleanup_pop(1);
	return fetched;
}

// 线程工作的封装函数
void* Thread::work_wrapper(void* arg) {
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, 0);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
	Thread* th = (Thread*)arg;
	while (1) {
		pthread_mutex_lock(&th->lock);
		pthread_cleanup_push(cleanup, th);
		while (th->is_idle) {
			pthread_cond_wait(&th->cond, &th->lock);
		}
		// 执行任务
		th->job.w_func(th->job.arg);
		th->is_idle = true;
		pthread_cleanup_pop(0);
		pthread_mutex_unlock(&th->lock);
		// 执行完后将线程放回线程池中
		pthread_mutex_lock(th->pool->th_lock);
		th->pool->threads.push(th);
		pthread_mutex_unlock(th->pool->th_lock);
		sem_post(th->pool->barrier);
		// 只有可用线程为空的时候才等待 th_cond, 该信号结束线程池的等待
		pthread_cond_signal(th->pool->th_cond);
	}
	return 0;
}

// 只能调用 stop 结束线程, 并通过间接调用下面的 cleanup handler 释放内存
void Thread::cleanup(void* arg) {
	Thread* th = (Thread*)arg;
	pthread_mutex_unlock(&th->lock);
	pthread_mutex_destroy(&th->lock);
	pthread_cond_destroy(&th->cond);
	delete th;
}

Thread::Thread(ThreadPool* p) {
	lock = PTHREAD_MUTEX_INITIALIZER;
	cond = PTHREAD_COND_INITIALIZER;
	is_idle = true;
	pool = p;
}

void Thread::create() {
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	int result = pthread_create(&tid, &attr, work_wrapper, this);
	pthread_attr_destroy(&attr);
	if (result == EAGAIN) {
		throw OutOfThreadError();
	}
}

// 为线程分派工作
void Thread::give_job(Job j) {
	pthread_mutex_lock(&lock);
	job = j;
	is_idle = false;
	pthread_mutex_unlock(&lock);
	pthread_cond_signal(&cond);
}

void Thread::stop() {
	pthread_cancel(tid);
	pthread_join(tid, 0);
}

// 线程池的任务扫描线程
void* ThreadPool::runner(void* arg) {
	ThreadPool* self = (ThreadPool*)arg;
	Thread* th;
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, 0);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
	pthread_cleanup_push(cleanqueue, self);
	while (1) {
		Job j;
		pthread_cleanup_push(cleanup, self);
		j = self->job_queue->get();
		pthread_mutex_lock(self->th_lock);
		while (self->threads.empty()) {
			pthread_cond_wait(self->th_cond, self->th_lock);
		}
		th = self->threads.top();
		self->threads.pop();
		pthread_cleanup_pop(0);
		pthread_mutex_unlock(self->th_lock);
		th->give_job(j);
	}
	pthread_cleanup_pop(1);
	return 0;
}

void ThreadPool::cleanup(void* arg) {
	ThreadPool* pool = (ThreadPool*)arg;
	sem_destroy(pool->barrier);
	pthread_mutex_unlock(pool->th_lock);
	pthread_mutex_destroy(pool->th_lock);
	pthread_cond_destroy(pool->th_cond);
	delete pool->th_lock;
	delete pool->th_cond;
	delete pool->run_tid;
	delete pool->barrier;
	pool->th_cond = 0;
	pool->th_lock = 0;
	pool->run_tid = 0;
	pool->barrier = 0;
}

void ThreadPool::cleanqueue(void* arg) {
	ThreadPool* pool = (ThreadPool*)arg;
	delete pool->job_queue;
	pool->job_queue = 0;
}

void ThreadPool::setNum(size_t thread_num){
	if (thread_num == 0) {
		throw IllegalThreadNumber();
	}
	unfinished = 0;
	total = thread_num;
	barrier = new sem_t;
	th_cond = new pthread_cond_t;
	th_lock = new pthread_mutex_t;
	run_tid = new pthread_t;
	job_queue = new WorkQueue;
	running = false;
	sem_init(barrier, 0, 0);
	pthread_cond_init(th_cond, 0);
	pthread_mutex_init(th_lock, 0);

	for (int i = 0; i < thread_num; ++i) {
		Thread* th = new Thread(this);
		try {
			th->create();
		} catch (const OutOfThreadError& err) {
			// 若新建线程发生错误, 释放所有资源后将异常继续抛出
			while (!threads.empty()) {
				Thread* t = threads.top();
				threads.pop();
				t->stop();
			}
			cleanup(this);
			throw err;
		}
		pthread_mutex_lock(th_lock);
		threads.push(th);
		pthread_mutex_unlock(th_lock);
	}
}

ThreadPool::ThreadPool(size_t thread_num) {
	if (thread_num == 0) {
		throw IllegalThreadNumber();
	}
	unfinished = 0;
	total = thread_num;
	barrier = new sem_t;
	th_cond = new pthread_cond_t;
	th_lock = new pthread_mutex_t;
	run_tid = new pthread_t;
	job_queue = new WorkQueue;
	running = false;
	sem_init(barrier, 0, 0);
	pthread_cond_init(th_cond, 0);
	pthread_mutex_init(th_lock, 0);

	for (int i = 0; i < thread_num; ++i) {
		Thread* th = new Thread(this);
		try {
			th->create();
		} catch (const OutOfThreadError& err) {
			// 若新建线程发生错误, 释放所有资源后将异常继续抛出
			while (!threads.empty()) {
				Thread* t = threads.top();
				threads.pop();
				t->stop();
			}
			cleanup(this);
			throw err;
		}
		pthread_mutex_lock(th_lock);
		threads.push(th);
		pthread_mutex_unlock(th_lock);
	}
} 

// 直到所有任务结束后才返回
ThreadPool::~ThreadPool() {
	// 完成未完成的任务
	sync();
	// 释放工作线程资源
	while (!threads.empty()) {
		Thread* th = threads.top();
		th->stop();
		threads.pop();
	}
	pthread_mutex_unlock(th_lock);
	// 最后关闭 runner 线程, 并等待其结束
	pthread_cancel(*run_tid);
	pthread_join(*run_tid, 0);
}

// 开始扫描任务队列线程
void ThreadPool::run() {
	// 线程池只能开始一次
	if (!running) {
		running = true;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		int result = pthread_create(run_tid, &attr, runner, this);
		pthread_attr_destroy(&attr);
		if (result == EAGAIN) {
			while (!threads.empty()) {
				Thread* th = threads.top();
				th->stop();
				threads.pop();
			}
			cleanup(this);
			throw OutOfThreadError();
		}
	}
}

// 往线程池的任务队列里添加任务
void ThreadPool::dispatch(WorkFunc func, Argument arg) {
	Job j;
	j.arg = arg;
	j.w_func = func;
	++unfinished;
	job_queue->add(j);
}

// 同步目前队列里的所有任务, 返回后线程池空闲
bool ThreadPool::sync() {
	if (!running) {
		return false;
	}
	while (unfinished != 0) {
		sem_wait(barrier);
		--unfinished;
	}
	return true;
}
