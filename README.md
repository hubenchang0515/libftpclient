# libftpclient
A library of ftp client on Linux

## Demo
![](https://raw.githubusercontent.com/hubenchang0515/libftpclient/master/demo.png)

## APIs
```C
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
ssize_t ftpTryRead(int fd,void* buffer,size_t bytes);



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
ssize_t ftpTryWrite(int fd,const void* buffer,size_t bytes);



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
int ftpConnect(const char* ipv4,uint16_t port);



/*
 * TODO
 *  Close
 *
 * PARAM
 *  sock     : socket of ftp control connection
 *
 */
void ftpClose(int sock);



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
int ftpCreateControlConnection(const char* ipv4,uint16_t port);



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
int ftpCreateDataConnection(int sock);



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
int ftpLogin(int sock,const char* username,const char* password);



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
int ftpChangeDirectory(int sock,const char* path);



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
int ftpDownload(int csock,int dsock,const char* file);



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
int ftpUpload(int csock,int dsock,const char* file);
```
