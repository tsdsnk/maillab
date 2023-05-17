#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define MAX_SIZE 65535

char buf[MAX_SIZE+1];


char * mystrcpy(char * dst, const char * src){
    while(*src != '\0'){
        *dst = *src;
        dst++;
        src++;
    }
    *dst = '\0';
    return dst;
}

void print_mail(int s_fd,int index);


void recv_mail()
{
    // 163邮箱的pop3服务器地址
    const char* host_name = "pop3.163.com"; // TODO: Specify the mail server domain name
    const unsigned short port = 110; // POP3 server port
    const char* user = "netlab123456789@163.com"; // TODO: Specify the user
    const char* pass = "IQPEECDKWNKIIKGB"; // TODO: Specify the password
    char dest_ip[16];
    int s_fd; // socket file descriptor
    struct hostent *host;
    struct in_addr **addr_list;
    int i = 0;
    int r_size;

    // Get IP from domain name
    if ((host = gethostbyname(host_name)) == NULL)
    {
        herror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    addr_list = (struct in_addr **) host->h_addr_list;
    while (addr_list[i] != NULL)
        ++i;
    strcpy(dest_ip, inet_ntoa(*addr_list[i-1]));

    // TODO: Create a socket,return the file descriptor to s_fd, and establish a TCP connection to the POP3 server
    // 创建套接字并建立tcp连接
    s_fd = socket(AF_INET, SOCK_STREAM, 0); 
    
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(dest_ip);

    if(connect(s_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        perror("tcp connect");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "connect finish\n");

    // Print welcome message
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", buf);

    char * pbuf;
    // TODO: Send user and password and print server response
    // 发送"USER ${user}\r\n"
    pbuf = mystrcpy(buf, "USER ");
    pbuf = mystrcpy(pbuf, user);
    pbuf = mystrcpy(pbuf, "\r\n");
    send(s_fd, buf, pbuf-buf, 0);
    fprintf(stdout, ">>>(%ld) %s", strlen(buf), buf);
    // 接收服务器回复
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    fprintf(stdout, "<<<%ld) %s", strlen(buf), buf);
    // 发送"PASS ${pass}\r\n"
    pbuf = mystrcpy(buf, "PASS ");
    pbuf = mystrcpy(pbuf, pass);
    pbuf = mystrcpy(pbuf, "\r\n");
    send(s_fd, buf, pbuf-buf, 0);
    fprintf(stdout, ">>>(%ld) %s", strlen(buf), buf);
    // 接收服务器消息
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    fprintf(stdout, "<<<%ld) %s", strlen(buf), buf);
    
    // TODO: Send STAT command and print server response
    // 发送"STAT\r\n"
    const char* STAT = "STAT\r\n";
    send(s_fd, STAT, strlen(STAT), 0);
    fprintf(stdout, ">>>(%ld) %s", strlen(STAT), STAT);
    // 接收服务器消息
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    fprintf(stdout, "<<<%ld) %s", strlen(buf), buf);

    // TODO: Send LIST command and print server response
    // 发送"LIST\r\n"
    const char* LIST = "LIST\r\n";
    send(s_fd, LIST, strlen(LIST), 0);
    fprintf(stdout, ">>>(%ld) %s", strlen(LIST), LIST);
    // 接收服务器消息
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    fprintf(stdout, "<<<%ld) %s", strlen(buf), buf);
    // TODO: Retrieve the first mail and print its content
    print_mail(s_fd, 1);
    // TODO: Send QUIT command and print server response
    // 发送"QUIT\r\n"
    const char* QUIT = "QUIT\r\n";
    send(s_fd, QUIT, strlen(QUIT), 0);
    fprintf(stdout, ">>>(%ld) %s", strlen(QUIT), QUIT);
    // 接收服务器消息
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    fprintf(stdout, "<<<%ld) %s", strlen(buf), buf);

    close(s_fd);
}

int main(int argc, char* argv[])
{
    recv_mail();
    exit(0);
}


/**
 * 打印第index封邮件
*/
void print_mail(int s_fd,int index){
    // 发送"RETR ${index}\r\n"
    char* pbuf;
    pbuf = mystrcpy(buf, "RETR ");
    sprintf(pbuf, "%d\r\n", index);
    send(s_fd, buf, strlen(buf), 0);
    fprintf(stdout, ">>>(%ld) %s", strlen(buf), buf);
    // 打印返回报文
    int r_size;
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    fprintf(stdout, "<<<(%ld) %s", strlen(buf), buf);
}