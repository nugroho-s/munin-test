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

void send_msg(char* msg, int socket);
//mengirim mesage ke suatu socket

int compare_hostnames(char hostname[]);
//mengecek apakah suatu string terdapat di array hostname,
//mengembalikan 1 jika ada, 0 jika tidak

char* temp_host_name;

void print_nodes();
//menuliskan semua node yang terhubung

int j_node = 0;

void get_mem();
//menyalin data yang didapat dari system call free

void do_the_things(int socket);
//prosedur yang dilakukan setiap child fork

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
	bzero(simpan,512);
	char* dest, src;
	int pid;
	int j_fork = 0;

	int sockfd, portno;
	socklen_t clilen;
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
	do{
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) 
			error("ERROR on accept");
		pid = fork();
		if (pid < 0)
			error("ERROR on fork");
		else if (pid == 0){
			//client process
			close(sockfd);
			do_the_things(newsockfd);
			exit(0);
		} else {
			close(newsockfd);
			j_fork--;
		}
		j_fork++;
	} while (1);
	
	return 0; 
}

void send_msg(char* msg, int socket){
	write(socket,msg,strlen(msg));
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

void do_the_things(int socket){
	int n;
	char tulis[100];
	int gai_result;
	char buffer[512];
	char hostname[512];
	hostname[511]='\0';
	char * cmd;
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

	for(p=info;p!=NULL;p=p->ai_next){
		temp_host_name = p->ai_canonname;
	}

	sprintf(tulis,"# munin node at %s\n",temp_host_name);
	send_msg(tulis,socket);
	bzero(tulis,100);
	do {
		bzero(buffer,512);
		bzero(tulis, 100); 
		n = read(socket,buffer,511);
		cmd = strtok(buffer," ");
		if (n < 0) error("ERROR reading from socket");
		//printf("%s\n",buffer);
		if (strcmp(cmd,"cap\n")==0){
			send_msg("cap multigraph dirtyconfig\n",socket);
		} else if (strcmp(cmd,"nodes\n")==0){
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

			for(p=info;p!=NULL;p=p->ai_next){
				temp_host_name = p->ai_canonname;
				sprintf(tulis,"%s\n",temp_host_name);
				send_msg(tulis,socket);
				bzero(tulis,100);
			}
			send_msg(".\n",socket);
		} else if (strcmp(cmd,"list")==0) {
			cmd = strtok(NULL," ");
			if (compare_hostnames(cmd)==1){
				send_msg("memory\n",socket);
			}
		} else if (strcmp(cmd,"version\n")==0){
			sprintf(tulis,"aaa node on %s version: 8.48\n",temp_host_name);
			send_msg(tulis,socket);
		} else if (strcmp(cmd,"fetch") == 0){
			cmd = strtok(NULL," ");
			if (strcmp(cmd,"memory\n")==0){
				get_mem();
				sprintf(tulis,"used.value %sfree.value %s",mem_used,mem_free);
				send_msg(tulis,socket);
			}
		} else if (strcmp(buffer,"config")==0){
			cmd = strtok(NULL," ");
			if (strcmp(cmd,"memory\n")==0){
				get_mem();
				sprintf(tulis,"graph_args --base 1024 -l 0 --upper-limit %s",mem_total);
				send_msg(tulis,socket);
				send_msg("graph_vlabel Bytes\n",socket);
				send_msg("graph_title Memory usage\n",socket);
				send_msg("graph_category system\n",socket);
				send_msg("graph_info This graph shows this machine memory.\n",socket);
				send_msg("graph_order used free\n",socket);
				send_msg("used.label used\n",socket);
				send_msg("used.draw STACK\n",socket);
				send_msg("used.info Used memory.\n", socket);
				send_msg("free.label free\n", socket);
				send_msg("free.draw STACK\n", socket);
				send_msg("free.info Free memory.\n", socket);
				send_msg(".\n",socket);
			}
		} else if (strncmp(cmd,"quit\n",5)!=0) {
			send_msg("# Unknown command. Try cap, list, nodes, config, fetch, version or quit\n",socket);
		}
		if (n < 0) error("ERROR writing to socket");
	} while (strncmp(cmd,"quit\n", 5)!=0);
}