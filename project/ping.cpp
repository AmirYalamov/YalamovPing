// program that implements the ping command in C++
// compile with "g++ ping.cpp" and run "./a.out"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <iostream>

int main (int argc, char *argv[]) {

    int sockfd;
    char *ipAddress, *reverseHostname;
    struct sockAddressIn addressCon;
    char netBuffer [NI_MAXHOST];

    // 2nd positional terminal argument should be the DNS address of a website
    ipAddress = dnsLookup (argv[1], &addressCon);   // TODO: implement dnsLookup function

    // if no address is provided for DNS address argument
    if (ipAddress == NULL) {

        printf("\nCould not resolve hostname, DNS lookup failed.\n");

        return 0;
    }



    return 0;
}
