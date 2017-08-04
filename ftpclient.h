/*
 * File     : ftpclient.h
 * Author   : blog.kurukurumi.com
 *
 */


#ifndef FTP_CLIENT_H
#define FTP_CLIENT_H

void ftpClearSocket(int sock);

ssize_t ftpTryRead(int fd,void* buffer,size_t bytes);

ssize_t ftpTryWrite(int fd,const void* buffer,size_t bytes);

int ftpConnect(const char* ipv4,uint16_t port);

void ftpClose(int sock);

int ftpCreateControlConnection(const char* ipv4,uint16_t port);

int ftpCreateDataConnection(int sock);

int ftpPort(int csock);

int ftpAccept(int socklisten);

int ftpLogin(int sock,const char* username,const char* password);

int ftpChangeDirectory(int sock,const char* path);

int ftpTellDownload(int csock,const char* file);

int ftpTellUpload(int csock,const char* file);

int ftpDownload(int dsock,const char* file);

int ftpUpload(int dsock,const char* file);

#endif


