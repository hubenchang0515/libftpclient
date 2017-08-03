#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ftpclient.h"


/* login , return fd */
int login()
{
    char username[128];
    char* ipv4 = username;
    char* buffer = username;
    uint16_t port;
    
    /* Connect to FTP server */
    printf("FTP Server IPv4 : ");
    scanf("%s",buffer);
    printf("FTP Server Port : ");
    scanf("%hu",&port);
    int csock = ftpCreateControlConnection(ipv4,port);
    if(csock < 0)
    {
        printf("Cannot connet to %s : %u\n",ipv4,port);
        return -1;
    }
    
    /* login */
    printf("       Username : ");
    scanf("%s",username);
    char* password = getpass("       Password : ");
    if(ftpLogin(csock,username,password))
    {
        return csock;
    }
    else
    {
        return -1;
    }
    
}


/* print help list */
void help()
{
    printf("help            -   show this help.\n");
    printf("list            -   show file list.\n");
    printf("cd <path>       -   change dierctory.\n");
    printf("get <file>      -   download file.\n");
    printf("put <file>      -   upload file.\n");
    printf("quit            -   quit.\n");
}

/* print file list */
void list(int sock)
{ 
    int dsock = ftpCreateDataConnection(sock);
    ftpTryWrite(sock,"LIST\r\n",6);
    
    char buffer[1024];
    ssize_t len = 0;
    
    /* 开始传输文件列表 */
    //len = ftpTryRead(sock,buffer,1024);
    //write(STDOUT_FILENO,buffer,len);
    
    /* 打印文件列表 */
    do
    {
        len = ftpTryRead(dsock,buffer,1024);
        write(STDOUT_FILENO,buffer,len);
    }while(len > 0);
    
    /* 文件列表传输完毕 */
    //len = ftpTryRead(sock,buffer,1024);
    //write(STDOUT_FILENO,buffer,len);
    
    close(dsock);
}

/* call function by coomand */
void command(int sock,const char* cmd)
{
    /* get param */
    char* param = strstr(cmd," ");
    while(param != NULL && *param == ' ')
    {
        param += 1;
    }
    
    if(!strncmp(cmd,"quit",4))
    {
        ftpClose(sock);
        exit(0);
    }
    else if(!strncmp(cmd,"help",4))
    {
        help();
    }
    else if(!strncmp(cmd,"list",4))
    {
        list(sock);
    }
    else if(!strncmp(cmd,"cd",2))
    {
        ftpChangeDirectory(sock,param);
    }
    else if(!strncmp(cmd,"get",3))
    {
        int dsock;
        dsock = ftpCreateDataConnection(sock);
        if(dsock < 0 || !ftpDownload(sock,dsock,param))
        {
            printf("Download failed.\n");
            return;
        }
    }
    else if(!strncmp(cmd,"put",3))
    {
        int dsock;
        dsock = ftpCreateDataConnection(sock);
        if(dsock < 0 || !ftpUpload(sock,dsock,param))
        {
            printf("Upload failed.\n");
            return;
        }
    }
    else
    {
        printf("Unknow Command.\n");
        help();
    }
}



int main(void)
{
    int csock = login();
    if(csock < 0)
    {
        printf("Create control connection failed.\n");
        return 1;
    }
    
    fd_set rfds; // fd_set to read
    struct timeval waitTime; // time to wait
    struct timeval* timeout = &waitTime; // time of timeout
    
    while(1)
    {
        /* init fd_set */
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO,&rfds);
        FD_SET(csock,&rfds);
        
        /* timeout */
        waitTime.tv_sec  = 0;
        waitTime.tv_usec = 100000;

        
        int state = select(csock + 1, &rfds, NULL,NULL,timeout);
        switch(state)
        {
            case 0: timeout = NULL; write(STDOUT_FILENO,"FTP >> ",7); break; // if timeout print "FTP >> ", last time will wait forever
            
            case -1: return 1;// error
            
            default: 
            {
                timeout = &waitTime;    // last time wait 1000us
                ssize_t length = 0;     // length of data
                char buffer[1024];      // data
                /* read command from stdin */
                if(FD_ISSET(STDIN_FILENO,&rfds))
                {
                    length = 0;
                    length = read(STDIN_FILENO,buffer,1024);
                    buffer[length-1] = 0; // '\n' ==> 0
                    /* execute input command */
                    command(csock,buffer);
                }
                
                /* read data from socket */
                if(FD_ISSET(csock,&rfds))
                {
                    length = 0;
                    length = read(csock,buffer,1024);
                    if(length <= 0)
                    {
                        printf("FTP server closed connection.\n");
                        return 1;
                    }
                    write(STDOUT_FILENO,buffer,length);
                }
            }
        }
    }
}


