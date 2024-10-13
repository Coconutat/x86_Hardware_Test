#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <windows.h>

#define SCALE_FACTOR 1e6 // 缩小结果的因子

// 整数运算测试
double integer_test() {
    int sum = 0;
    for (int i = 0; i < 100000000; ++i) { // 增加运算次数
        sum += i % 10;
    }
    return sum / SCALE_FACTOR;
}

// 浮点运算测试
double float_test() {
    double sum = 0.0;
    for (int i = 0; i < 10000000; ++i) { // 增加运算次数
        sum += sqrt(i) * 0.5;
    }
    return sum / SCALE_FACTOR;
}

// 位操作测试
double bitwise_test() {
    int sum = 0;
    for (int i = 0; i < 100000000; ++i) { // 增加运算次数
        sum ^= i;
    }
    return sum / SCALE_FACTOR;
}

// 分支预测测试
double branch_test() {
    int sum = 0;
    for (int i = 0; i < 10000000; ++i) { // 增加运算次数
        if (i % 2 == 0) {
            sum += i;
        } else {
            sum -= i;
        }
    }
    return sum / SCALE_FACTOR;
}

// 多线程测试（单线程模拟）
double multithread_test() {
    int sum = 0;
    for (int i = 0; i < 100000000; ++i) { // 增加运算次数
        sum += i % 3;
    }
    return sum / SCALE_FACTOR;
}

// 内存带宽测试
double memory_bandwidth_test() {
    int arr[1000000];
    int sum = 0;
    for (int i = 0; i < 1000000; ++i) {
        arr[i] = i;
    }
    for (int i = 0; i < 1000000; ++i) {
        sum += arr[i];
    }
    return sum / SCALE_FACTOR;
}

// 缓存测试
double cache_test() {
    int arr[1000000];
    int sum = 0;
    for (int i = 0; i < 1000000; ++i) {
        arr[i] = i;
    }
    for (int i = 0; i < 1000000; ++i) {
        sum += arr[i];
    }
    return sum / SCALE_FACTOR;
}

// 内存延迟测试
double memory_latency_test() {
    int sum = 0;
    for (int i = 0; i < 1000000; ++i) {
        sum += i % 5;
    }
    return sum / SCALE_FACTOR;
}

// 内存分配性能测试
double memory_allocation_test() {
    int *arr = (int*)malloc(100000 * sizeof(int));
    if (!arr) return 0;
    int sum = 0;
    for (int i = 0; i < 100000; ++i) {
        arr[i] = i;
        sum += arr[i];
    }
    free(arr);
    return sum / SCALE_FACTOR;
}

int main() {
    // 设置控制台为UTF-8编码
    SetConsoleOutputCP(CP_UTF8);

    printf("开始整数运算测试...\n");
    double integer_score = integer_test();
    printf("整数运算分数: %.2f\n", integer_score);

    printf("开始浮点运算测试...\n");
    double float_score = float_test();
    printf("浮点运算分数: %.2f\n", float_score);

    printf("开始位操作测试...\n");
    double bitwise_score = bitwise_test();
    printf("位操作分数: %.2f\n", bitwise_score);

    printf("开始分支预测测试...\n");
    double branch_score = branch_test();
    printf("分支预测分数: %.2f\n", branch_score);

    printf("开始多线程测试...\n");
    double multithread_score = multithread_test();
    printf("多线程分数: %.2f\n", multithread_score);

    printf("开始内存带宽测试...\n");
    double memory_bandwidth_score = memory_bandwidth_test();
    printf("内存带宽分数: %.2f\n", memory_bandwidth_score);

    printf("开始缓存测试...\n");
    double cache_score = cache_test();
    printf("缓存分数: %.2f\n", cache_score);

    printf("开始内存延迟测试...\n");
    double memory_latency_score = memory_latency_test();
    printf("内存延迟分数: %.2f\n", memory_latency_score);

    printf("开始内存分配性能测试...\n");
    double memory_allocation_score = memory_allocation_test();
    printf("内存分配性能分数: %.2f\n", memory_allocation_score);

    double total_score = integer_score + float_score + bitwise_score + branch_score + 
                         multithread_score + memory_bandwidth_score + cache_score + 
                         memory_latency_score + memory_allocation_score;

    printf("综合得分: %.2f\n", total_score);

    return 0;
}
