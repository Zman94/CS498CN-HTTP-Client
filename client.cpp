/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <iostream>
#include <fstream>
using namespace std;

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	char protocol[100];
	char ip[100];
	char auth[100];
    char port[10] = "80";
    char page[100];
	unsigned char buffer[4096];
	/* Get the parts of the input url */
	sscanf(argv[1], "%99[^:]://%99[^/]/%99[^\n]", protocol, auth, page);
	sscanf(auth, "%99[^:]:%99[^\n]", ip, port);
	cout << auth << "\nprotocol: " << protocol << "\nip: " << ip << "\nport: " << port << "\npage: " << page << "\n\n";

	/* Test if http protocol */
	int is_prot_http = strcmp("http", protocol);
	if(is_prot_http > 0) {
		ofstream myfile;
		myfile.open("output");
		myfile << "INVALIDPROTOCOL";
		myfile.close();
		return 1;
	}

	if ((rv = getaddrinfo(ip, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		ofstream myfile;
		myfile.open("output");
		myfile << "NOCONNECTION";
		myfile.close();
		return 2;
	}

	send(sockfd, "HEAD / HTTP/1.0\r\n\r\n", 23, 0);
	int recv_length = 1;
	recv_length = recv(sockfd, &buffer, 1024, 0);
	cout << recv_length << "\n";
	cout << "buffer: " << buffer << "\n\n";
	char tmp[100];
	int response_code;
	sscanf(buffer+8, "%d", response_code);
	cout << response_code << "\n";
	while(recv_length > 0){
		printf("The web server is %s\n", buffer+8);
		freeaddrinfo(servinfo);
		return 0;
	} 

	freeaddrinfo(servinfo); // all done with this structure
	close(sockfd);

	return 0;
}