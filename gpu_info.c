#include "gpu_info.h"
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

void get_gpu_info() {
#if defined(_WIN32) || defined(_WIN64)
    // Windows特定的代码
    printf("GPU Information (Windows):\n");
    // system("wmic path win32_VideoController get name");
    system("powershell -Command \"Get-WmiObject win32_VideoController | Select-Object -ExpandProperty Name\""); // After Windows 10...
#elif defined(__linux__)
    // Linux特定的代码
    FILE *fp;
    char buffer[256];

    printf("GPU Information (Linux):\n");
    fp = popen("lspci | grep -i vga", "r");
    if (fp == NULL) {
        perror("Failed to run command");
        return;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
    }
    pclose(fp);
#else
    printf("Unsupported OS\n");
#endif
}
