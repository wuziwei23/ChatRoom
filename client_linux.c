#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>

int main(){
    // 创建通信的套接字
    int connect_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(connect_fd == -1){
        perror("socket");
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9999);

    inet_pton(AF_INET, "192.168.80.128",
              &server_addr.sin_family.s_addr);

    int ret = connect(connect_fd, (struct sockaddr *)&server_addr,
                      sizeof(server_addr));
    if(ret == -1){
        perror("connect");
        return -1;
    }
    
    while(1){

    }
}