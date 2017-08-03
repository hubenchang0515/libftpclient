/*
 * File     : ftpclient.h
 * Author   : blog.kurukurumi.com
 *
 */


#ifndef FTP_CLIENT_H
#define FTP_CLIENT_H

ssize_t ftpTryRead(int fd,void* buffer,size_t bytes);
ssize_t ftpTryWrite(int fd,const void* buffer,size_t bytes);
void ftpClose(int sock);
int ftpConnect(const char* ipv4,uint16_t port);
int ftpCreateControlConnection(const char* ipv4,uint16_t port);
int ftpCreateDataConnection(int sock);
int ftpLogin(int sock,const char* username,const char* password);
int ftpChangeDirectory(int sock,const char* path);
int ftpDownload(int csock,int dsock,const char* file);
int ftpUpload(int csock,int dsock,const char* file);

#endif


