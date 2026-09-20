// Windows 终端聊天室客户端 —— pthread 版本
//
// 与 client_win.c 功能完全一致, 唯一区别是线程部分用 pthread 代替 _beginthreadex。
//
// 注意: 这个文件只能在 MinGW(posix 线程模型)下编译, MSVC 没有 pthread.h。
//       如果目标包括 MSVC, 请用 client_win.c。
//
// 编译: gcc client_win_pthread.c -o client_win_pthread.exe -lws2_32 -pthread

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>      // pthread_create / pthread_detach 代替 process.h
#include <winsock2.h>     // socket/connect/send/recv
#include <ws2tcpip.h>     // inet_pton

#pragma comment(lib, "ws2_32.lib")   // 仅 MSVC 有效; MinGW 需手动加 -lws2_32

void *receive(void *arg);

int main(){
    // 1. 初始化 Winsock(Windows 上使用 socket 前必须做这一步)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){
        fprintf(stderr, "WSAStartup failed\n");
        return -1;
    }

    // 2. 创建通信套接字
    SOCKET connect_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect_fd == INVALID_SOCKET){
        fprintf(stderr, "socket failed: %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    // 3. 连接服务器
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9999);
    // 服务器 IP, 改成你服务端实际所在的地址
    inet_pton(AF_INET, "192.168.80.128", &server_addr.sin_addr.s_addr);

    if (connect(connect_fd, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) == SOCKET_ERROR){
        fprintf(stderr, "connect failed: %d\n", WSAGetLastError());
        closesocket(connect_fd);
        WSACleanup();
        return -1;
    }

    // 4. 创建接收线程
    // SOCKET 是指针大小, 必须经 uintptr_t 转换再塞进 void*, 否则 64 位下会被截断
    pthread_t thread;
    int ret = pthread_create(&thread, NULL, receive,
                             (void *)(uintptr_t)connect_fd);
    if (ret != 0){
        // pthread 系列函数直接把错误码当返回值, 不用查 errno
        fprintf(stderr, "pthread_create: %s\n", strerror(ret));
        closesocket(connect_fd);
        WSACleanup();
        return -1;
    }
    pthread_detach(thread);   // 分离线程, 结束时自动回收

    // 5. 主线程: 读终端输入并发送
    while (1){
        char buf[1024];
        if (fgets(buf, sizeof(buf), stdin) == NULL){
            printf("input error...\n");
            break;
        }
        send(connect_fd, buf, (int)strlen(buf), 0);
    }

    closesocket(connect_fd);
    WSACleanup();
    return 0;
}

void *receive(void *arg){
    SOCKET connect_fd = (SOCKET)(uintptr_t)arg;

    while (1){
        char msg[1024];
        int len = recv(connect_fd, msg, sizeof(msg), 0);

        if (len > 0){
            msg[len] = '\0';
            printf("%s", msg);
        }
        else if (len == 0){
            printf("服务器已经断开...\n");
            _exit(0);   // 主线程正阻塞在 fgets, 直接结束整个进程
        }
        else{
            fprintf(stderr, "recv failed: %d\n", WSAGetLastError());
            _exit(1);
        }
    }

    return NULL;
}
