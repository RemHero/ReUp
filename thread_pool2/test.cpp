#include "queue_pool.h"
#include <cstdio>
#include <cstdlib>
#include "assert.h"

// 测试
// 并行化与串行化的矩阵向量乘法

struct PartJob {
	int begin; int end;
	int len; int* m;
	int* v; int* out;
};

void* partial_mxv(void* arg) {
	PartJob* p = (PartJob*)arg;
	for (int i = p->begin; i < p->end; ++i) {
		p->out[i] = 0;
		for (int j = 0; j < p->len; ++j) {
			p->out[i] += p->m[i * p->len + j] * p->v[j];
		}
	}
	return 0;
}

// 利用 OpenMP directive 进行加速
void omp_mxv(void* arg) {
	PartJob* p = (PartJob*)arg;
	int i, j;
	#pragma omp parallel for private(i, j) shared(p)
	for (i = p->begin; i < p->end; ++i) {
		p->out[i] = 0;
		for (j = 0; j < p->len; ++j) {
			p->out[i] += p->m[i * p->len + j] * p->v[j];
		}
	}
}

int main(int argc, char const *argv[]) {
	// 这个问题的并行化程度很高，能够很接近完美加速
	// 分配矩阵 m 和 v, out 的空间
	int row = 8000;
	int col = 8000;
	int thread_num = 700;
	int job_num = 400;

	int* m = (int*)malloc(row * col * sizeof(int));
	int* v = (int*)malloc(col * sizeof(int));
	int* out = (int*)malloc(row * sizeof(int));
	
	for (int i = 0; i < row * col; ++i) {
		m[i] = 1;	
	}
	for (int i = 0; i < col; ++i) {
		v[i] = 1;
	}

	// 初始化并运行线程池
	ThreadPool p(thread_num);
	p.run();

	// 拆分并分配任务
	clock_t t = clock();
	PartJob* jobs = new PartJob[job_num];
	int row_job = row / job_num;
	for (int i = 0; i < job_num; ++i) {
		jobs[i].begin = i * row_job;
		if (jobs[i].begin + row_job >= row) {
			jobs[i].end = row;
		} else {
			jobs[i].end = jobs[i].begin + row_job;
		}
		jobs[i].m = m;
		jobs[i].v = v;
		jobs[i].out = out;
		jobs[i].len = col;
		p.dispatch(partial_mxv, jobs + i);
	}
	p.sync();
	printf("parallel time : %d ms\n", clock() - t);
	// 验证运算的正确性
	for (int i = 0; i < row; ++i) {
		assert (out[i] == col);
	}

	t = clock();
	PartJob j;
	j.begin = 0;
	j.end = row;
	j.m = m;
	j.v = v;
	j.out = out;
	j.len = col;
	partial_mxv(&j);
	printf("sequential time : %d ms\n", clock() - t);

	// 验证运算的正确性
	for (int i = 0; i < row; ++i) {
		assert (out[i] == col);
	}

	t = clock();
	j.begin = 0;
	j.end = row;
	j.m = m;
	j.v = v;
	j.out = out;
	j.len = col;
	omp_mxv(&j);
	printf("OpenMP time : %d ms\n", clock() - t);

	// 验证运算的正确性
	for (int i = 0; i < row; ++i) {
		assert (out[i] == col);
	}

	delete [] jobs;
	free(m);
	free(v);
	free(out);
	printf("test over!\n");
	getchar();
	return 0;
}