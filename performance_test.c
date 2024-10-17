#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#define sleep_ms(x) Sleep(x)
#else
#include <unistd.h>
#include <sys/sysinfo.h>
#define sleep_ms(x) usleep((x) * 1000)
#endif

#define ITERATIONS 1000000000
#define BASELINE_SCORE 1000.0
#define PROGRESS_BAR_LENGTH 30

typedef struct {
    int num_cores;
    double integer_score;
    double floating_point_score;
    double parallel_score;
    double memory_score;
} ScoreCard;

typedef struct {
    long start;
    long end;
    long *progress;  // 共享进度变量
    pthread_mutex_t *mutex;  // 进度条更新的互斥锁
} ThreadData;

void print_progress_bar(double progress) {
    int pos = (int)(progress * PROGRESS_BAR_LENGTH);
    printf("[");
    for (int i = 0; i < PROGRESS_BAR_LENGTH; ++i) {
        if (i < pos) printf("=");
        else if (i == pos) printf(">");
        else printf(" ");
    }
    printf("] %.2f%%\r", progress * 100.0);
    fflush(stdout);
}

void* parallel_task(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    long local_progress = 0;

    for (long i = data->start; i < data->end; i++) {
        // 模拟计算任务
        local_progress++;
        if (local_progress % (data->end / 100) == 0) {
            pthread_mutex_lock(data->mutex);
            *data->progress += 1;  // 更新整体进度
            pthread_mutex_unlock(data->mutex);
        }
    }
    return NULL;
}

double calculate_score(double baseline, double time_taken) {
    return (baseline / time_taken) * BASELINE_SCORE;
}

double run_integer_test() {
    long sum = 0;
    clock_t start, end;
    start = clock();

    for (long i = 0; i < ITERATIONS; i++) {
        sum += i;
        if (i % (ITERATIONS / 100) == 0) print_progress_bar((double)i / ITERATIONS);
    }

    end = clock();
    printf("\n");
    return calculate_score(1.0, ((double)(end - start)) / CLOCKS_PER_SEC);
}

double run_floating_point_test() {
    double result = 0.0;
    clock_t start, end;
    start = clock();

    for (long i = 0; i < ITERATIONS; i++) {
        result += sin(i) * cos(i);
        if (i % (ITERATIONS / 100) == 0) print_progress_bar((double)i / ITERATIONS);
    }

    end = clock();
    printf("\n");
    return calculate_score(1.5, ((double)(end - start)) / CLOCKS_PER_SEC);
}

double run_parallel_test(int num_cores) {
    pthread_t threads[num_cores];
    ThreadData thread_data[num_cores];
    long part = ITERATIONS / num_cores;
    long progress = 0;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    for (int i = 0; i < num_cores; i++) {
        thread_data[i].start = i * part;
        thread_data[i].end = (i + 1) * part;
        thread_data[i].progress = &progress;
        thread_data[i].mutex = &mutex;
        pthread_create(&threads[i], NULL, parallel_task, &thread_data[i]);
    }

    while (progress < 100) {  // 主线程定期刷新进度条
        pthread_mutex_lock(&mutex);
        print_progress_bar(progress / 100.0);
        pthread_mutex_unlock(&mutex);
        sleep_ms(50);  // 每50ms刷新一次
    }

    for (int i = 0; i < num_cores; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\n");
    pthread_mutex_destroy(&mutex);
    return BASELINE_SCORE * num_cores;  // 假设基准得分为多核数的倍数
}

double run_memory_test() {
    size_t size = 1024 * 1024 * 200;
    char *buffer = (char *)malloc(size);
    if (buffer == NULL) {
        printf("Memory allocation failed\n");
        return 0.0;
    }
    clock_t start, end;
    start = clock();

    for (size_t i = 0; i < size; i++) {
        buffer[i] = (char)(i % 256);
        if (i % (size / 100) == 0) print_progress_bar((double)i / size);
    }

    end = clock();
    free(buffer);
    printf("\n");
    return calculate_score(2.5, ((double)(end - start)) / CLOCKS_PER_SEC);
}

int get_core_count() {
#ifdef _WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#else
    return get_nprocs();
#endif
}

void display_scores(ScoreCard *scores) {
    printf("\n===== CPU 测试结果 =====\n");
    printf("整数运算得分: %.2f\n", scores->integer_score);
    printf("浮点运算得分: %.2f\n", scores->floating_point_score);
    printf("并行运算得分: %.2f\n", scores->parallel_score);
    printf("内存带宽得分: %.2f\n", scores->memory_score);
    printf("\n总得分: %.2f\n", scores->integer_score + scores->floating_point_score +
                                     scores->parallel_score + scores->memory_score);
}

void run_performance_test() {
    ScoreCard scores;
    scores.num_cores = get_core_count();

    printf("正在运行整数运算测试...\n");
    scores.integer_score = run_integer_test();

    printf("正在运行浮点运算测试...\n");
    scores.floating_point_score = run_floating_point_test();

    printf("正在运行并行运算测试 (核心数: %d)...\n", scores.num_cores);
    scores.parallel_score = run_parallel_test(scores.num_cores);

    printf("正在运行内存测试...\n");
    scores.memory_score = run_memory_test();

    display_scores(&scores);
}
