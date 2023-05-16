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

void get_filename(char* name, const char* pth){
    char* p = strrchr(pth, '/');
    if(p == NULL){
        strcpy(name, pth);
    }else{
        strcpy(name, p+1);
    }
}



void send_content(int s_fd, const char * split, char* content_type, FILE* fp, char* filename);






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

    fprintf(stdout, "================= send begin =================\n");
    fprintf(stdout, "receiver: %s\nsubject: %s\nmsg: %s\natt_path: %s\n", receiver, subject, msg, att_path);
    fprintf(stdout, "==============================================\n");


    const char* split = "qwertyuiopasdfghjklzxcvbnm";


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
    const char* AUTH = "AUTH login\r\n";
    send(s_fd, AUTH, strlen(AUTH), 0);
    fprintf(stdout, ">>>(%ld) %s\n", strlen(AUTH), AUTH);
    if((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1){
        perror("recv: AUTH");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    fprintf(stdout, "<<<(%ld) %s\n", strlen(buf), buf);


    char* encode_user = encode_str(user);
    send(s_fd, encode_user, strlen(encode_user), 0);
    fprintf(stdout, ">>>(%ld) %s\n", strlen(encode_user), encode_user);
    free(encode_user);

    if((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1){
        perror("recv: AUTH");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    fprintf(stdout, "<<<(%ld) %s\n", strlen(buf), buf);


    char* encode_pass = encode_str(pass);
    send(s_fd, encode_pass, strlen(encode_pass), 0);
    fprintf(stdout, ">>>(%ld) %s\n", strlen(encode_pass), encode_pass);
    free(encode_pass);

    if((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1){
        perror("recv: AUTH");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    fprintf(stdout, "<<<(%ld) %s\n", strlen(buf), buf);

    // TODO: Send MAIL FROM command and print server response
    strcpy(buf, "MAIL FROM:<");
    strcat(buf, user);
    strcat(buf, ">\r\n");
    send(s_fd, buf, strlen(buf), 0);
    fprintf(stdout, ">>>(%ld) %s\n", strlen(buf), buf);
    if((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1){
        perror("recv: MAIL FROM");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    fprintf(stdout, "<<<(%ld) %s\n", strlen(buf), buf);

    // TODO: Send RCPT TO command and print server response
    strcpy(buf, "RCPT TO:<");
    strcat(buf, receiver);
    strcat(buf, ">\r\n");
    send(s_fd, buf, strlen(buf), 0);
    fprintf(stdout, ">>>(%ld) %s\n", strlen(buf), buf);
    if((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1){
        perror("recv: RCPT TO");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    fprintf(stdout, "<<<(%ld) %s\n", strlen(buf), buf);
    // TODO: Send DATA command and print server response

    const char* DATA = "DATA\r\n";
    send(s_fd, DATA, strlen(DATA), 0);
    fprintf(stdout, ">>>(%ld) %s\n", strlen(DATA), DATA);
    if((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1){
        perror("recv: DATA");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    fprintf(stdout, "<<<(%ld) %s\n", strlen(buf), buf);


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
    pbuf = mystrcpy(pbuf, ">\nContent-Type: multipart/mixed; boundary=");
    pbuf = mystrcpy(pbuf, split);
    pbuf = mystrcpy(pbuf, "\nSubject: ");
    pbuf = mystrcpy(pbuf, subject);
    pbuf = mystrcpy(pbuf, "\n\n");
    
    send(s_fd, buf, (pbuf - buf), 0);
    fprintf(stdout, "%s", buf);


    char *filename;
    if(msg != NULL){
        filename = malloc((strlen(msg)+1)*sizeof(char));
        get_filename(filename, msg);
        FILE * fmsg = fopen(msg, "r");
        if(fmsg == NULL){
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        send_content(s_fd, split, "text/plan", fmsg, filename);
        free(filename);
        fclose(fmsg);
    }

    if(att_path != NULL){

        filename = malloc((strlen(att_path)+1)*sizeof(char));
        get_filename(filename, att_path);
        FILE * fp = fopen(att_path, "r");
        if(fp == NULL){
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        
        send_content(s_fd, split, "application/zip", fp, filename);
        fclose(fp);
        free(filename);
    }
    pbuf = mystrcpy(buf, "--");
    pbuf = mystrcpy(pbuf, split);
    pbuf = mystrcpy(pbuf, "\n");
    send(s_fd, buf, (pbuf-buf), 0);
    fprintf(stdout, "%s\n", buf);


    
    // TODO: Message ends with a single period
    send(s_fd, end_msg, strlen(end_msg), 0);
    if((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1){
        perror("recv: DATA");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0';
    fprintf(stdout, "<<<(%ld) %s\n",strlen(buf), buf);
    // TODO: Send QUIT command and print server response
    send(s_fd, "QUIT\r\n", strlen("QUIT\r\n"), 0);

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


void send_content(int s_fd, const char * split, char* content_type, FILE* fp, char* filename){
    char* pbuf;
    pbuf = mystrcpy(buf, "--");
    pbuf = mystrcpy(pbuf, split);
    pbuf = mystrcpy(pbuf, "\nContent-Type: ");
    pbuf = mystrcpy(pbuf, content_type);
    pbuf = mystrcpy(pbuf, ";name=\"");
    pbuf = mystrcpy(pbuf, filename);
    pbuf = mystrcpy(pbuf, "\"\nContent-Transfer-Encoding: base64\n\n");
    send(s_fd, buf, (pbuf-buf), 0);
    fprintf(stdout, "%s", buf);

    FILE * encode_fp = fopen("./.encode_temp.txt", "w+");
    if(encode_fp == NULL){
        perror("encode fopen:");
        exit(EXIT_FAILURE);
    }
    encode_file(fp, encode_fp);
    rewind(encode_fp);

    while(fgets(buf, MAX_SIZE, encode_fp) !=NULL){      
        send(s_fd, buf, strlen(buf), 0);
        fprintf(stdout, "%s", buf);
    }
    fclose(encode_fp);
    send(s_fd, "\n\n", strlen("\n\n"), 0);
    fprintf(stdout, "\n\n");
}