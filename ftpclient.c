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
 *  Clear socket
 *
 * PARAM
 *  sock : socket
 *
 */
void ftpClearSocket(int sock)
{
    char buffer[256];
    do
    {
        struct timeval timeout; // not block
        timeout.tv_sec = 0;
        timeout.tv_usec = 10000; 
        
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(sock,&fds);
        
        switch(select(sock+1,&fds,NULL,NULL,&timeout))
        {
            case -1: return ; // error
            
            case 0 : return ;  // timeout : means cleared
            
            default:{ ssize_t len = read(sock,buffer,256); write(STDOUT_FILENO,buffer,len);}
        }
    }while(1);
}





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
    int csock = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in ftpAddr;
    ftpAddr.sin_family = AF_INET;
    ftpAddr.sin_addr.s_addr = inet_addr(ipv4);
    ftpAddr.sin_port = htons(port);
    if(connect(csock,(struct sockaddr*)&ftpAddr,sizeof(ftpAddr)))
    {
        return -1;
    }
    else
    {
        return csock;
    }
}





/*
 * TODO
 *  Close socket
 *
 * PARAM
 *  csock     : socket of ftp control connection
 *
 */
void ftpClose(int csock)
{
    ftpTryWrite(csock,"QUIT\r\n",6);
    close(csock);
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
    int csock = ftpConnect(ipv4,port);
    if(csock != -1)
    {  
        char buffer[128];
        ftpTryRead(csock,buffer,128);
        return csock;
    }
    else
    {
        return -1;
    }
}





/*
 * TODO
 *  Create FTP data connection by PASV (passive mode)
 *
 * PARAM
 *  csock     : socket of ftp control connection
 *
 * RETURN
 *  success : socket of ftp data connection
 *  failed  : -1
 */
int ftpCreateDataConnection(int csock)
{

    /* send PASV */
    if(ftpTryWrite(csock,"PASV\r\n",6) <= 0)
    {
        return -1;
    }
    
    /* read ip and port */
    char buffer[128];
    ssize_t len = ftpTryRead(csock,buffer,128);
    if(len <= 0)
    {
        return -1;
    }
    
    //write(STDOUT_FILENO,buffer,len);
    uint16_t ip0,ip1,ip2,ip3,port0,port1;
    char* fp = strstr(buffer,"(");
    sscanf(fp,"(%hu,%hu,%hu,%hu,%hu,%hu",&ip0,&ip1,&ip2,&ip3,&port0,&port1);
    char ip[32];
    sprintf(ip,"%hu.%hu.%hu.%hu",ip0,ip1,ip2,ip3);
    uint16_t port = port0*256 + port1;
    return ftpConnect(ip,port);
}




/*
 * TODO
 *  Create FTP data connection by PORT (active mode)
 *
 * PARAM
 *  csock : socket of ftp control connection
 *
 * RETURN
 *  success : socket lstenning to ftp data connection
 *  failed  : -1
 */
int ftpPort(int csock)
{
    /* get ipv4 address by csock */
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    if(getsockname(csock,(struct sockaddr*)&addr,&addrlen) == -1)
    {
        return -1;
    }
    uint16_t ip0,ip1,ip2,ip3;
    sscanf(inet_ntoa(addr.sin_addr),"%hu.%hu.%hu.%hu",&ip0,&ip1,&ip2,&ip3);
    
    /* create a socket to listen */
    int socklisten = socket(AF_INET,SOCK_STREAM,0);
    if(socklisten == -1)
    {
        return -1;
    }
    
    /* bind port */
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    if(bind(socklisten,(struct sockaddr*)&addr,sizeof(addr)) == -1)
    {
        close(socklisten);
        return -1;
    }
    
    /* get port by socklisten */
    uint16_t port0,port1;
    addrlen = sizeof(addr);
    if(getsockname(socklisten,(struct sockaddr*)&addr,&addrlen) == -1)
    {
        close(socklisten);
        return -1;
    }
    port0 = ntohs(addr.sin_port) / 256;
    port1 = ntohs(addr.sin_port) % 256;
    
    /* listen */
    if(listen(socklisten,10) == -1)
    {
        close(socklisten);
        return -1;
    }
    
    /* send port command */
    char portcmd[64];
    sprintf(portcmd,"PORT %hu,%hu,%hu,%hu,%hu,%hu\r\n",ip0,ip1,ip2,ip3,port0,port1);
    if(ftpTryWrite(csock,portcmd,strlen(portcmd)) <= 0)
    {
        close(socklisten);
        return -1;
    }
    
    return socklisten;
}




/*
 * TODO
 *  Accept data connection from socklisten
 *
 * PARAM
 *  socklisten : socket listening to data connection
 *
 * RETURN
 *  success : socket of ftp data connection
 *  failed  : -1
 */
int ftpAccept(int socklisten)
{
    return accept(socklisten,NULL,NULL);
}




/*
 * TODO
 *  Login
 *
 * PARAM
 *  csock     : socket of ftp control connection
 *  username : username
 *  password : password
 *
 *
 * RETURN
 *  success : 1
 *  failed  : 0
 */
int ftpLogin(int csock,const char* username,const char* password)
{
    char buffer[128];
    
    /* send username */
    if(
        ftpTryWrite(csock,"USER ",5) <= 0 ||
        ftpTryWrite(csock,username,strlen(username)) <= 0 ||
        ftpTryWrite(csock,"\r\n",2) <= 0
      )
    {
        return 0;
    }
    
    ssize_t len = ftpTryRead(csock,buffer,128);
    if(len <= 0)
    {
        return 0;
    }
    
    /* send password */
    if(
        ftpTryWrite(csock,"PASS ",5) <= 0 ||
        ftpTryWrite(csock,password,strlen(password)) <= 0 ||
        ftpTryWrite(csock,"\r\n",2) <= 0
      )
    {
        return 0;
    }
    
    len = ftpTryRead(csock,buffer,128);
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
 *  csock : socket of ftp control connection
 *  path : path to change to
 *
 * RETURN
 *  succedd : 1
 *  failed  : 0
 *
 */
int ftpChangeDirectory(int csock,const char* path)
{
    if(
        ftpTryWrite(csock,"CWD ",4) <= 0 ||
        ftpTryWrite(csock,path,strlen(path)) <=0 ||
        ftpTryWrite(csock,"\r\n",2) <= 0
    )
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
 *  Tell server to download file
 *
 * PARAM
 *  csock   : socket of ftp control connection
 *  file    : file path in server
 *
 * RETURN
 *  success : 1
 *  failed  : 0
 */
int ftpTellDownload(int csock,const char* file)
{
    /* seend download command RETR <file> */
    if(
        ftpTryWrite(csock,"RETR ",5) <= 0 ||
        ftpTryWrite(csock,file,strlen(file)) <= 0 ||
        ftpTryWrite(csock,"\r\n",2) <= 0
    )
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
 *  Tell server to upload file
 *
 * PARAM
 *  csock   : socket of ftp control connection
 *  file    : file path in server
 *
 * RETURN
 *  success : 1
 *  failed  : 0
 */
int ftpTellUpload(int csock,const char* file)
{ 
    /* send upload command STOR <file> */
    if(
        ftpTryWrite(csock,"STOR ",5) <= 0 ||
        ftpTryWrite(csock,file,strlen(file)) <= 0 ||
        ftpTryWrite(csock,"\r\n",2) <= 0
    )
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
 *  dsock   : socket of ftp data connection
 *  file    : file path in local
 *
 * RETURN
 *  success : 1
 *  failed  : 0
 */
int ftpDownload(int dsock,const char* file)
{
    int fd = open(file,O_WRONLY|O_CREAT,0755);
    if(fd < 0)
    {
        return 0;
    }
    
    ssize_t len;
    char buffer[1024];
    do
    {
        len = ftpTryRead(dsock,buffer,1024);
        write(fd,buffer,len);
    }while(len > 0);
    close(fd);
    
    return 1;
}



/*
 * TODO
 *  Up file
 * 
  * PARAM
 *  dsock   : socket of ftp data connection
 *  file    : file path in local
 *
 * RETURN
 *  success : 1
 *  failed  : 0
 */
int ftpUpload(int dsock,const char* file)
{
    int fd = open(file,O_RDONLY,0755);
    if(fd < 0)
    {
        return 0;
    }
    
    ssize_t len;
    char buffer[1024];
    do
    {
        len = read(fd,buffer,1024);
        ftpTryWrite(dsock,buffer,len);
    }while(len > 0);
    close(fd);
    
    return 1;
}
