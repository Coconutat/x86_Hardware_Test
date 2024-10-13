#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h> // 包含 memset 函数

#if defined(_WIN32) || defined(_WIN64) // Windows 系统
#include <windows.h>

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
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL,
                         (LPBYTE)processorName, &bufferSize);
        RegCloseKey(hKey);
    }

    printf("处理器名称: %s\n", processorName);
    printf("物理核心数: %d\n", physical_cores);
    printf("逻辑核心数: %d\n", logical_cores);
}

#elif defined(__linux__) // Linux 系统
#include <unistd.h>
#include <string.h>

void print_processor_info() {
    // 获取逻辑核心数
    int logical_cores = sysconf(_SC_NPROCESSORS_ONLN);

    // 获取物理核心数
    int physical_cores = 0;
    FILE *cpuInfoFile = fopen("/proc/cpuinfo", "r");
    char line[256];
    while (fgets(line, sizeof(line), cpuInfoFile)) {
        if (strncmp(line, "core id", 7) == 0) {
            physical_cores++;
        }
    }
    fclose(cpuInfoFile);

    // 读取处理器名称
    char processorName[256] = "Unknown Processor";
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

#else
void print_processor_info() {
    printf("不支持的操作系统。\n");
}
#endif

// 进度显示
#define PROGRESS_STEP 5 // 每个测试的进度步长

void show_progress(const char *test_name, int percent) {
    printf("%s测试进度: %d%%\n", test_name, percent);
}

// 整数运算测试
double integer_operations_test() {
    double sum = 0.0;
    int max_iter = 10000; // 降低迭代次数
    for (int i = 1; i <= max_iter; ++i) {
        sum += i; // 计算1到max_iter的和
        if (i % (max_iter / (100 / PROGRESS_STEP)) == 0) {
            show_progress("整数运算", i * 100 / max_iter);
        }
    }
    double score = sum / 100; // 缩放以控制分数范围
    printf("整数运算分数: %.2f\n", score); // 输出每个项目的分数
    return score;
}

// 浮点运算测试
double float_operations_test() {
    double sum = 0.0;
    int max_iter = 10000; // 降低迭代次数
    for (int i = 1; i <= max_iter; ++i) {
        sum += (double)i / 3.14; // 浮点运算
        if (i % (max_iter / (100 / PROGRESS_STEP)) == 0) {
            show_progress("浮点运算", i * 100 / max_iter);
        }
    }
    double score = sum / 100; // 缩放以控制分数范围
    printf("浮点运算分数: %.2f\n", score); // 输出每个项目的分数
    return score;
}

// 位操作测试
double bit_operations_test() {
    double sum = 0.0;
    int max_iter = 10000; // 降低迭代次数
    for (int i = 1; i <= max_iter; ++i) {
        sum += (double)(i ^ (i >> 1)); // 使用异或操作
        if (i % (max_iter / (100 / PROGRESS_STEP)) == 0) {
            show_progress("位操作", i * 100 / max_iter);
        }
    }
    double score = sum / 100; // 确保返回值在合理范围内
    printf("位操作分数: %.2f\n", score); // 输出每个项目的分数
    return score;
}

// 分支预测测试
double branch_test() {
    double sum = 0.0;
    int max_iter = 100000; // 降低迭代次数
    for (int i = 0; i < max_iter; ++i) {
        sum += (i % 2 == 0) ? i : i * 2; // 确保不会有负值
        if (i % (max_iter / (100 / PROGRESS_STEP)) == 0) {
            show_progress("分支预测", i * 100 / max_iter);
        }
    }
    double score = sum / (max_iter / 100); // 缩放以控制分数范围
    printf("分支预测分数: %.2f\n", score); // 输出每个项目的分数
    return score;
}

// 简单线程功能
void *thread_function(void *arg) {
    double *sum = (double *)arg;
    for (int i = 1; i <= 1000; i++) {
        *sum += i; // 各线程累加
    }
    return NULL;
}

// 多线程测试
double multithreading_test() {
    double sum = 0.0;
    int max_threads = 4; // 线程数量
    pthread_t threads[max_threads];
    double *thread_sums = (double *)malloc(max_threads * sizeof(double)); // 动态分配数组
    memset(thread_sums, 0, max_threads * sizeof(double)); // 初始化为0

    // 创建线程并计算
    for (int i = 0; i < max_threads; ++i) {
        pthread_create(&threads[i], NULL, thread_function, (void *)&thread_sums[i]);
    }

    for (int i = 0; i < max_threads; ++i) {
        pthread_join(threads[i], NULL);
        sum += thread_sums[i]; // 汇总每个线程的结果
    }
    
    free(thread_sums); // 释放动态分配的内存
    double score = sum / (max_threads * 100); // 缩放以控制分数范围
    printf("多线程分数: %.2f\n", score); // 输出每个项目的分数
    return score;
}

// 内存带宽测试
double memory_bandwidth_test() {
    double sum = 0.0;
    int max_iter = 10000; // 降低迭代次数
    double *arr = (double *)malloc(max_iter * sizeof(double)); // 使用double类型

    if (!arr) {
        fprintf(stderr, "内存分配失败！\n");
        return 0.0;
    }

    for (int i = 0; i < max_iter; ++i) {
        arr[i] = (double)i; // 初始化数组
        sum += arr[i]; // 简单计算
        if (i % (max_iter / (100 / PROGRESS_STEP)) == 0) {
            show_progress("内存带宽", i * 100 / max_iter);
        }
    }
    free(arr);
    double score = sum / (max_iter / 100); // 缩放以控制分数范围
    printf("内存带宽分数: %.2f\n", score); // 输出每个项目的分数
    return score;
}

// 主函数
int main() {

    #if defined(_WIN32) || defined(_WIN64)
        printf("This program is compiled for Windows.\n");
    #elif defined(__linux__)
        printf("This program is compiled for Linux.\n");
    #elif defined(__APPLE__)
        printf("This program is compiled for macOS.\n");
    #else
        printf("Unsupported operating system.\n");
    #endif

    print_processor_info();

    double total_score = 0.0;

    printf("开始整数运算测试...\n");
    total_score += integer_operations_test();

    printf("开始浮点运算测试...\n");
    total_score += float_operations_test();

    printf("开始位操作测试...\n");
    total_score += bit_operations_test();

    printf("开始分支预测测试...\n");
    total_score += branch_test();

    printf("开始多线程测试...\n");
    total_score += multithreading_test();

    printf("开始内存带宽测试...\n");
    total_score += memory_bandwidth_test();

    printf("综合得分: %.2f\n", total_score);
    return 0;
}
