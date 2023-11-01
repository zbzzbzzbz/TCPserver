#include <sys/select.h>
#include <stdio.h>
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
#include <fcntl.h>


#define MAX_CLIENTS 1024
#define BUFFER_SIZE 1025
#define MID_SIZE 40
#define PATH "/home/zheng/shixun/shiyan10"
//注意：记得改路经

//定义类型常量
typedef struct {
    char *extension;
    char *mime_type;
} MimeMap;

MimeMap mime_map[] = {
    {".html", "text/html"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".css", "text/css"},
    {".js", "application/javascript"},
    {".ico", "image/x-icon"}
};

int send_ok(int sd,char *path,char *type){
    int fd = open(path,O_RDONLY);
    if(fd == -1)
    {
        write(sd,"404",3);
        return 0;
    }

    int size = lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);

    char head_buff[512] = {"HTTP/1.1 200 OK\r\n"};
    strcat(head_buff,"Server: myhttp\r\n");
    sprintf(head_buff+strlen(head_buff),"Content-type: %s\r\n",type);
    sprintf(head_buff+strlen(head_buff),"Content-Length: %d\r\n",size);
    strcat(head_buff,"\r\n");//分隔报头和数据 空行
    write(sd,head_buff,strlen(head_buff));
}

int send_notfound(int sd){
    int fd = open("error.html",O_RDONLY);
    if ( fd == -1 )
    {
        write(sd,"404",3);
        return 0;
    }

    int size = lseek(fd,0,SEEK_END);

    char head_buff[512] = {"HTTP/1.1 404 Not Found\r\n"};
    strcat(head_buff,"Server: myhttp\r\n");
    //sprintf连接字符串,第一个参数是被追加的位置
    sprintf(head_buff+strlen(head_buff),"Content-Length: %d\r\n",size);
    strcat(head_buff,"\r\n");//分隔报头和数据 空行
    write(sd,head_buff,strlen(head_buff));

    //读取error.html文件内容
    FILE* file = fopen("error.html", "r");
    if (file == NULL) {
        perror("打开文件失败");
        exit(EXIT_FAILURE);
    }
    //发送error.html内容
    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        write(sd,buffer,bytes_read);
    }        
}

char *get_mime_type(char *exten)
{
    int i, num_types;
    
    num_types = sizeof(mime_map) / sizeof(MimeMap);
    
    for (i = 0; i < num_types; i++) {
        if (strcmp(exten, mime_map[i].extension) == 0) {
            return mime_map[i].mime_type;
        }
    }
    
    return "text/plain"; // 默认返回纯文本类型
}


int handle_get_request(int sd,char *path)
{
    //查看路径文件是否存在
    if (access(path, F_OK) == 0) {
        printf("文件存在\n");
        char *type = "GET";
        int fd = open(path,O_RDONLY);
        if(fd == -1)
        {
            write(sd,"404",3);
            return 0;
        }

        int size = lseek(fd,0,SEEK_END);
        lseek(fd,0,SEEK_SET);

        char head_buff[512] = {"HTTP/1.1 200 OK\r\n"};
        strcat(head_buff,"Server: myhttp\r\n");
        sprintf(head_buff+strlen(head_buff),"Content-type: %s\r\n",type);
        sprintf(head_buff+strlen(head_buff),"Content-Length: %d\r\n",size);
        strcat(head_buff,"\r\n");//分隔报头和数据 空行
        write(sd,head_buff,strlen(head_buff));
        
        char *exten = strrchr(path,'.');
        char *mime_type = get_mime_type(exten);
        printf("mime_type is :%s\n",mime_type);
        // 读取index.html文件内容
        FILE* file = fopen(path, "r");
        if (file == NULL) {
            perror("打开文件失败");
            exit(EXIT_FAILURE);
        }
        //发送index.html内容
        char buffer[1024];
        size_t bytes_read;
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            //printf("bytes_read :%ld",bytes_read);
            write(sd,buffer,bytes_read);
        }        
    } else {
        printf("文件不存在\n");
        //重定向后浏览器自己会有一份记录，要保证第一次重定向的地址正确
        if(strcmp(path, "/home/zheng/shixun/shiyan10/321.txt") == 0)
        {
            //301
            memset(path,0,sizeof(path));
            char head_buff[512] = {"HTTP/1.1 301 Moved Permanently\r\n"};
            strcat(head_buff,"Server: myhttp\r\n");
            sprintf(head_buff+strlen(head_buff),"Location: %s\r\n","/login.html");
            sprintf(head_buff+strlen(head_buff),"Content-Length: %d\r\n",0);
            strcat(head_buff,"\r\n");//分隔报头和数据 空行
            memset(path,0,sizeof(path));
            strcpy(path,"/home/zheng/shixun/shiyan10/login.html");
            write(sd,head_buff,strlen(head_buff));
        }else{
            //发送错误应答报文
            send_notfound(sd);
        }
    }

}

int handle_post_request(int sd,char *path ,char *post_buff)
{

    //查看路径文件是否存在
    if (access(path, F_OK) == 0) {
        printf("文件存在\n");
        char *type = "POST";
        int fd = open(path,O_RDONLY);
        if(fd == -1)
        {
            write(sd,"404",3);
            return 0;
        }

        int size = lseek(fd,0,SEEK_END);
        lseek(fd,0,SEEK_SET);

        char head_buff[512] = {"HTTP/1.1 200 OK\r\n"};
        strcat(head_buff,"Server: myhttp\r\n");
        sprintf(head_buff+strlen(head_buff),"Content-type: %s\r\n",type);
        sprintf(head_buff+strlen(head_buff),"Content-Length: %d\r\n",size);
        strcat(head_buff,"\r\n");//分隔报头和数据 空行
        write(sd,head_buff,strlen(head_buff));

        //解析请求体中数据
        char *body = strstr(post_buff,"\r\n\r\n")+4;
        if(body == NULL)
        {
            printf("error");
        }
        char* username = strtok(body, "&");
        char* password = strtok(NULL, "&");
        
        printf("查询班级: %s\n", username);
        printf("关键字: %s\n", password); 

        // 返回响应给客户端
        char response[] = "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/html\r\n"
                      "\r\n"
                      "<html><body><h1>Login Successful!</h1></body></html>";
        write(sd, response, strlen(response));

    } else {
        printf("文件不存在\n");
        //发送错误应答报文
        send_notfound(sd);
    }
    
}

int handle_head_request(int sd,char *path)
{

    //查看路径文件是否存在
    if (access(path, F_OK) == 0) {
        printf("文件存在\n");
        send_ok(sd,path,"HEAD");
        

    } else {
        printf("文件不存在\n");
        //发送错误应答报文
        send_notfound(sd);
    }
}

int Handle_requestmss(int sd)
{
    while(1)
    {
        char buff[BUFFER_SIZE],post_buff[BUFFER_SIZE];
        char method[MID_SIZE],uri[MID_SIZE],version[MID_SIZE];
        memset(&buff,0,sizeof(buff));//初始化 


        ssize_t size_read;
        if((size_read = read(sd, buff, BUFFER_SIZE)) == 0){
            return 0;
        }  
        printf("请求报文：");
        printf("\n%s\n\n",buff);
        strcpy(post_buff,buff);
        char* request_line = strtok(buff, "\r\n");
        printf("request line :%s\n",request_line);
        sscanf(request_line,"%s %s %s",method,uri,version);
        printf("method:%s\n",method);
        printf("uri:%s\n",uri);
        printf("version:%s\n",version);


        char path[128] = PATH;
        strcat(path,uri);
        if(strcmp(uri, "/") == 0)
        {
            strcat(path,"index.html");
        }
        printf("path:%s\n",path);


        if (strcmp(method, "GET") == 0) {
            handle_get_request(sd,path);
        } else if (strcmp(method, "POST") == 0) {
            handle_post_request(sd,path,post_buff);
        } else if (strcmp(method, "HEAD") == 0) {
            handle_head_request(sd, path);
        }else{
        // 其他请求类型暂不处理，返回404错误
            send_notfound(sd);
        }

    }
    return 1;
}


int main(int argc, char **argv)
{
    char *myip = argv[1];
    int myport = atoi(argv[2]);

    struct sockaddr_in server_addr,client_addr;
    socklen_t client_len = sizeof(client_addr);
    int server_fd;
    // 创建服务器套接字
    if ((server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(myip);
    server_addr.sin_port = htons(myport);
    bind(server_fd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr));

    if(listen(server_fd,MAX_CLIENTS)!=-1){
        printf("[srv][srv_sa](%s:%d) Server has initializing!\n",myip,myport);
    }

    fd_set server_fd_set;//文件描述符集合
    int max_fd,sd,new_socket;
    int *client_fds;
    client_fds = (int *)malloc(sizeof(int)*MAX_CLIENTS);
    while(1)
    {
        FD_ZERO(&server_fd_set);
        FD_SET(server_fd,&server_fd_set);
        max_fd = server_fd;

        //添加客户端套接字到文件描述符集合
        for(int i=0;i< MAX_CLIENTS;i++)
        {
            sd = client_fds[i];

            if(sd>0)
                FD_SET(sd, &server_fd_set);

            if(sd > max_fd)
                max_fd = sd;
        }

        int activity = select(max_fd + 1, &server_fd_set, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            perror("select error");
        }

        if(FD_ISSET(server_fd, &server_fd_set))//有新的连接请求到达服务器套接字，需要使用accept函数来接受该连接并处理。
        {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&client_addr,&client_len)) < 0) {
                perror("accept failed");
                exit(EXIT_FAILURE);
            }

            printf("New connection, socket fd is %d\n", new_socket);

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_fds[i] == 0) {
                    client_fds[i] = new_socket;
                    printf("number of client is:%d\n",i+1);
                    break;
                }
            }
        }

        //处理客户端消息
        for(int i=0;i<MAX_CLIENTS;i++)
        {
            sd = client_fds[i];

            if(FD_ISSET(sd, &server_fd_set))
            {
                if(Handle_requestmss(sd) == 0){
                    printf("[srv] Client_socket(%d) disconnected\n",sd);

                    // 关闭套接字，并从客户端套接字数组中移除
                    close(sd);
                    client_fds[i] = 0;
                }
            }

        }


    }


    return 0;
}