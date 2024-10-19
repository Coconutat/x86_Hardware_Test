#include <stdio.h>
#include "cpu_info.h"
#include "gpu_info.h"
#include "performance_test.h"

#if defined(_WIN32) || defined(_WIN64) // Windows 系统
#include <windows.h>

void set_utf8_encoding_if_powershell() {
    SetConsoleOutputCP(CP_UTF8); // 设置控制台编码为 UTF-8
}
#endif

int main() {
    #if defined(_WIN32) || defined(_WIN64)
        set_utf8_encoding_if_powershell();
    #endif

    // 显示CPU信息
    printf("显示CPU信息...\n");
    display_cpu_info();

    // 显示GPU信息
    printf("显示GPU信息...\n");
    get_gpu_info();

    // 运行性能测试
    printf("运行性能测试...\n");
    run_performance_test();

    #if defined(_WIN32) || defined(_WIN64)
        system("pause"); // 在程序结束前暂停，等待用户按键
    #else
        printf("按 Enter 键继续...\n");  // Linux 下等待用户按下 Enter 键
        getchar();
    #endif

    return 0;
}
