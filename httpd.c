#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h>
#include <pthread.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define URL_BUF_SIZE 1024
#define METHOD_BUF_SIZE 16

static const char POST[] = "POST";
static const char GET[] = "GET";
//static const char *POST = "POST";
//static const char *GET = "GET";

static const char SERVER_STRING[] = "Server: jdbhttpd/0.1.0\r\n";
static const char INDEX_HTML[] = "index.html";

void error_die(const char *sc)
{
   perror(sc);
   exit(1);
}

int startup(in_port_t *port){                                            //create
   int httpd = socket(PF_INET, SOCK_STREAM, 0);
   if (httpd == -1){
	   error_die("socket");
   }

   struct sockaddr_in name;                                         
   memset(&name, 0, sizeof(name));
   name.sin_family = AF_INET;
   name.sin_port = htons(*port);
   name.sin_addr.s_addr = htonl(INADDR_ANY);
   if (bind(httpd, (struct sockaddr *)&name, sizeof(name)) < 0){        //bind
      error_die("bind");
   }
   printf("bind on %s\n", inet_ntoa(name.sin_addr));

   if (*port == 0){
      socklen_t len = sizeof(name);
      if(getsockname(httpd, (struct sockaddr *)&name, &len) == -1){
         error_die("getsockname");
      }
      *port = ntohs(name.sin_port);
   }

   if (listen(httpd, 5) < 0){                                           //listen
      error_die("listen");
   }

   return httpd;
}

int get_line(int socket, char *buf, int size){                          //recev
    int i = 0;
    char c = '\0';

    while ((i < size - 1) && (c != '\n')){
        int n = recv(socket, &c, 1, 0);
        if(n > 0){
            if (c == '\r'){
                n = recv(socket, &c, 1, MSG_PEEK);
                if ((n > 0) && (c == '\n')){
                    recv(socket, &c, 1, 0);
                }
                else{
                    c = '\n';
                }
            }
            buf[i] = c;
            i ++;
        }
        else{
            c = '\n';
        }
    }
    buf[i] = '\0';

    return i;
}

void *accept_request(void *args)
{
    char buf[URL_BUF_SIZE];
    int client_socket = *((int*)args);

    int numchars = get_line(client_socket, buf, sizeof(buf));

#ifdef DEBUG
    printf("(%s,%d)received %s. %d", __FUNCTION__,__LINE__,buf, numchars);
#endif

    send(client_socket, buf, strlen(buf), 0);                      //send 

    close(client_socket);                                          //close

    return 0;
}

int main(){
	
   //in_port_t port = 0;
   in_port_t port = 9735;

   int server_socket = startup(&port);                  
   printf("httpd running on port %d\n", port);

   while(1){
      struct sockaddr_in client_name;
      socklen_t len = sizeof(client_name);
      int client_socket = accept(server_socket,                              //accept
                                 (struct sockaddr *)&client_name,
                                 &len);
      if(client_socket == -1){
         error_die("accept");
      }

      pthread_t newthread;

      if (pthread_create(&newthread, NULL, 
                         accept_request, (void*)&client_socket) != 0){
         perror("pthread_create");
      }
   }

   close(server_socket);

   return 0;
}
