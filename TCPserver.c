//
//  TCPserver.c
//  lab7
//
//  Created by mac on 2017/9/27.
//  Copyright © 2017年 Maggie. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define PORT 8900
#define SIZE 2048     /* 2K BUFFER */  

// 建立exec函数来完成原来popen()函数实现的功能
void  exec (char* command,char* true_result) {
    FILE * in;
    char c;
    char result[2048];
    memset(result,0,2048);
    int fd[2];
    int pid;
    
    if ( pipe(fd)<0 )
        printf("create pipe fail\n");
    if ( (pid = fork())<0 )
        printf("fork pipe fail \n");
    else if ( pid == 0 )  {
        close(fd[0]);
        dup2(fd[1],STDOUT_FILENO);
        execlp("sh","sh -c","-c",command,NULL);
    }
    
    close(fd[1]);
    read(fd[0],result,2048);
    strcpy(true_result,result);
}

void work(int connectd) {
    int recvnum;
    int rvalue;
    char cmd[10];                               /* these two var used  to deal with cd command */
    char path[2048];
    char recv_buf[2048];
    char send_buf[2048];
    
    while (1) {
        memset(send_buf,0,2048);
        memset(recv_buf,0,2048);
        
        if ( 0>(recvnum = recv(connectd,recv_buf,sizeof(recv_buf),0)) ) {
            perror("recv error\n");
            close(connectd);
            continue;
        }
        recv_buf[recvnum-1]='\0'; //这里要注意-1，不然会保留换行符
        
        if ( 0==strcmp(recv_buf,"quit")||(0==strcmp(recv_buf,"QUIT")) ) {//结束对话，跳出内部循环，继续外部循环
            printf("over\n");
            break;
        }
        sscanf(recv_buf,"%s %s",cmd,path);
        if ( (0==strcmp("cd",cmd)) ||(0==strcmp("CD",cmd)) ) {
            chdir(path);
            continue;
        }
        
        exec(recv_buf,send_buf);
        rvalue = -1;
        rvalue = send(connectd,send_buf,sizeof(send_buf),0);
    }
}

int main(int argc,char** argv) {
    struct sockaddr_in server;
    struct sockaddr_in client;
    int len;
    int port;
    int listend;
    int connectd;
    int sendnum;
    int opt;
    char addr_p[2048]; //存储客户端地址的缓冲区
    int pid;
    port = PORT;
    opt = SO_REUSEADDR;
    
    if (-1==(listend=socket(AF_INET,SOCK_STREAM,0))) { //使用TCP创建 监听用的套接字
        perror("create listen socket error\n");
        exit(1);
    }
    
    setsockopt(listend,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
#ifdef DEBUG  
    printf("the listen id is %d\n",listend);
#endif  
    memset(&server,0,sizeof(struct sockaddr_in));
    server.sin_family = AF_INET; //IPv4协议
    server.sin_addr.s_addr = htonl(INADDR_ANY); //接收任意地址
    server.sin_port = htons(port); //设置端口号
    if (-1==bind(listend,(struct sockaddr *)&server,sizeof(struct  sockaddr))) { //绑定
        perror("bind error\n");
        exit(1);
    }
    
    if (-1==listen(listend,5)) {
        perror("listen error\n");
        exit(1);
    }
    
    memset(&client,0,sizeof(struct sockaddr_in));
    client.sin_family = AF_INET; //IPv4协议
    while(1) {//外部死循环，用来重复连接客户端
        if (-1==(connectd=accept(listend,(struct  sockaddr*)&client,&len))) {//创建新的套接字连接客户端
            perror("create connect socket error\n");
            continue;
        }
        
        inet_ntop(AF_INET,&client.sin_addr,addr_p,sizeof(addr_p));  //将客户端地址转为字符串
        printf("client IP is %s, port  is %d\n",addr_p,ntohs(client.sin_port));
        pid = fork();
        
        if (pid < 0) {
            printf("fail to fork\n");
            exit(1);
        }
        else if (pid == 0) {
            work(connectd);
            exit(0);
        }
        if (waitpid(pid,NULL,WNOHANG) == -1)
            printf("fail to waitpid\n");
    }
    close(listend);
    return 0;
}
