#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <intrin.h>
#else
#include <unistd.h>
#include <sys/sysinfo.h>
#include <stdlib.h>
#endif

#ifdef _WIN32
void get_cpu_name(char* cpuBrandString) {
    int cpuInfo[4] = {0};

    // 获取CPU品牌字符串的各个部分
    __cpuid(cpuInfo, 0x80000002);
    memcpy(cpuBrandString, cpuInfo, sizeof(cpuInfo));

    __cpuid(cpuInfo, 0x80000003);
    memcpy(cpuBrandString + 16, cpuInfo, sizeof(cpuInfo));

    __cpuid(cpuInfo, 0x80000004);
    memcpy(cpuBrandString + 32, cpuInfo, sizeof(cpuInfo));

    cpuBrandString[48] = '\0'; // 确保字符串以null结尾
}
#endif

void display_cpu_info() {
#ifdef _WIN32
    // 获取CPU名称
    char cpuBrandString[0x40];
    get_cpu_name(cpuBrandString);
    printf("CPU 名称: %s\n", cpuBrandString);

    // 获取核心数和线程数
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    printf("CPU 核心数: %u\n", sysinfo.dwNumberOfProcessors);
#else
    // 获取CPU名称
    FILE* fp;
    char buffer[1024];  // 使用适当大小的缓冲区
    size_t bytes_read;
    char* match;

    fp = fopen("/proc/cpuinfo", "r");
    if (fp == NULL) {
        perror("fopen");
        return;
    }
    bytes_read = fread(buffer, 1, sizeof(buffer) - 1, fp);  // -1 为了保留 null 结尾
    fclose(fp);
    if (bytes_read == 0) {
        printf("读取 /proc/cpuinfo 失败\n");
        return;
    }
    buffer[bytes_read] = '\0';  // 添加 null 结尾

    match = strstr(buffer, "model name");
    if (match != NULL) {
        char* colon = strchr(match, ':');
        if (colon != NULL) {
            colon++;
            while (*colon == ' ') colon++;  // 跳过空格
            printf("CPU 名称: %s\n", colon);
        }
    } else {
        printf("无法找到 CPU 名称\n");
    }

    // 获取核心数和线程数
    printf("CPU 核心数: %d\n", get_nprocs());
#endif
}