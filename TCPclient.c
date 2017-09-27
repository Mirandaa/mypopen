#include<stdlib.h>
#include<stdio.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define SIZE 2048

void print_usage(char * cmd) {
    fprintf(stderr,"Usage: %s ip address\n",cmd);
}


int main(int argc,char** argv) {
    struct sockaddr_in server;
    int ret;
    int len;
    int port;
    int sockfd;
    
    int rvalue;
    char buffer[SIZE];
    char cmd[10]; // 用于处理cd命令
    
    if ((2>argc) || (argc>3)) {
        print_usage(argv[0]);
        exit(1);
    }
    
    if (3==argc) {
        //  这里的端口号从主函数参数获取
        port = atoi(argv[2]);
    }
    
    if (-1==(sockfd=socket(AF_INET,SOCK_STREAM,0))) {
        perror("can not create socket\n");
        exit(1);
    }
    
    memset(&server,0,sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(port);
    
    if (0>(ret=connect(sockfd,(struct sockaddr*)&server,sizeof(struct sockaddr)))) {
        perror("connect error");
        close(sockfd);
        exit(1);
    }
    
    while (1) {
        // 这边只需一个循环
        memset(buffer,0,2048);
        fprintf(stderr, "TCP>");
        fgets(buffer,SIZE,stdin);
        
        rvalue = send(sockfd,buffer,strlen(buffer),0);
        
        if (0==strcmp(buffer,"quit\n") || (0==strcmp(buffer,"QUIT\n"))) {
            close(sockfd);
            
            break; // 结束对话
        }
        
        if (0>rvalue) {
            fprintf(stderr,"error in receiving data\n");
            
            continue;
        }
        
        sscanf(buffer,"%s",cmd);
        
        if ((0==strcmp("cd",cmd)) || (0==strcmp("CD",cmd)))
            // 如果是cd命令则没有返回内容直接重新开始循环
            continue;
        
        len = sizeof(struct sockaddr);
        
        if (0>(len=recv(sockfd,buffer,2048,0))) {
            perror("recv data error\n");
            close(sockfd);
            
            exit(1);
        }
        
        buffer[len] = '\0';
        
        printf("The message from the server is: %s\n",buffer);
    }
    
    return 0;
    
}




