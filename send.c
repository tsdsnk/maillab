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


char * mystrcpy(char * dst, const char * src){
    while(*src != '\0'){
        *dst = *src;
        dst++;
        src++;
    }
    *dst = '\0';
    return dst;
}





// receiver: mail address of the recipient
// subject: mail subject
// msg: content of mail body or path to the file containing mail body
// att_path: path to the attachment
void send_mail(const char* receiver, const char* subject, const char* msg, const char* att_path)
{
    const char* end_msg = "\r\n.\r\n";
    const char* host_name = "smtp.163.com"; // TODO: Specify the mail server domain name
    const unsigned short port = 25; // SMTP server port
    const char* user = "netlab123456789@163.com"; // TODO: Specify the user
    const char* pass = "IQPEECDKWNKIIKGB"; // TODO: Specify the password
    const char* from = "netlab123456789@163.com"; // TODO: Specify the mail address of the sender
    char dest_ip[16]; // Mail server IP address
    int s_fd; // socket file descriptor
    struct hostent *host;
    struct in_addr **addr_list;
    int i = 0;
    int r_size;

    int conn_fd;
    const char* mail_server = "220.181.15.161";

    fprintf(stdout, ">>> hostname:%s\n", host_name);
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

    // TODO: Create a socket, return the file descriptor to s_fd, and establish a TCP connection to the mail server
    s_fd = socket(AF_INET, SOCK_STREAM, 0); 
    
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(mail_server);

    if(conn_fd = connect(s_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        perror("tcp connect");
        exit(EXIT_FAILURE);
    }
    
    // Print welcome message
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", buf);

    // Send EHLO command and print server response
    const char* EHLO = "EHLO 163.com"; // TODO: Enter EHLO command here
    
    send(s_fd, EHLO, strlen(EHLO), 0);
    // TODO: Print server response to EHLO command
    if(r_size = recv(s_fd, buf, MAX_SIZE, 0) == -1){
        perror("recv: ELHO");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    printf("%s", buf);


    // TODO: Authentication. Server response should be printed out.
    const char* AUTH = "AUTH login\r\n";
    send(s_fd, AUTH, strlen(AUTH), 0);

    if(r_size = recv(s_fd, buf, MAX_SIZE, 0) == -1){
        perror("recv: AUTH");
        exit(EXIT_FAILURE);
    }

    char* encode_user = encode_str(user);
    strcpy(buf, encode_user);
    strcat(buf, "\r\n");
    send(s_fd, buf, strlen(buf), 0);
    free(encode_user);

    if(r_size = recv(s_fd, buf, MAX_SIZE, 0) == -1){
        perror("recv: AUTH");
        exit(EXIT_FAILURE);
    }

    char* encode_pass = encode_str(pass);
    strcpy(buf, encode_user);
    strcat(buf, "\r\n");
    send(s_fd, buf, strlen(buf), 0);
    free(encode_pass);

    if(r_size = recv(s_fd, buf, MAX_SIZE, 0) == -1){
        perror("recv: AUTH");
        exit(EXIT_FAILURE);
    }

    // TODO: Send MAIL FROM command and print server response
    strcpy(buf, "MAIL FROM:<");
    strcat(buf, user);
    strcat(buf, ">\r\n");
    send(s_fd, buf, strlen(buf), 0);
    if(r_size = recv(s_fd, buf, MAX_SIZE, 0) == -1){
        perror("recv: MAIL FROM");
        exit(EXIT_FAILURE);
    }

    // TODO: Send RCPT TO command and print server response
    strcpy(buf, "RCPT TO:<");
    strcat(buf, receiver);
    strcat(buf, ">\r\n");
    send(s_fd, buf, strlen(buf), 0);
    if(r_size = recv(s_fd, buf, MAX_SIZE, 0) == -1){
        perror("recv: RCPT TO");
        exit(EXIT_FAILURE);
    }
    // TODO: Send DATA command and print server response

    const char* DATA = "DATA\r\n";
    send(s_fd, buf, strlen(buf), 0);
    if(r_size = recv(s_fd, buf, MAX_SIZE, 0) == -1){
        perror("recv: DATA");
        exit(EXIT_FAILURE);
    }

    // TODO: Send message data
    char * pbuf;
    pbuf = mystrcpy(buf, "From: ");
    pbuf = mystrcpy(pbuf, user);
    pbuf = mystrcpy(pbuf, "\nTO: ");
    pbuf = mystrcpy(pbuf, receiver);
    pbuf = mystrcpy(pbuf, "\nMIME-Version: 1.0\nMessage-ID: <");
    sprintf(pbuf, "%ld", time((time_t *)NULL));
    pbuf += strlen(pbuf);
    pbuf = mystrcpy(pbuf, strchr(receiver, '@'));
    pbuf = mystrcpy(pbuf, ">\nContent-Type: multipart/mixed; boundary=qwertyuiopasdfghjklzxcvbnm\nSubject: ");
    pbuf = mystrcpy(pbuf, subject);
    pbuf = mystrcpy(pbuf, "\n\n");

    send(s_fd, buf, (pbuf - buf), 0);
    send(s_fd, msg, strlen(msg), 0);
    pbuf = mystrcpy(buf, "\n\n--qwertyuiopasdfghjklzxcvbnm\nContent-Type: ");
    char * fname = strrchr(att_path, '/');
    if(fname){
        pbuf = mystrcpy(pbuf, fname);
    }
    else{
        pbuf = mystrcpy(pbuf, att_path);
    }
    pbuf = mystrcpy(pbuf, "\n");
    send(s_fd, buf, (pbuf - buf), 0);

    FILE * fp = fopen(att_path, "r");
    if(fp == NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    FILE * encode_fp = fopen("./.encode_temp.txt", "w+");
    if(fp == NULL){
        perror("encode fopen");
        exit(EXIT_FAILURE);
    }
    encode_file(fp, encode_fp);
    fclose(fp);
    rewind(encode_fp);
    while(fgets(buf, MAX_SIZE, encode_fp) != NULL){
        send(s_fd, buf, strlen(buf), 0);
    }
    fclose(encode_fp);
    const char* end_split = "\n--qwertyuiopasdfghjklzxcvbnm\r\n";
    send(s_fd, end_split, strlen(end_split), 0);


    
    // TODO: Message ends with a single period
    send(s_fd, end_msg, strlen(end_msg), 0);
    if(r_size = recv(s_fd, buf, MAX_SIZE, 0) == -1){
        perror("recv: DATA");
        exit(EXIT_FAILURE);
    }
    // TODO: Send QUIT command and print server response
    send(s_fd, "QUIT", strlen("QUIT"), 0);
    if(r_size = recv(s_fd, buf, MAX_SIZE, 0) == -1){
        perror("recv: QUIT");
        exit(EXIT_FAILURE);
    }
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
