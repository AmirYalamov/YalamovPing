// Written by Amir Yalamov, on April 14th, 2020

// program that implements the ping command in C++
// compile with "g++ ping.cpp" and run "./a.out"

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <args.hxx> // CLI library for C++

#define DEFDATALEN  (64 - ICMP_MINLEN)
#define MAXIPLEN    60
#define MAXICMPLEN  76
#define MAXPACKET   (65536 - 60 - ICMP_MINLEN)

uint16_t in_cksum (uint16_t *addr, unsigned len) {

    uint16_t final = 0;

    uint32_t sum = 0;

    while (len > 1) {

        sum += *addr++;
        len -= 2;
    }

    if (len == 1) {

        *(unsigned char *)&final = *(unsigned char *)addr;
        sum += final;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    final = ~sum;
    return final;
}

uint16_t in_cksum (uint16_t *addr, unsigned len);

// funciton to perform the ping linux Command
int ping (std::string address) {

    printf("beginning of ping\n");

    // declaration of variables and structs
    int s, i, cc, packLength, dataLength = DEFDATALEN;

    struct hostent *hp;
    struct sockaddr_in to, from;
    struct ip *ip;

    u_char *packet, outpack[MAXPACKET];
    char hnamebuf[MAXHOSTNAMELEN];
    std::string hostname;

    struct icmp *icp;

    int ret, fromlen, hlen;

    fd_set rfds;

    struct timeval timeVal;
    int retVal;
    struct timeval start, end;
    int endTime;

    bool cont = true;

    to.sin_family = AF_INET;

    to.sin_addr.s_addr = inet_addr (address.c_str());

    if (to.sin_addr.s_addr != (u_int) - 1) {
        printf("if (to.sin_addr.s_addr != (u_int) - 1)\n");

        hostname = address;
    }

    else {

        hp = gethostbyname (address.c_str());

        if (!hp) {

            std::cerr << "Unknown host " << address << std::endl;
            return -1;
        }

        to.sin_family = hp->h_addrtype;

        bcopy (hp->h_addr, (caddr_t)&to.sin_addr, hp->h_length);

        strncpy (hnamebuf, hp->h_name, sizeof(hnamebuf) - 1);
        hostname = hnamebuf;
    }

    packLength = dataLength + MAXIPLEN + MAXICMPLEN;

    if ((packet = (u_char *)malloc((u_int)packLength)) == NULL) {

        std::cerr << "Memory allocation error.\n";
        return -1;
    }

    if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        // needs to run as sudo
        return -1;
    }

    icp = (struct icmp *) outpack;
    icp->icmp_type = ICMP_ECHO;
    icp->icmp_code = 0;
    icp->icmp_cksum = 0;
    icp->icmp_seq = 12345;
    icp->icmp_id = getpid();

    cc = dataLength + ICMP_MINLEN;
    icp->icmp_cksum = in_cksum((unsigned short *)icp, cc);

    gettimeofday (&start, NULL);

    i = sendto (s, (char *)outpack, cc, 0, (struct sockaddr*)&to, (socklen_t)sizeof(struct sockaddr_in));

    if (i < 1 || i != cc) {

        if (i < 0) {

            perror ("sendto error.\n");
        }

        std::cout << "wrote " << hostname << " " << cc << " chars, ret = " << i << std::endl;
    }

    FD_ZERO (&rfds);
    FD_SET (s, &rfds);
    timeVal.tv_sec = 1;
    timeVal.tv_usec = 0;

    while (cont) {

        retVal = select (s + 1, &rfds, NULL, NULL, &timeVal);

        if (retVal == -1) {

            perror("select()\n");
            return -1;
        }
        else if (retVal) {

            fromlen = sizeof (sockaddr_in);

            if ((ret = recvfrom (s, (char *)packet, packLength, 0, (struct sockaddr *)&from, (socklen_t *)&fromlen)) < 0) {

                perror ("recvfrom error.\n");
                return -1;
            }

            // chekc IP header
            ip = (struct ip *)((char *)packet);
            hlen = sizeof (struct ip);

            if (ret < (hlen + ICMP_MINLEN)) {

                std::cerr << "Packet was too short (" << ret << " bytes) from " << hostname << std::endl;
                return -1;
            }

            // ICMP Section
            icp = (struct icmp *)(packet + hlen);

            if (icp->icmp_type == ICMP_ECHOREPLY) {

                // output icmp sequence
                if (icp->icmp_seq != 12345) {

                    std::cout << "Received sequence #" << icp->icmp_seq << std::endl;
                    continue;
                }
                // output icmp id
                if (icp->icmp_id != getpid()) {

                    std::cout << "Received id " << icp->icmp_id << std::endl;
                    continue;
                }

                cont = false;
            }

            else {

                std::cout << "Recv: not an echo reply." << std::endl;
                continue;
            }

            gettimeofday (&end, NULL);
            endTime = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);

            if (endTime < 1) {

                endTime = 1;

                std::cout << "Elapsed time: " << endTime << " usec" << std::endl;
            }

            return endTime;

        }

        else {

            std::cout << "No data within one second.\n";
            return 0;
        }
    }
    printf("end of ping\n");

    return 0;
}


// function that will keep sending ICMP "echo requests" in an infinite loop until user quits CLI
// argument 'address' is in the form of either a hostname or an IP address
// void requestLoop (std::string address) {
//
//     // keep sending out requests until user enters Control+C
//     while (1) {
//
//         return ping(address);
//     }
// }

int main (int argc, char *argv[]) {

    // int sockfd;
    // char *ipAddress, *reverseHostname;
    // // struct sockAddressIn addressCon;
    // char netBuffer [NI_MAXHOST];

    // 2nd positional terminal argument should be the DNS address of a website
    //ipAddress = dnsLookup (argv[1], &addressCon);   // TODO: implement dnsLookup function

    // // if no address is provided for DNS address argument
    // if (ipAddress == NULL) {
    //
    //     printf("\nCould not resolve hostname, DNS lookup failed.\n");
    //
    //     return 0;
    // }



    // instantiating objects and characteristics for a parser to  create a CLI
    args::ArgumentParser parser ("Ping Command Line Interface");
    args::HelpFlag help (parser, "help", "display help menu", {'h'});
    args::CompletionFlag completion (parser, {"complete"});
    args::Positional<std::string> address(parser, "address", "The hostname or IP Address");

    try {

        parser.ParseCLI(argc, argv);
    }
    catch (const args::Completion& e) {

        std::cout << e.what();
    }
    catch (const args::Help&) {

        std::cout << parser;
        return 0;
    }
    catch (const args::ParseError& e) {

        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    // here is where the program calls the code to  sned ICMP "echo requests" in an infinite loop
    if (address) {
        std::cout << "address: " << args::get(address) << std::endl;
        return ping(get(address).c_str());
    }


    return 0;
}
