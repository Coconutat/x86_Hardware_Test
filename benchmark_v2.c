#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <string.h> // 包含 memset 函数

#if defined(_WIN32) || defined(_WIN64) // Windows 系统
#include <windows.h>
#include <tlhelp32.h>

void set_utf8_encoding_if_powershell() {
    SetConsoleOutputCP(CP_UTF8); // 设置控制台编码为 UTF-8
}

void print_processor_info() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    int logical_cores = sysInfo.dwNumberOfProcessors;

    // 使用 GetLogicalProcessorInformationEx 获取物理核心数
    DWORD len = 0;
    GetLogicalProcessorInformationEx(RelationProcessorCore, NULL, &len);
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)malloc(len);
    GetLogicalProcessorInformationEx(RelationProcessorCore, buffer, &len);

    int physical_cores = 0;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX ptr = buffer;
    while ((BYTE *)ptr < (BYTE *)buffer + len) {
        if (ptr->Relationship == RelationProcessorCore) {
            physical_cores++;
        }
        ptr = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)((BYTE *)ptr + ptr->Size);
    }
    free(buffer);

    // 获取处理器名称
    char processorName[256] = "Unknown Processor";
    DWORD bufferSize = sizeof(processorName);
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL, (LPBYTE)processorName, &bufferSize);
        RegCloseKey(hKey);
    }

    printf("处理器名称: %s\n", processorName);
    printf("物理核心数: %d\n", physical_cores);
    printf("逻辑核心数: %d\n", logical_cores);
}

int get_cpu_threads() {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
}
#define THREAD_COUNT (get_cpu_threads())    // 使用动态获取多线程数

#elif defined(__linux__)    // Linux 系统
#include <unistd.h>
#include <string.h>

void print_processor_info() {
    int logical_cores = sysconf(_SC_NPROCESSORS_ONLN);

    int physical_cores = 0;
    FILE *cpuInfoFile = fopen("/proc/cpuinfo", "r");
    char line;
    while (fgets(line, sizeof(line), cpuInfoFile)) {
        if (strncmp(line, "core id", 7) == 0) {
            physical_cores++;
        }
    }
    fclose(cpuInfoFile);

    char processorName = "Unknown Processor";
    cpuInfoFile = fopen("/proc/cpuinfo", "r");
    if (cpuInfoFile != NULL) {
        while (fgets(line, sizeof(line), cpuInfoFile)) {
            if (strncmp(line, "model name", 10) == 0) {
                char *name = strchr(line, ':') + 2;
                strncpy(processorName, name, sizeof(processorName) - 1);
                processorName[strcspn(processorName, "\n")] = '\0';
                break;
            }
        }
        fclose(cpuInfoFile);
    }

    printf("处理器名称: %s\n", processorName);
    printf("物理核心数: %d\n", physical_cores > 0 ? physical_cores : logical_cores / 2);
    printf("逻辑核心数: %d\n", logical_cores);
}
#define THREAD_COUNT (sysconf(_SC_NPROCESSORS_ONLN))

#else
void print_processor_info() {
    printf("不支持的操作系统。\n");
}
#endif

#define BASELINE_SCORE 1000 // 目标多核基准分数
#define MAX_SCORE 999999999999  // 每项测试最高分

// 测试参数
#define INTEGER_OPS_COUNT 1000000000      // 整数运算循环次数
#define FLOATING_POINT_OPS_COUNT 500000000 // 浮点运算循环次数
#define BITWISE_OPS_COUNT 500000000       // 位操作次数
#define BRANCH_TEST_COUNT 500000000       // 分支预测测试次数// 多线程数
#define MEMORY_BANDWIDTH_SIZE 500000000   // 内存带宽测试数据大小 (字节)
#define CACHE_TEST_SIZE 100000000         // 缓存测试数据大小 (字节)
#define MEMORY_LATENCY_TEST_COUNT 1000000000 // 内存延迟测试次数

void print_progress(double progress) {
    printf("\r进度: %.2f%%", progress * 100);
    fflush(stdout);
}

double measure_time(double (*test_func)()) {
    clock_t start = clock();
    double score = test_func();
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    return BASELINE_SCORE / time_spent;
}

double integer_operations() {
    volatile int a = 1;
    for (int i = 0; i < INTEGER_OPS_COUNT; i++) {
        a += (a * 3 + i) % 7;
        if (i % (INTEGER_OPS_COUNT / 100) == 0) print_progress((double)i / INTEGER_OPS_COUNT);
    }
    printf("\n");
    return (double)INTEGER_OPS_COUNT / BASELINE_SCORE;
}

double floating_point_operations() {
    volatile double a = 1.0;
    for (int i = 0; i < FLOATING_POINT_OPS_COUNT; i++) {
        a += sin(a) * cos(a) + tan(a);
        if (i % (FLOATING_POINT_OPS_COUNT / 100) == 0) print_progress((double)i / FLOATING_POINT_OPS_COUNT);
    }
    printf("\n");
    return (double)FLOATING_POINT_OPS_COUNT / BASELINE_SCORE;
}

double bitwise_operations() {
    volatile int a = 0x55555555;
    for (int i = 0; i < BITWISE_OPS_COUNT; i++) {
        a ^= (a << 1) | (a >> 1);
        if (i % (BITWISE_OPS_COUNT / 100) == 0) print_progress((double)i / BITWISE_OPS_COUNT);
    }
    printf("\n");
    return (double)BITWISE_OPS_COUNT / BASELINE_SCORE;
}

double branch_prediction_test() {
    volatile int a = 0;
    for (int i = 0; i < BRANCH_TEST_COUNT; i++) {
        if (i % 2 == 0) a++;
        else a--;
        if (i % (BRANCH_TEST_COUNT / 100) == 0) print_progress((double)i / BRANCH_TEST_COUNT);
    }
    printf("\n");
    return (double)BRANCH_TEST_COUNT / BASELINE_SCORE;
}

void *thread_task(void *arg) {
    volatile int a = 1;
    for (int i = 0; i < INTEGER_OPS_COUNT / THREAD_COUNT; i++) {
        a += (a * 3 + i) % 7;
    }
    return NULL;
}

double multi_thread_test() {
    pthread_t threads[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, thread_task, NULL);
        print_progress((double)i / THREAD_COUNT);
    }
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("\n");
    return (double)INTEGER_OPS_COUNT / BASELINE_SCORE;
}

double memory_bandwidth_test() {
    volatile char *buffer = (char *)malloc(MEMORY_BANDWIDTH_SIZE);
    memset((void *)buffer, 0, MEMORY_BANDWIDTH_SIZE);
    for (int i = 0; i < MEMORY_BANDWIDTH_SIZE; i++) {
        buffer[i] = (char)(i % 256);
        if (i % (MEMORY_BANDWIDTH_SIZE / 100) == 0) print_progress((double)i / MEMORY_BANDWIDTH_SIZE);
    }
    free((void *)buffer);
    printf("\n");
    return (double)MEMORY_BANDWIDTH_SIZE / BASELINE_SCORE;
}

double cache_test() {
    volatile char *buffer = (char *)malloc(CACHE_TEST_SIZE);
    memset((void *)buffer, 0, CACHE_TEST_SIZE);
    for (int i = 0; i < CACHE_TEST_SIZE; i++) {
        buffer[i] = (char)(i % 256);
        if (i % (CACHE_TEST_SIZE / 100) == 0) print_progress((double)i / CACHE_TEST_SIZE);
    }
    free((void *)buffer);
    printf("\n");
    return (double)CACHE_TEST_SIZE / BASELINE_SCORE;
}

double memory_latency_test() {
    volatile int a = 0;
    for (int i = 0; i < MEMORY_LATENCY_TEST_COUNT; i++) {
        a += (a * 3 + i) % 7;
        if (i % (MEMORY_LATENCY_TEST_COUNT / 100) == 0) print_progress((double)i / MEMORY_LATENCY_TEST_COUNT);
    }
    printf("\n");
    return (double)MEMORY_LATENCY_TEST_COUNT / BASELINE_SCORE;
}

double encryption_test() {
    unsigned char key[16] = {0};     // 定义16字节的key数组
    unsigned char data[16] = {0};    // 定义16字节的data数组
    for (int i = 0; i < 100000000; i++) {
        for (int j = 0; j < 16; j++) {
            data[j] ^= key[j];       // 对应字节逐一异或
        }
        if (i % (100000000 / 100) == 0) 
            print_progress((double)i / 100000000); // 打印进度
    }
    printf("\n");
    return 100000000.0 / BASELINE_SCORE;
}

double compression_test() {
    volatile char *data = (char *)malloc(100000000);
    memset((void *)data, 'A', 100000000);
    for (int i = 0; i < 100000000; i++) {
        data[i] = (char)((data[i] + i) % 256);
        if (i % (100000000 / 100) == 0) print_progress((double)i / 100000000);
    }
    free((void *)data);
    printf("\n");
    return 100000000.0 / BASELINE_SCORE;
}

int main() {
    #if defined(_WIN32) || defined(_WIN64)
        printf("This program is compiled for Windows.\n");
        set_utf8_encoding_if_powershell();
    #elif defined(__linux__)
        printf("This program is compiled for Linux.\n");
    #else
        printf("Unsupported operating system.\n");
    #endif

    print_processor_info();

    printf("开始整数运算测试...\n");
    double integer_score = measure_time(integer_operations);
    printf("整数运算分数: %.2f\n", integer_score);

    printf("开始浮点运算测试...\n");
    double floating_point_score = measure_time(floating_point_operations);
    printf("浮点运算分数: %.2f\n", floating_point_score);

    printf("开始位操作测试...\n");
    double bitwise_score = measure_time(bitwise_operations);
    printf("位操作分数: %.2f\n", bitwise_score);

    printf("开始分支预测测试...\n");
    double branch_score = measure_time(branch_prediction_test);
    printf("分支预测分数: %.2f\n", branch_score);

    printf("开始多线程测试...\n");
    double multi_thread_score = measure_time(multi_thread_test);
    printf("多线程分数: %.2f\n", multi_thread_score);

    printf("开始内存带宽测试...\n");
    double memory_bandwidth_score = measure_time(memory_bandwidth_test);
    printf("内存带宽分数: %.2f\n", memory_bandwidth_score);

    printf("开始缓存测试...\n");
    double cache_score = measure_time(cache_test);
    printf("缓存分数: %.2f\n", cache_score);

    printf("开始内存延迟测试...\n");
    double memory_latency_score = measure_time(memory_latency_test);
    printf("内存延迟分数: %.2f\n", memory_latency_score);

    printf("开始加密运算测试...\n");
    double encryption_score = measure_time(encryption_test);
    printf("加密运算分数: %.2f\n", encryption_score);

    printf("开始压缩/解压缩测试...\n");
    double compression_score = measure_time(compression_test);
    printf("压缩/解压缩分数: %.2f\n", compression_score);

    double total_score = integer_score + floating_point_score + bitwise_score + branch_score + multi_thread_score +
                         memory_bandwidth_score + cache_score + memory_latency_score + encryption_score + compression_score;
    printf("综合得分: %.2f\n", total_score);

    #if defined(_WIN32) || defined(_WIN64)
    system("pause"); // 在程序结束前暂停，等待用户按键
    #else
        printf("按 Enter 键继续...\n");  // Linux 下等待用户按下 Enter 键
    getchar();
    #endif

    return 0;
}
