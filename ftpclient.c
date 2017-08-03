/*
 * File     : ftpclient.c
 * Author   : blog.kurukurumi.com
 *
 */







#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ftpclient.h"


/*
 * TODO
 *  Try to read file , 1s timeout
 *
 * PARAM
 *  fd     : file descriptor
 *  buffer : read buffer
 *  bytes  : buffer size
 *
 * RETURN
 *  success  : >0
 *  file end : 0
 *  timeout  : -1
 *  failed   : -2
 */
ssize_t ftpTryRead(int fd,void* buffer,size_t bytes)
{
    /* block 1s at most */
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0; 
    
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd,&fds);
    
    switch(select(fd+1,&fds,NULL,NULL,&timeout))
    {
        case -1: return -2; // error
        
        case 0 : return -1;  // timeout
        
        default: return read(fd,buffer,bytes);
    }
}


/*
 * TODO
 *  Try to write file , 1s timeout
 *
 * PARAM
 *  fd     : file descriptor
 *  buffer : write buffer
 *  bytes  : buffer size
 *
 * RETURN
 *  success  : >0
 *  file end : 0
 *  timeout  : -1
 *  failed   : -2
 */
ssize_t ftpTryWrite(int fd,const void* buffer,size_t bytes)
{

    /* block 1s at most */
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0; 
    
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd,&fds);
    
    switch(select(fd+1,NULL,&fds,NULL,&timeout))
    {
        case -1: return -2; // error
        
        case 0 : return -1;  // timeout
        
        default: return write(fd,buffer,bytes);
    }
}




/*
 * TODO
 *  Connect to ip:port
 *
 * PARAM
 *  ipv4 : the ipv4 address of ftp server
 *  port : the port of ftp server
 *
 *
 * RETURN
 *  success : socket
 *  failed  : -1
 */
int ftpConnect(const char* ipv4,uint16_t port)
{
    int sock = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in ftpAddr;
    ftpAddr.sin_family = AF_INET;
    ftpAddr.sin_addr.s_addr = inet_addr(ipv4);
    ftpAddr.sin_port = htons(port);
    if(connect(sock,(struct sockaddr*)&ftpAddr,sizeof(ftpAddr)))
    {
        return -1;
    }
    else
    {
        return sock;
    }
}





/*
 * TODO
 *  Close
 *
 * PARAM
 *  sock     : socket of ftp control connection
 *
 */
void ftpClose(int sock)
{
    ftpTryWrite(sock,"QUIT\r\n",6);
    close(sock);
}





/*
 * TODO
 *  Create FTP control connection 
 *
 * PARAM
 *  ipv4 : the ipv4 address of ftp server
 *  port : the port of ftp server
 *
 *
 * RETURN
 *  success : socket of ftp
 *  failed  : -1
 */
int ftpCreateControlConnection(const char* ipv4,uint16_t port)
{
    int sock = ftpConnect(ipv4,port);
    if(sock != -1)
    {  
        char buffer[128];
        ftpTryRead(sock,buffer,128);
        return sock;
    }
    else
    {
        return -1;
    }
}





/*
 * TODO
 *  Create FTP data connection 
 *
 * PARAM
 *  sock     : socket of ftp control connection
 *
 * RETURN
 *  success : socket of ftp data connection
 *  failed  : -1
 */
int ftpCreateDataConnection(int sock)
{
    /* send PASV */
    if(ftpTryWrite(sock,"PASV\r\n",6) <= 0)
    {
        return -1;
    }
    
    /* read ip and port */
    char buffer[128];
    ssize_t len = ftpTryRead(sock,buffer,128);
    if(len <= 0)
    {
        write(STDOUT_FILENO,buffer,len);
        return -1;
    }
    
    unsigned int ip0,ip1,ip2,ip3,port0,port2;
    char* fp = strstr(buffer,"(");
    sscanf(fp,"(%u,%u,%u,%u,%u,%u",&ip0,&ip1,&ip2,&ip3,&port0,&port2);
    char ip[32];
    sprintf(ip,"%u.%u.%u.%u",ip0,ip1,ip2,ip3);
    uint16_t port = port0*256 + port2;
    return ftpConnect(ip,port);
}





/*
 * TODO
 *  Login
 *
 * PARAM
 *  sock     : socket of ftp control connection
 *  username : username
 *  password : password
 *
 *
 * RETURN
 *  success : 1
 *  failed  : 0
 */
int ftpLogin(int sock,const char* username,const char* password)
{
    char buffer[128];
    
    /* send username */
    if(
        ftpTryWrite(sock,"USER ",5) <= 0 ||
        ftpTryWrite(sock,username,strlen(username)) <= 0 ||
        ftpTryWrite(sock,"\r\n",2) <= 0
      )
    {
        return 0;
    }
    
    ssize_t len = ftpTryRead(sock,buffer,128);
    if(len <= 0)
    {
        return 0;
    }
    
    /* send password */
    if(
        ftpTryWrite(sock,"PASS ",5) <= 0 ||
        ftpTryWrite(sock,password,strlen(password)) <= 0 ||
        ftpTryWrite(sock,"\r\n",2) <= 0
      )
    {
        return 0;
    }
    
    len = ftpTryRead(sock,buffer,128);
    if(len <= 0)
    {
        return 0;
    }
    
    /* check */
    if(!strncmp(buffer,"230",3))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}





/*
 * TODO
 *  Change FTP server working directory
 *
 * PARAM
 *  sock : socket of ftp control connection
 *  path : path to change to
 *
 * RETURN
 *  succedd : 1
 *  failed  : 0
 *
 */
int ftpChangeDirectory(int sock,const char* path)
{
    if(
        ftpTryWrite(sock,"CWD ",4) <= 0 ||
        ftpTryWrite(sock,path,strlen(path)) <=0 ||
        ftpTryWrite(sock,"\r\n",2) <= 0
    )
    {
        return 0;
    }
    
    char buffer[128];
    ssize_t len = ftpTryRead(sock,buffer,128);
    if(len <=0 || strncmp(buffer,"250",3))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}




/*
 * TODO
 *  Download file
 *
 * PARAM
 *  csock   : socket of ftp control connection
 *  dsock   : socket of ftp data connection
 *  file    : file path in server
 *
 * RETURN
 *  success : 1
 *  failed  : 0
 */
int ftpDownload(int csock,int dsock,const char* file)
{
    /* 发出下载的指令 */
    if(
        ftpTryWrite(csock,"RETR ",5) <= 0 ||
        ftpTryWrite(csock,file,strlen(file)) <= 0 ||
        ftpTryWrite(csock,"\r\n",2) <= 0
    )
    {
        return 0;
    }
        
    char buffer[1024];
    ssize_t len;
    
    /* 检查服务器应答 150开始传输 */
    len = ftpTryRead(csock,buffer,1024);
    if(len < 3 || strncmp(buffer,"150",3))
    {
        return 0;
    }
    
    /* 创建本地文件，不提示存在，直接覆盖 */
    const char* name = strrchr(file,'/');
    name = name != NULL ? name + 1 : file;
    int fd = open(name,O_WRONLY|O_CREAT,0755);
    if(fd < 0)
    {
        return 0;
    }
    
    /* 读取数据链接 */
    do
    {
        len = ftpTryRead(dsock,buffer,1024);
        write(fd,buffer,len);
    }while(len>0);
    close(fd);
    close(dsock);
    
    /* 检查服务器应答 226传输完成 */
    len = ftpTryRead(csock,buffer,1024);
    if(len < 3 || strncmp(buffer,"226",3))
    {
        return 0;
    }
    
    
    return 1;
}





/*
 * TODO
 *  Upload file
 *
 * PARAM
 *  csock   : socket of ftp control connection
 *  dsock   : socket of ftp data connection
 *  file    : file path in local
 *
 * RETURN
 *  success : 1
 *  failed  : 0
 */
int ftpUpload(int csock,int dsock,const char* file)
{
    /* 打开本地文件，不提示存在，直接覆盖 */
    int fd = open(file,O_RDONLY);
    if(fd < 0)
    {
        return 0;
    }
    
    /* 发出上传指令 */
    const char* name = strrchr(file,'/');
    name = name != NULL ? name + 1 : file;
    if(
        ftpTryWrite(csock,"STOR ",5) <= 0 ||
        ftpTryWrite(csock,name,strlen(name)) <= 0 ||
        ftpTryWrite(csock,"\r\n",2) <= 0
    )
    {
        return 0;
    }
    
    /* 检查服务器应答 150开始传输 */
    char buffer[1024];
    ssize_t len = ftpTryRead(csock,buffer,1024);
    if(len < 3 || strncmp(buffer,"150",3))
    {
        return 0;
    }
    
    /* 上传文件数据 */
    do
    {
        len = read(fd,buffer,1024);
        if(ftpTryWrite(dsock,buffer,len) < len)
        {
            return 0;
        }
    }while(len>0);
    close(fd);    // 关闭本地文件
    close(dsock); // 关闭数据连接
    
    /* 检查服务器应答 226传输完成 */
    len = ftpTryRead(csock,buffer,1024);
    if(len < 3 || strncmp(buffer,"226",3))
    {
        return 0;
    }
    
    return 1;
}

