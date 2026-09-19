#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>

int main(){
    // 创建通信的套接字
    int connect_fd = socket(AF_INET, SOCK_STREAM, 0);
    
}