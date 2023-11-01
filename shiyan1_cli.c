#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>


#define BUF_LEN 1025
struct pdu{
    short header;
    char payload[BUF_LEN];
};
void sig_chld(int signo)
{
    pid_t pid_chld;
    int stat;
    pid_t mypid_chld = waitpid(-1,&stat,WNOHANG);
    while((pid_chld = waitpid(-1,&stat,WNOHANG))> 0);
    printf("[srv](%d)[chd](%d) Child has terminated!\n",getpid(),mypid_chld);
}

void cli_biz(int connfd,pid_t mypid,char *cid);

int main(int argc, char **argv)
{
    char * srvip = argv[1];         //服务器套接字地址
    char * srvport = argv[2];    //服务器端口
    char * cid =argv[3];        //客户端编号

    int cnt = -1;

    int connfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP); //创建一个socket

    struct sockaddr_in srv_addr;
    memset(&srv_addr,0,sizeof(srv_addr));//初始化结构体
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = inet_addr(srvip);
    srv_addr.sin_port = htons(atoi(srvport));

    cnt = connect(connfd,(struct sockaddr *)&srv_addr,sizeof(struct sockaddr));
    pid_t mypid = getpid();
    if(cnt >=0)
    {
        printf("[cli](%d)[srv_sa](%s:%s) Server is connected!\n",mypid,srvip,srvport);
      
    }
    else{
        printf("error\n");
        exit(EXIT_FAILURE);
    }

    cli_biz(connfd,mypid,cid);

}

void cli_biz(int connfd,pid_t mypid,char *cid)
{
    struct pdu send_buff;
    memset(&send_buff,0,sizeof(send_buff));//初始化

    while(1)
    {
        char send_buff[BUF_LEN];
        memset(&send_buff,0,sizeof(send_buff));//初始化
        char read_buff[BUF_LEN];
        memset(&read_buff,0,sizeof(read_buff));//初始化
        fgets(send_buff,sizeof(send_buff),stdin);
        strcat(send_buff,"Host:hackr.jp\r\n\r\nmypostbody");

        ssize_t size_write = write(connfd,&send_buff,sizeof(send_buff));
        memset(send_buff,0,sizeof(send_buff));//清0

        ssize_t size_read;
        if((size_read = read(connfd,read_buff,sizeof(read_buff))) == 0){
            printf("read error!\n");
            return;
        }  
        printf("响应报文：\n");
        printf("%s",read_buff);
        getchar();
    }
} 