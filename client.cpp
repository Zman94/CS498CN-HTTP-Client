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
#include <vector>
#include <map>
#include <sstream>
using namespace std;

#define PORT "3490" // the port client will be connecting to

#define MAXDATASIZE 100 // max number of bytes we can get at once

size_t split(const string &txt, vector<string> &strs, char ch)
{
    size_t pos = txt.find(ch);
    size_t initialPos = 0;
    strs.clear();

    // Decompose statement
    while (pos != std::string::npos)
    {
        strs.push_back(txt.substr(initialPos, pos - initialPos));
        initialPos = pos + 1;
        pos = txt.find(ch, initialPos);
    }

    // Add the last one
    strs.push_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1));

    return strs.size();
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    std::ofstream myfile;

    if (argc != 2)
    {
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    char protocol[100];
    char ip[100];
    char auth[100];
    char port[10] = "80";
    char page[8192];
    /* Get the parts of the input url */
    sscanf(argv[1], "%99[^:]://%99[^/]/%8192[^\n]", protocol, auth, page);
    sscanf(auth, "%99[^:]:%99[^\n]", ip, port);

    /* Test if http protocol */
    int is_prot_http = strcmp("http", protocol);
    if (is_prot_http > 0)
    {
        myfile.open("output");
        myfile.write("INVALIDPROTOCOL", 15);
        myfile.close();
        return 1;
    }

    if ((rv = getaddrinfo(ip, port, &hints, &servinfo)) != 0)
    {
        myfile.open("output");
        myfile.write("NOCONNECTION", 12);
        myfile.close();
        return 1;
    }

    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        myfile.open("output");
        myfile.write("NOCONNECTION", 12);
        myfile.close();
        return 2;
    }
    string get_request = "GET " + string(page) + " HTTP/1.0\r\n\r\n";
    const char *get_request_type = get_request.c_str();

    int sent_bytes = send(sockfd, get_request_type, strlen(get_request_type), 0);
    int recv_length = 1;
    int length = 0;
    char buffer[8192];
    bool parseHeader = true;
    myfile.open("output");
    while (recv_length > 0)
    {
        recv_length = recv(sockfd, buffer, 8192, 0);
        if (recv_length == 0)
        {
            break;
        }

        int firstLine = true;
        if (parseHeader)
        {
            parseHeader = false;
            string buffer_str = string(buffer);
            vector<string> response;

            split(buffer_str, response, ' ');
            int response_code;
            sscanf(response[1].c_str(), "%d", &response_code);

            if (response_code == 404)
            {
                myfile.write("FILENOTFOUND", 12);
                myfile.close();
                return 2;
            }
            else if (response_code == 301)
            {
                myfile.write("REDIRECTING", 11);
                myfile.close();
                return 2;
            }

            const char *target = "\r\n\r\n";
            char *eoheader = buffer;
            eoheader = strstr(buffer, target);
            eoheader = eoheader + 4; //Remove target
            length = recv_length - (eoheader - buffer);
            int file_name = string(page).length();
            if (strcmp(page, eoheader + length - file_name) == 0)
            {
                myfile.write(eoheader, length - file_name);
            }
            else
            {
                myfile.write(eoheader, length);
            }
            memset(buffer, 0, sizeof(buffer));
        }
        else
        {
            length = recv_length;
            int file_name = string(page).length();
            if (strcmp(page, buffer + length - file_name) == 0)
            {
                myfile.write(buffer, length - file_name);
            }
            else
            {
                myfile.write(buffer, length);
            }
            memset(buffer, 0, sizeof(buffer));
        }
    }
    myfile.close();
    freeaddrinfo(servinfo); // all done with this structure
    close(sockfd);

    return 0;
}
