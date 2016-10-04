/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

struct addrinfo hints, *info, *p;

int newsockfd;

void send_msg(char* msg);

int compare_hostnames(char hostname[]);

char* temp_host_name;

void print_nodes();

int j_node = 0;

void get_mem();

FILE* in;
extern FILE *popen();
char buff[512];
char simpan[512];
char mem_total[10];
char mem_free[10];
char mem_used[10];
char * pch;

int main(int argc, char *argv[])
{
	int gai_result;
	char tulis[100];
	char* dest, src;

	char hostname[512];
	hostname[511]='\0';
	gethostname(hostname, 512);
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_CANONNAME;

	if((gai_result = getaddrinfo(hostname, "http", &hints, &info)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai_result));
		exit(0);
	}

	for(p=info;p!=NULL;p=p->ai_next){
		temp_host_name = p->ai_canonname;
		j_node++;
	}

	int sockfd, portno;
	socklen_t clilen;
	char buffer[1024];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		error("ERROR on binding");
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if (newsockfd < 0) 
		error("ERROR on accept");

	sprintf(tulis,"# munin node at %s\n",temp_host_name);
	send_msg(tulis);
	bzero(tulis,100);
	do {
		bzero(buffer,1024);
		bzero(tulis, 100); 

		n = read(newsockfd,buffer,1023);
		if (n < 0) error("ERROR reading from socket");
		//printf("%s\n",buffer);
		if (strcmp(buffer,"cap\n")==0){
			send_msg("cap multigraph dirtyconfig\n");
		} else if (strcmp(buffer,"nodes\n")==0){
			for(p=info;p!=NULL;p=p->ai_next){
				temp_host_name = p->ai_canonname;
				sprintf(tulis,"%s\n",temp_host_name);
				send_msg(tulis);
				bzero(tulis,100);
			}
			send_msg(".\n");
		} else if (compare_hostnames(buffer)==1) {
			send_msg("memory\n");
		} else if (strcmp(buffer,"version\n")==0){
			sprintf(tulis,"aaa node on %s version: 8.48\n",temp_host_name);
			send_msg(tulis);
		} else if (strcmp(buffer,"fetch memory\n") == 0){
			get_mem();
			sprintf(tulis,"used.value %sfree.value %s",mem_used,mem_free);
			send_msg(tulis);
			bzero(tulis,100);
		} else if (strcmp(buffer,"config memory\n")==0){
			send_msg("graph_args --base 1024 -l 0 --upper-limit");
		} else if (strncmp(buffer,"quit\n",5)!=0) {
			send_msg("# Unknown command. Try cap, list, nodes, config, fetch, version or quit\n");
		}
		if (n < 0) error("ERROR writing to socket");
	} while (strncmp(buffer,"quit\n", 5)!=0);
	close(newsockfd);
	close(sockfd);
	return 0; 
}

void send_msg(char* msg){
	write(newsockfd,msg,strlen(msg));
}


int compare_hostnames(char * hostname) {
	int i;
	char temp[100];
	memset(temp, '\0', sizeof(temp));
	strcpy(temp, temp_host_name);
	strcat(temp, "\n");
	for (i=0; i<j_node; i++) {
		if (strcmp(hostname, temp)==0) {
			return 1;
		}
	}
	return 0;
}

void get_mem(){
	if(!(in = popen("free","r"))){
		exit(1);
	}
	int i=0;
	memset(simpan,0,sizeof(simpan));

	fgets(buff, sizeof(buff), in);
	fgets(buff, sizeof(buff), in);
	pch = strtok(buff," ");
	pch = strtok(NULL," ");
	sprintf(mem_total,"%s\n",pch);
	pch = strtok(NULL," ");
	sprintf(mem_used,"%s\n",pch);
	pch = strtok(NULL," ");
	sprintf(mem_free,"%s\n",pch);
}
