#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
   int sockfd;
   int len;
   struct sockaddr_in address;
   int result;
   char buff[1024] = "GET index.html\n";

   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   address.sin_family = AF_INET;
   address.sin_addr.s_addr = inet_addr("127.0.0.1");
   address.sin_port = htons(9735);
   len = sizeof(address);
   result = connect(sockfd, (struct sockaddr *)&address, len);

   if (result == -1)
   {
      perror("oops: client1");
      exit(1);
   }
   //write(sockfd, buff, 1);
   //read(sockfd, buff, 1);
   send(sockfd, buff, strlen(buff), 0);

   //sleep(1);

   recv(sockfd, buff, sizeof(buff), 0);
   printf("string from server = %s\n", buff);
   close(sockfd);
   exit(0);
}
