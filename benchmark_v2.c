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

int get_cpu_threads() {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
}
#define THREAD_COUNT (get_cpu_threads())   // 使用动态获取多线程数

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
    #define THREAD_COUNT (sysconf(_SC_NPROCESSORS_ONLN)) // 多线程数
}

#else
void print_processor_info() {
    printf("不支持的操作系统。\n");
}
#endif

#define BASELINE_SCORE 1000   // 目标多核基准分数
#define MAX_SCORE 999999999999        // 每项测试最高分

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


// 主函数
int main() {

    #if defined(_WIN32) || defined(_WIN64)
        printf("This program is compiled for Windows.\n");
        get_cpu_threads();
    #elif defined(__linux__)
        printf("This program is compiled for Linux.\n");
    #else
        printf("Unsupported operating system.\n");
    #endif

    #if defined(_WIN64) || defined(_WIN32)
        set_utf8_encoding_if_powershell();
    #endif

    print_processor_info();

    printf("开始整数运算测试...\n");
    double integer_score = fmin(integer_operations(), MAX_SCORE);
    printf("整数运算分数: %.2f\n", integer_score);

    printf("开始浮点运算测试...\n");
    double floating_point_score = fmin(floating_point_operations(), MAX_SCORE);
    printf("浮点运算分数: %.2f\n", floating_point_score);

    printf("开始位操作测试...\n");
    double bitwise_score = fmin(bitwise_operations(), MAX_SCORE);
    printf("位操作分数: %.2f\n", bitwise_score);

    printf("开始分支预测测试...\n");
    double branch_score = fmin(branch_prediction_test(), MAX_SCORE);
    printf("分支预测分数: %.2f\n", branch_score);

    printf("开始多线程测试...\n");
    double multi_thread_score = fmin(multi_thread_test(), MAX_SCORE);
    printf("多线程分数: %.2f\n", multi_thread_score);

    // 综合得分计算
    double total_score = integer_score + floating_point_score + bitwise_score +
                         branch_score + multi_thread_score;

    printf("综合得分: %.2f\n", fmin(total_score, MAX_SCORE * 5));

    #if defined(_WIN32) || defined(_WIN64)
    system("pause");  // 在程序结束前暂停，等待用户按键
    #else
    printf("按 Enter 键继续...\n");
    getchar();  // Linux 下等待用户按下 Enter 键
    #endif

    return 0;
}
