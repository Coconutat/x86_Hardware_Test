#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <intrin.h>
#else
#include <unistd.h>
#include <sys/sysinfo.h>
#endif

#define ITERATIONS 500000000  // 每个测试的迭代次数
#define BASELINE_CPU_SCORE 1000.0  // 基准CPU的分数

typedef struct {
    int num_cores;
    double single_core_score;
    double multi_core_score;
    double floating_point_score;
    double memory_score;
} ScoreCard;

void *multithreaded_task(void *arg) {
    long sum = 0;
    for (long i = 0; i < ITERATIONS / *((int *)arg); i++) {
        sum += i;
    }
    return NULL;
}

double calculate_score(double baseline, double time_taken) {
    return (baseline / time_taken) * BASELINE_CPU_SCORE;
}

double run_single_core_test() {
    clock_t start, end;
    double time_taken;
    
    start = clock();
    long sum = 0;
    for (long i = 0; i < ITERATIONS; i++) {
        sum += i;
    }
    end = clock();

    time_taken = ((double) (end - start)) / CLOCKS_PER_SEC;
    return calculate_score(1.0, time_taken);  // 假设1.0秒为基准
}

double run_multi_core_test(int num_cores) {
    pthread_t threads[num_cores];
    clock_t start, end;
    double time_taken;
    
    start = clock();
    for (int i = 0; i < num_cores; i++) {
        pthread_create(&threads[i], NULL, multithreaded_task, &num_cores);
    }
    for (int i = 0; i < num_cores; i++) {
        pthread_join(threads[i], NULL);
    }
    end = clock();

    time_taken = ((double) (end - start)) / CLOCKS_PER_SEC;
    return calculate_score(0.5 * num_cores, time_taken);  // 假设0.5秒为基准
}

double run_floating_point_test() {
    clock_t start, end;
    double result = 1.0, time_taken;

    start = clock();
    for (long i = 0; i < ITERATIONS; i++) {
        result *= 1.00000001;
        if (result > 1e10) result = 1.0;
    }
    end = clock();

    time_taken = ((double) (end - start)) / CLOCKS_PER_SEC;
    return calculate_score(1.2, time_taken);  // 假设1.2秒为基准
}

double run_memory_test() {
    clock_t start, end;
    double time_taken;
    const size_t size = 1024 * 1024 * 50;
    char *buffer = (char*) malloc(size);

    start = clock();
    for (size_t i = 0; i < size; i++) {
        buffer[i] = (char)(i % 256);
    }
    end = clock();
    free(buffer);

    time_taken = ((double) (end - start)) / CLOCKS_PER_SEC;
    return calculate_score(1.5, time_taken);  // 假设1.5秒为基准
}

int get_core_count() {
#ifdef _WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#else
    return get_nprocs();  // Linux下使用get_nprocs获取核心数
#endif
}

void display_scores(ScoreCard *scores) {
    printf("\n===== CPU 测试结果 =====\n");
    printf("单核得分: %.2f\n", scores->single_core_score);
    printf("多核得分: %.2f\n", scores->multi_core_score);
    printf("浮点运算得分: %.2f\n", scores->floating_point_score);
    printf("内存带宽得分: %.2f\n", scores->memory_score);
    printf("\n总得分: %.2f\n", scores->single_core_score + scores->multi_core_score +
                                     scores->floating_point_score + scores->memory_score);
}

void run_performance_test() {
    ScoreCard scores;
    scores.num_cores = get_core_count();

    printf("正在运行单核测试...\n");
    scores.single_core_score = run_single_core_test();

    printf("正在运行多核测试 (核心数: %d)...\n", scores.num_cores);
    scores.multi_core_score = run_multi_core_test(scores.num_cores);

    printf("正在运行浮点运算测试...\n");
    scores.floating_point_score = run_floating_point_test();

    printf("正在运行内存测试...\n");
    scores.memory_score = run_memory_test();

    display_scores(&scores);
}