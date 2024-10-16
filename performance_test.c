#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>  // 用于 sysconf 函数（POSIX）
#endif
#include "performance_test.h"

#define ITERATIONS 1000000000  // 迭代次数，可以根据需要调整

void *multithreaded_task(void *arg) {
    long sum = 0;
    for (long i = 0; i < ITERATIONS / *((int *)arg); i++) {
        sum += i;
    }
    return NULL;
}

void run_single_core_test() {
    clock_t start, end;
    double cpu_time_used;
    
    start = clock();
    long sum = 0;
    for (long i = 0; i < ITERATIONS; i++) {
        sum += i;
    }
    end = clock();

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("单核性能测试时间: %f 秒\n", cpu_time_used);
}

void run_multi_core_test() {
    int num_cores;

    #ifdef _WIN32
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        num_cores = sysinfo.dwNumberOfProcessors;  // 获取逻辑CPU核数
    #else
        num_cores = sysconf(_SC_NPROCESSORS_ONLN);  // POSIX系统获取逻辑CPU核数
    #endif

    pthread_t threads[num_cores];
    clock_t start, end;
    double cpu_time_used;
    
    start = clock();
    for (int i = 0; i < num_cores; i++) {
        pthread_create(&threads[i], NULL, multithreaded_task, &num_cores);
    }
    for (int i = 0; i < num_cores; i++) {
        pthread_join(threads[i], NULL);
    }
    end = clock();

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("多核性能测试时间: %f 秒 (线程数: %d)\n", cpu_time_used, num_cores);
}

void run_floating_point_test() {
    clock_t start, end;
    double cpu_time_used;
    double result = 1.0;

    start = clock();
    for (long i = 0; i < ITERATIONS; i++) {
        result *= 1.00000001;
        if (result > 1e10) result = 1.0;
    }
    end = clock();

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("浮点运算性能测试时间: %f 秒\n", cpu_time_used);
}

void run_memory_test() {
    clock_t start, end;
    double cpu_time_used;
    const size_t size = 1024 * 1024 * 100;
    char *buffer = (char*) malloc(size);

    start = clock();
    for (size_t i = 0; i < size; i++) {
        buffer[i] = (char)(i % 256);
    }
    end = clock();
    free(buffer);

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("内存性能测试时间: %f 秒\n", cpu_time_used);
}

void run_performance_test() {
    printf("开始单核性能测试...\n");
    run_single_core_test();
    
    printf("开始多核性能测试...\n");
    run_multi_core_test();

    printf("开始浮点运算性能测试...\n");
    run_floating_point_test();

    printf("开始内存性能测试...\n");
    run_memory_test();
}
