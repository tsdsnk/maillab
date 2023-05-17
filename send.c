#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <getopt.h>
#include "base64_utils.h"
#include <time.h>

#define MAX_SIZE 4095

char buf[MAX_SIZE+1];

/**
 * 自定义的strcpy
 * 在strcpy的基础上返回dst的字符串末尾地址(指向'\0'的地址)
*/
char * mystrcpy(char * dst, const char * src){
    while(*src != '\0'){
        *dst = *src;
        dst++;
        src++;
    }
    *dst = '\0';
    return dst;
}

/**
 * 从路径截取文件名
*/
void get_filename(char* name, const char* pth){
    char* p = strrchr(pth, '/');
    if(p == NULL){
        strcpy(name, pth);
    }else{
        strcpy(name, p+1);
    }
}



void send_content(int s_fd, FILE* fp, char* filename);






// receiver: mail address of the recipient
// subject: mail subject
// msg: content of mail body or path to the file containing mail body
// att_path: path to the attachment
void send_mail(const char* receiver, const char* subject, const char* msg, const char* att_path)
{
    /**
     * 实验测试采用163邮箱
     * smtp的服务器名称smtp.163.com
     * 测试用邮箱netlab123456789@163.com
    */
    
    const char* end_msg = "\r\n.\r\n";
    const char* host_name = "smtp.163.com"; // TODO: Specify the mail server domain name
    const unsigned short port = 25; // SMTP server port
    const char* user = "netlab123456789@163.com"; // TODO: Specify the user
    const char* pass = "*****"; // TODO: Specify the password
    const char* from = "netlab123456789@163.com"; // TODO: Specify the mail address of the sender
    char dest_ip[16]; // Mail server IP address
    int s_fd; // socket file descriptor
    struct hostent *host;
    struct in_addr **addr_list;
    int i = 0;
    int r_size;

    int conn_fd;

    fprintf(stdout, "================= send begin =================\n");
    fprintf(stdout, "receiver: %s\nsubject: %s\nmsg: %s\natt_path: %s\n", receiver, subject, msg, att_path);
    fprintf(stdout, "==============================================\n");


    /**
     * 邮件格式为
     * +--------multipart/mixed-------------+
     * |                                    |
     * |    +---multupart/alternative---+   |
     * |    |                           |   |
     * |    |           正文            |   |
     * |    |                           |   |
     * |    +---------------------------+   |
     * |                                    |
     * |                附件                |
     * |                                    |
     * +------------------------------------|
     * 
     * 其中两个部分的分割线bound如下
    */

    const char* split_mixed = "qwertyuiopasdfghjklzxcvbnm";
    const char* split_alternative = "mnbvcxzlkjhgfdsapoiuytrewq";



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
    fprintf(stdout, "dest_ip %s\n", dest_ip);

    // TODO: Create a socket, return the file descriptor to s_fd, and establish a TCP connection to the mail server
    
    // 创建套接字并建立tcp连接
    s_fd = socket(AF_INET, SOCK_STREAM, 0); 
    
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(dest_ip);

    if(conn_fd = connect(s_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
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

    // Send EHLO command and print server response
    // 发送"EHLO 163.com\r\n"
    const char* EHLO = "EHLO 163.com\r\n"; // TODO: Enter EHLO command here
    send(s_fd, EHLO, strlen(EHLO), 0);
    fprintf(stdout, ">>>(%ld) %s\n", strlen(EHLO), EHLO);
    
    // TODO: Print server response to EHLO command
    if((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1){
        perror("recv: ELHO");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    fprintf(stdout, "<<<(%ld) %s\n", strlen(buf), buf);


    // TODO: Authentication. Server response should be printed out.
    // 发送"AUTH login\r\n"，请求登录
    const char* AUTH = "AUTH login\r\n";
    send(s_fd, AUTH, strlen(AUTH), 0);
    fprintf(stdout, ">>>(%ld) %s\n", strlen(AUTH), AUTH);
   // 接受服务器返回的base64编码的"Username:"
   if((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1){
        perror("recv: AUTH");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    fprintf(stdout, "<<<(%ld) %s\n", strlen(buf), buf);

    // 发送base64的邮箱账户名
    char* encode_user = encode_str(user);
    send(s_fd, encode_user, strlen(encode_user), 0);
    fprintf(stdout, ">>>(%ld) %s\n", strlen(encode_user), encode_user);
    free(encode_user);
    // 接收服务器返回的base64编码的"Password:"
    if((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1){
        perror("recv: AUTH");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    fprintf(stdout, "<<<(%ld) %s\n", strlen(buf), buf);

    // 发送base64的邮箱授权码
    char* encode_pass = encode_str(pass);
    send(s_fd, encode_pass, strlen(encode_pass), 0);
    fprintf(stdout, ">>>(%ld) %s\n", strlen(encode_pass), encode_pass);
    free(encode_pass);
    // 接受服务器返回的登录成功与否信息
    if((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1){
        perror("recv: AUTH");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    fprintf(stdout, "<<<(%ld) %s\n", strlen(buf), buf);

    // TODO: Send MAIL FROM command and print server response
    // 发送"MAIL FROM<$user>\r\n"
    strcpy(buf, "MAIL FROM:<");
    strcat(buf, user);
    strcat(buf, ">\r\n");
    send(s_fd, buf, strlen(buf), 0);
    fprintf(stdout, ">>>(%ld) %s\n", strlen(buf), buf);
    // 接受服务器返回"250 OK"
    if((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1){
        perror("recv: MAIL FROM");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    fprintf(stdout, "<<<(%ld) %s\n", strlen(buf), buf);

    // TODO: Send RCPT TO command and print server response
    // 发送"RCPT TO:<$receiver>\r\n"
    strcpy(buf, "RCPT TO:<");
    strcat(buf, receiver);
    strcat(buf, ">\r\n");
    send(s_fd, buf, strlen(buf), 0);
    fprintf(stdout, ">>>(%ld) %s\n", strlen(buf), buf);
    // 接收服务器返回"250 OK"
    if((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1){
        perror("recv: RCPT TO");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    fprintf(stdout, "<<<(%ld) %s\n", strlen(buf), buf);

    // TODO: Send DATA command and print server response
    // 发送"DATA\r\n"
    const char* DATA = "DATA\r\n";
    send(s_fd, DATA, strlen(DATA), 0);
    fprintf(stdout, ">>>(%ld) %s\n", strlen(DATA), DATA);
    // 接收服务器返回"354 End data with <CR><LF>.<CR><LF>"
    if((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1){
        perror("recv: DATA");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    fprintf(stdout, "<<<(%ld) %s\n", strlen(buf), buf);


    // TODO: Send message data
    /**
     * 填写DATA数据部分报头
     * 
     * =================================================================
     * From: ${user}\n
     * TO: ${receiver}\n
     * MIME-Version: 1.0\n
     * Message-ID: <${Message_id}>\n              // 本实现中采用 当前时间戳 + receiver变量的'@'字符后变量
     * Content-Type: multipart/mixed; boundary=${split_mixd}\n
     * Subject: ${subject}\n
     * \n
     * =================================================================
     * 具体实现借用了mystrcpy函数，只是在strcpy基础上返回了复制串后的'\0'的地址
     * 头部代码如下
     * 
    */
    char * pbuf;
    pbuf = mystrcpy(buf, "From: ");
    pbuf = mystrcpy(pbuf, user);
    pbuf = mystrcpy(pbuf, "\nTO: ");
    pbuf = mystrcpy(pbuf, receiver);
    pbuf = mystrcpy(pbuf, "\nMIME-Version: 1.0\nMessage-ID: <");
    // 获取当前时间戳作为@前部分
    sprintf(pbuf, "%ld", time((time_t *)NULL));
    pbuf += strlen(pbuf);
    // '@'后部分使用receiver的邮箱域名
    pbuf = mystrcpy(pbuf, strchr(receiver, '@'));
    pbuf = mystrcpy(pbuf, ">\nContent-Type: multipart/mixed; boundary=");
    pbuf = mystrcpy(pbuf, split_mixed);
    pbuf = mystrcpy(pbuf, "\nSubject: ");
    pbuf = mystrcpy(pbuf, subject);
    pbuf = mystrcpy(pbuf, "\n\n");
    
    // 先将部分报文发送，释放buf空间
    send(s_fd, buf, (pbuf - buf), 0);
    fprintf(stdout, "%s", buf);

    char *filename;
    // 如果存在正文部分邮件
    if(msg != NULL){
        // 获取文件名字，即从路径截取最后一个'/'之后的字符串
        filename = malloc((strlen(msg)+1)*sizeof(char));
        get_filename(filename, msg);
        // 以只读方式打开文件
        FILE * fmsg = fopen(msg, "r");
        if(fmsg == NULL){
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        /**
         * 继续追加data部分
         * =======================================================================
         * --${split_mixed}\n                                   // multipart/mixed部分的模块
         * Content-Type: multipart/alternative;boundary=${split_alternative};\n     // 声明multipart/alternative的子模块
         * \n
         * --${split_alternative}\n                 // multipart/alternative的模块内容
         * Content-Type: text/plain;\n              // 该模块以text/plaint纯文本形式
         * Content-Transfer-Encoding: base64\n      // 该模块使用base64编码
         * \n
         * ${file_data_base64}\n                    // base64编码的正文内容
         * \n
         * --${split_alternative}--                 // 标志multipart/alternative模块结束
         * =========================================================================
        */
        pbuf = mystrcpy(buf, "--");
        pbuf = mystrcpy(pbuf, split_mixed);
        pbuf = mystrcpy(pbuf, "\nContent-Type: multipart/alternative;");
        pbuf = mystrcpy(pbuf, "boundary=");
        pbuf = mystrcpy(pbuf, split_alternative);
        pbuf = mystrcpy(pbuf, ";\n\n");
        pbuf = mystrcpy(pbuf, "--");
        pbuf = mystrcpy(pbuf, split_alternative);
        pbuf = mystrcpy(pbuf, "\nContent-Type: text/plain;\nContent-Transfer-Encoding: base64\n\n");
        // 先发送部分数据，清空buf便于读取文件内容
        send(s_fd, buf, pbuf-buf, 0);
        fprintf(stdout, "%s", buf);
        // 以base64编码并发送 FILE* fmsg的内容，追加"\n\n"
        send_content(s_fd, fmsg, filename);

        pbuf = mystrcpy(buf, "--");
        pbuf = mystrcpy(pbuf, split_alternative);
        pbuf = mystrcpy(pbuf, "--\n\n");

        free(filename);
        fclose(fmsg);
    }

    if(att_path != NULL){
        // 获取附件的文件名
        filename = malloc((strlen(att_path)+1)*sizeof(char));
        get_filename(filename, att_path);
        // 以只读方式打开文件
        FILE * fp = fopen(att_path, "r");
        if(fp == NULL){
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        /**
         * 继续追加data部分
         * ==================================================================================================
         * --${split_mixed}\n                           // multipart/mixed的分割线
         * Content-Type: application/octet-stream;name=${filename}\n      // 文件格式为二进制，文件名为${filename}
         * Content-Transfer-Encoding: base64\n          // 以base64格式编码
         * \n
         * ${file_data_base64}\n                        // 以base64编码的文件内容
         * \n
         * ==================================================================================================
        */
        pbuf = mystrcpy(buf, "--");
        pbuf = mystrcpy(pbuf, split_mixed);
        pbuf = mystrcpy(pbuf, "\nContent-Type: application/octet-stream;name=");
        pbuf = mystrcpy(pbuf, filename);
        pbuf = mystrcpy(pbuf, "\nContent-Transfer-Encoding: base64\n\n");
        // 发送部分内容以清空buf
        send(s_fd, buf, pbuf-buf, 0);
        fprintf(stdout, "%s", buf);
        // 发送文件内容，以base64编码
        send_content(s_fd, fp, filename);
        fclose(fp);
        free(filename);
    }

    /**
     * 追加DATA部分
     * ========================================================================
     * --${split_mixed}--\n             \\ 结束 multipart/mixed部分
     * \r\n.\r\n                        \\告知服务器DATA部分结束
     * ========================================================================
    */
    pbuf = mystrcpy(buf, "--");
    pbuf = mystrcpy(pbuf, split_mixed);
    pbuf = mystrcpy(pbuf, "--\n");
    send(s_fd, buf, (pbuf-buf), 0);
    fprintf(stdout, "%s\n", buf);
    // TODO: Message ends with a single period
    send(s_fd, end_msg, strlen(end_msg), 0);
    // 接受服务器返回的"250 OK"
    if((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1){
        perror("recv: DATA");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    fprintf(stdout, "<<<(%ld) %s\n",strlen(buf), buf);
    // TODO: Send QUIT command and print server response
    // 发送"QUIT\r\n""
    send(s_fd, "QUIT\r\n", strlen("QUIT\r\n"), 0);
    // 接收服务器返回的"221 Bye"
    if((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1){
        perror("recv: QUIT");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    fprintf(stdout, "<<<(%ld) %s\n",strlen(buf), buf);
    close(s_fd);
}

int main(int argc, char* argv[])
{
    int opt;
    char* s_arg = NULL;
    char* m_arg = NULL;
    char* a_arg = NULL;
    char* recipient = NULL;
    const char* optstring = ":s:m:a:";
    while ((opt = getopt(argc, argv, optstring)) != -1)
    {
        switch (opt)
        {
        case 's':
            s_arg = optarg;
            break;
        case 'm':
            m_arg = optarg;
            break;
        case 'a':
            a_arg = optarg;
            break;
        case ':':
            fprintf(stderr, "Option %c needs an argument.\n", optopt);
            exit(EXIT_FAILURE);
        case '?':
            fprintf(stderr, "Unknown option: %c.\n", optopt);
            exit(EXIT_FAILURE);
        default:
            fprintf(stderr, "Unknown error.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc)
    {
        fprintf(stderr, "Recipient not specified.\n");
        exit(EXIT_FAILURE);
    }
    else if (optind < argc - 1)
    {
        fprintf(stderr, "Too many arguments.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        recipient = argv[optind];
        send_mail(recipient, s_arg, m_arg, a_arg);
        exit(0);
    }
}


void send_content(int s_fd, FILE* fp, char* filename){
    // 以读写、清空原文件的方式打开临时文件./.encode_temp.txt
    char* pbuf;
    FILE * encode_fp = fopen("./.encode_temp.txt", "w+");
    if(encode_fp == NULL){
        perror("encode fopen:");
        exit(EXIT_FAILURE);
    }
    encode_file(fp, encode_fp);
    // 将文件指针重新置于文件开头
    rewind(encode_fp);
    // 每MAX_SIZE或遇到'\n'则发送
    while(fgets(buf, MAX_SIZE, encode_fp) !=NULL){      
        send(s_fd, buf, strlen(buf), 0);
        fprintf(stdout, "%s", buf);
    }
    fclose(encode_fp);
    // 文件结尾追加"\n\n"
    send(s_fd, "\n\n", strlen("\n\n"), 0);
    fprintf(stdout, "\n\n");
}
