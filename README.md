# libftpclient
A library of ftp client on Linux

## Demo
![](https://raw.githubusercontent.com/hubenchang0515/libftpclient/master/demo.png)

## PORT Example
```C
int socklisten = ftpPort(csock);
ftpTellDownload(csock,"./dir/file.txt");
int dsock = ftpAccept(socklisten);
ftpDownload(dsock,"file.txt");
```

## PASV Example
```C
int dsock = ftpCreateDataConnection(csock);
ftpTellDownload(csock,"./dir/file.txt");
ftpDownload(dsock,"file.txt");
```

## APIs
```C
/*
 * TODO
 *  Clear socket
 *
 * PARAM
 *  sock : socket
 *
 */
void ftpClearSocket(int sock);




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
 *  Close socket
 *
 * PARAM
 *  csock     : socket of ftp control connection
 *
 */
void ftpClose(int csock);





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
 *  Create FTP data connection by PASV (passive mode)
 *
 * PARAM
 *  csock     : socket of ftp control connection
 *
 * RETURN
 *  success : socket of ftp data connection
 *  failed  : -1
 */
int ftpCreateDataConnection(int csock);




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
int ftpPort(int csock);




/*
 * TODO
 *  Accept data connection from socklisten
 *
 * PARAM
 *  socklisten : socket listening to data connection
 *
 * RETURN
 *  success : socket of ftp data connection(return value of ftpPort)
 *  failed  : -1
 */
int ftpAccept(int socklisten);




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
int ftpLogin(int csock,const char* username,const char* password);





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
int ftpChangeDirectory(int csock,const char* path);




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
int ftpTellDownload(int csock,const char* file);





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
int ftpTellUpload(int csock,const char* file);





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
int ftpDownload(int dsock,const char* file);



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
int ftpUpload(int dsock,const char* file);
```
