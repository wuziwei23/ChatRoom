#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 32

typedef struct ClientSockInfo{
    //客户端的fd
    int fd;
    //客户端的addr信息
    struct sockaddr_in addr;
} ClientSockInfo;

ClientSockInfo clients[MAX_CLIENTS];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *commu(void *arg);

int main(){
    for (int i = 0; i < MAX_CLIENTS; i++){
        clients[i].fd = -1;
    }
    // 创建监听的套接字
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd == -1){
        perror("socket");
        return -1;
    }

#if 0
    struct sockaddr_in{
        sa_family_t sin_family;		/* 地址族协议: AF_INET */
        in_port_t sin_port;         /* 端口, 2字节-> 大端  */
        struct in_addr sin_addr;    /* IP地址, 4字节 -> 大端  */
        /* 填充 8字节 */
        unsigned char sin_zero[sizeof (struct sockaddr) - sizeof(sin_family) -
            sizeof (in_port_t) - sizeof (struct in_addr)];
    };  
#endif
    //绑定本地IP和端口
    //需要用到 sockaddr_in 类型的结构体
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(9999);
    saddr.sin_addr.s_addr = inet_addr("0.0.0.0");

    //绑定, 需要监听的套接字和sockaddr_in 类型的结构体地址
    int _bind = bind(listen_fd, (struct sockaddr *)&saddr, sizeof(saddr));
    if(_bind == -1){
        perror("bind");
        return -1;
    }

    //监听
    int _listen = listen(listen_fd, 128);
    if(_listen == -1){
        perror("listen");
        return -1;
    }

    //连接客户端
    while(1){
        struct sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);

        int client_fd = accept(listen_fd,
            (struct sockaddr *)&addr, &addr_len);
        if(client_fd == -1){
            perror("accept");
            break;
        }

        ClientSockInfo *ClientInfo = NULL;

        pthread_mutex_lock(&mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd == -1) {
                ClientInfo = &clients[i];
                break;
            }
        }
        
        if(ClientInfo == NULL){
            close(client_fd);
            //注意死锁
            pthread_mutex_unlock(&mutex);
            continue;
        }
        ClientInfo->fd = client_fd;
        ClientInfo->addr = addr;
        pthread_mutex_unlock(&mutex);


        // 创建子线程来处理通信
        pthread_t thread;
        int ret = pthread_create(&thread, NULL, commu, ClientInfo);

        if(ret != 0){
            fprintf(stderr, "pthread_create: %s\n", strerror(ret));

            pthread_mutex_lock(&mutex);
            close(ClientInfo->fd);
            ClientInfo->fd = -1;
            pthread_mutex_unlock(&mutex);

            continue;
        }

        pthread_detach(thread);
    }

    close(listen_fd);

    return 0;
}

void* commu(void* arg){
    ClientSockInfo *ClientInfo = (ClientSockInfo *)arg;
    char ip[32];
    printf("连接到客户端, IP: %s, 端口: %d\n",
        inet_ntop(AF_INET, &ClientInfo->addr.sin_addr.s_addr, ip, sizeof(ip)),
        ntohs(ClientInfo->addr.sin_port));
    while(1){
        char buf[1024];
        int len = recv(ClientInfo->fd, buf, sizeof(buf), 0);
        if(len > 0){
            //暂时不实现广播操作
            fwrite(buf, 1, len, stdout);
            fflush(stdout);
        }
        else if(len == 0){
            printf("%s 断开了连接...\n", ip);
            break;
        }
        else{
            perror("recv");
            break;
        }
    }

    pthread_mutex_lock(&mutex);
    close(ClientInfo->fd);
    ClientInfo->fd = -1;
    pthread_mutex_unlock(&mutex);

    return NULL;
}