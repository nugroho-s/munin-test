#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int size = 2048;
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	    char buffer[size];

	    if (argc < 3) {
	       fprintf(stderr,"usage %s hostname port\n", argv[0]);
	       exit(0);
	    }
	    portno = atoi(argv[2]);
	    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	    if (sockfd < 0) 
		error("ERROR opening socket");
	    server = gethostbyname(argv[1]);
	    if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	    }
	    bzero((char *) &serv_addr, sizeof(serv_addr));
	    serv_addr.sin_family = AF_INET;
	    bcopy((char *)server->h_addr, 
		 (char *)&serv_addr.sin_addr.s_addr,
		 server->h_length);
	    serv_addr.sin_port = htons(portno);
	    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		error("ERROR connecting");
	char* x;
	while (strcmp(x,"quit")!=0){
	    x="";
	    bzero(buffer,size);
	    n = read(sockfd,buffer,size-1);
	    if (n < 0) 
		 error("ERROR reading from socket");
	    printf("%s\n",buffer);
	    printf("Please enter the message: ");
	    bzero(buffer,size);
	    x = fgets(buffer,size-1,stdin);
	    ///printf("%s",*x);
	    n = write(sockfd,buffer,strlen(buffer));
	    if (n < 0) 
		 error("ERROR writing to socket");
	}
    close(sockfd);
    return 0;
}
