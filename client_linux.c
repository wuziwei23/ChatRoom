#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>

void *receive(void *arg);

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
              &server_addr.sin_addr.s_addr);

    int ret = connect(connect_fd, (struct sockaddr *)&server_addr,
                      sizeof(server_addr));
    if(ret == -1){
        perror("connect");
        return -1;
    }

    pthread_t thread;
    pthread_create(&thread, NULL, receive, connect_fd);
    pthread_detach(thread);

    while(1){
        char buf[1024];
        if(fgets(buf, sizeof(buf), stdin) == NULL){
            printf("input error...\n");
            break;
        }
        send(connect_fd, buf, strlen(buf), 0);

        // memset(buf, 0, sizeof(buf));
        // int len = recv(connect_fd, buf, sizeof(buf), 0);
        // if(len > 0){
        //     buf[len] = '\0';
        //     printf("%s\n", buf);
        // }
        // else if(len == 0){
        //     printf("服务器已经断开\n");
        //     break;
        // }
        // else{
        //     perror("recv");
        //     break;
        // }
    }


    close(connect_fd);

    return 0;
}

void *receive(void* arg){
    int connect_fd = (int)arg;

    while(1){
        char msg[1024];
        int len = recv(connect_fd, msg, sizeof(msg), 0);

        if(len > 0){
            msg[len] = '\0';
            printf("%s", msg);
        }
        else if(len == 0){
            printf("服务器已经断开\n");
            break;
        }
        else{
            perror("recv");
            break;
        }
    }

    return NULL;
}