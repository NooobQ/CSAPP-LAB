#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400


/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *connection_hdr = "Connection: close\r\n";
static const char *proxy_hdr = "Proxy-Connection: close\r\n";

void parse_url(char *url, char *host, char *port, char *path);
void doit(int fd);

int main(int argc, char** argv)
{
    int listenfd, connfd, port;
    struct sockaddr_storage clientaddr;
    socklen_t clientlen;

    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    while(1){
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        doit(connfd);
        Close(connfd);
    }
    printf("%s", user_agent_hdr);
    return 0;
}

//parse url to host:port & path
void parse_url(char *url, char *host, char *port, char *path){
    if(sscanf(url, "http://%99[^:]:%99[^/]%99[^\n]", host, port, path) != 3){
		printf("Port undetected.\n");
        // *port = 80;
        strcpy(port, "80");
    	sscanf(url, "http://%99[^/]%99[^\n]", host, path);
	}
    printf("Parsed request Host: %s:%s Path: %s\n", host, port, path);
    return ;
}

void doit(int fd){
    rio_t rio, s_rio;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE],
        host[MAXLINE], path[MAXLINE], port[MAXLINE];
    // int port;


    //Read HTTP Request
    Rio_readinitb(&rio, fd);
    if (!Rio_readlineb(&rio, buf, MAXLINE))  //line:netp:doit:readrequest
        return;
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);       //line:netp:doit:parserequest
    if (strcasecmp(method, "GET")) {                     //line:netp:doit:beginrequesterr
        printf("Error: Method Not Implement.\n");
        return ;
    }
    // parse_url(uri, host, &port, path);
    parse_url(uri, host, port, path);

    Rio_readlineb(&rio, buf, MAXLINE);
    printf("%s", buf);
    while(strcmp(buf, "\r\n")) {          //line:netp:readhdrs:checkterm
        Rio_readlineb(&rio, buf, MAXLINE);
        printf("%s", buf);
    }

    //Proxy
    struct hostent *hptr;
    int server_connfd;
    // char port_string[MAXLINE];
    if((hptr = Gethostbyname(host))==NULL){
        return;
    }
    if((server_connfd = Open_clientfd(host, port)) <= 0){
        return;
    }
    printf("Info: Remote server detected.\n");

    Rio_readinitb(&s_rio, server_connfd);

    //module it to a function.
    sprintf(buf, "%s %s HTTP/1.0\r\n", method, path);
    Rio_writen(server_connfd, buf, strlen(buf));
    // Rio_writen(server_connfd, method, strlen(method));
    // Rio_writen(server_connfd, " ", 1);
    // Rio_writen(server_connfd, path, strlen(path));
    // Rio_writen(server_connfd, " HTTP/1.0\r\n", 11);
    Rio_writen(server_connfd, user_agent_hdr, strlen(user_agent_hdr));
    sprintf(buf, "Host: %s\r\n", host);
    Rio_writen(server_connfd, buf, strlen(buf));
    Rio_writen(server_connfd, proxy_hdr, strlen(proxy_hdr));
    Rio_writen(server_connfd, connection_hdr, strlen(connection_hdr));
    Rio_writen(server_connfd, "\r\n", 2);
    //TO-DO: Other addition request additional request headers

    printf("Info: sending request ended.\n");

    while(Rio_readlineb(&s_rio, buf, MAXLINE) > 0){
        Rio_writen(fd, buf, strlen(buf));
        printf("%s", buf);
    }

    printf("Info: Received Response.\n");

    Close(server_connfd);
}
