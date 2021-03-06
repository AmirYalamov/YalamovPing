// Written by Amir Yalamov, from April 14th - April 17th, 2020

// program that implements the ping command in C++
// compile with "g++ ping.cpp -o ping" and run "sudo ./ping hostname.com/IP Address"
// run "sudo ./ping -h/--help" to show a full list of all the possible functionality:
/*
OPTIONS:

      -h, --help                        display help menu
      address                           The hostname or IP Address
      -t[Time To Live(TTL)]             Set the IP Time to Live
      --ipv6=[IPv6 Support On(y/n)]     Use IP version 6to support new addresses
      -i[interval seconds]              Wait an interval amount of seconds
                                        between sending each packet for
                                        optimization
      -p[number of packets]             Specify how many packets should be sent
                                        during the ping
      "--" can be used to terminate flag options and force all following
      arguments to be treated as positional options
 */

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
#include <iomanip>
#include <math.h>

#define DEFDATALEN  (64 - ICMP_MINLEN)
#define MAXIPLEN    60
#define MAXICMPLEN  76
#define MAXPACKET   (65536 - 60 - ICMP_MINLEN)

// -------------- function for calculating the checksum of a packet --------------
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

// global variables to keep track of packets sent or lost
int packCounter = 0;
int lostCounter = 0;

// -------------- funciton to perform the ping linux Command --------------
int ping (std::string address, int timeToLive, char internetProtocol6, double newInterval, int packetsToSend) {

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
    float endTime;
    bool cont = true;
    to.sin_family = AF_INET;
    to.sin_addr.s_addr = inet_addr (address.c_str());

    // if given address is in IP form, set the hostname as the given address in an argument of the ping command
    if (to.sin_addr.s_addr != (u_int) - 1) {

        hostname = address;
    }

    // if given address is in a DNS form
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

    bool ipv6 = false;
    // if ipv6 is requested from the ping command
    if (internetProtocol6 == 'y') {
        ipv6 = true;

        to.sin_family = AF_INET6;
        s = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMP);

        if ((s = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMP)) < 0) {
            // needs to run as sudo
            return -1;
        }
    }

    else if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        // needs to run as sudo
        return -1;
    }

    // default value for TTL is 255 seconds, unless a value is specified from the command ping
    int ttlVal = 255;
    if (timeToLive > 0) {
        ttlVal = timeToLive;
    }

    // set socket options to account for TTL argument
    setsockopt(s, SOL_IP, IP_TTL, &ttlVal, sizeof(ttlVal));

    icp = (struct icmp *) outpack;
    icp->icmp_type = ICMP_ECHO;
    icp->icmp_code = 0;
    icp->icmp_cksum = 0;
    icp->icmp_seq = 12345;
    icp->icmp_id = getpid();
    cc = dataLength + ICMP_MINLEN;
    icp->icmp_cksum = in_cksum((unsigned short *)icp, cc);

    /* ********** timer starts ********** */
    gettimeofday (&start, NULL);
    /* ********** ECHO_REQUEST SENT ********** */
    i = sendto (s, (char *)outpack, cc, 0, (struct sockaddr*)&to, (socklen_t)sizeof(struct sockaddr_in));


    if (i < 1 || i != cc) {

        if (i < 0) {

            perror ("sendto error.\n");
        }

        std::cout << "wrote " << hostname << " " << cc << " chars, ret = " << i << std::endl;
    }

    FD_ZERO (&rfds);
    FD_SET (s, &rfds);

    // if -i argument specified in ping, set new itnerval
    if (newInterval > 0) {
        timeVal.tv_sec = floor(newInterval);            // set the seconds
        timeVal.tv_usec = std::fmod(newInterval, 1) * 1000000;    // set the microseconds
    }
    else{
        timeVal.tv_sec = 1;     // default time interval that requests wait is 1 seconds
        timeVal.tv_usec = 0;
    }

    while (cont) {

        // checks how many packets have been sent if the -p argument was passed
        if (packetsToSend > 0 && packCounter >= packetsToSend) {
            return -1;
        }

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

            // check IP header
            ip = (struct ip *)((char *)packet);
            hlen = sizeof( struct ip );

            if (ret < (hlen + ICMP_MINLEN)) {
                std::cerr << "Packet was too short (" << ret << " bytes) from " << hostname << std::endl;
                return -1;
            }

            // ICMP Section
            icp = (struct icmp *)(packet + hlen);

            std::cout << "Receiving from " << hostname << ", ";

            // if reply is recieved from pinged address
            if (icp->icmp_type == ICMP_ECHOREPLY) {



                // this section is for Round Trip Delay Time (RTT) calculations
                gettimeofday (&end, NULL);  /* ********** timer ends ********** */

                // calculate time between ECHO_REQUEST and ECHO_REPLY in terms of milliseconds
                endTime = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
                // convert from usec to ms and round the value to 2 decimal places
                endTime =  floorf((endTime / 1000) * 100) / 100;

                // output RTT info
                std::cout << lostCounter << " total packet(s) lost, ";
                std::cout << "ICMP Sequence = " << (1 + packCounter + lostCounter) << ", ";
                std::cout << "Time To Live(TTL) = " << ttlVal << ", ";
                std::cout << "Round Trip Time(RTT) = " << endTime << " ms" << std::endl;

                packCounter ++;	// + 1 transmitted packet
                continue;
            }

            // packet is lost
            else {
                lostCounter ++; // + 1 lost packet

                std::cout << "1 packet lost, ";
                std::cout << lostCounter << " total packet(s) lost, ";
                std::cout << "ICMP Sequence = " << (1 + packCounter + lostCounter) << ", ";
                std::cout << "Time to Live(TTL) exceeded" << std::endl;

                continue;
            }

        }

        cont = false;
    }

    return 0;
}

int main (int argc, char *argv[]) {

    // using the Args library to implement a C++ CLI: https://github.com/taywee/args
    // instantiating objects and characteristics for a parser to  create a CLI
    args::ArgumentParser parser ("Ping Command Line Interface");
    args::HelpFlag help (parser, "help", "Display help menu", {'h', "help"});
    args::CompletionFlag completion (parser, {"complete"});
    args::Positional<std::string> address (parser, "address", "The hostname or IP Address");
    args::ValueFlag<int> timeToLive (parser, "Time To Live(TTL)", "Set the IP Time to Live", {'t'});
    args::ValueFlag<char> ipv6 (parser, "IPv6 Support On(y/n)", "Use IP version 6 to support new addresses", {"ipv6"});
    args::ValueFlag<double> newInterval (parser, "interval seconds", "Wait an interval amount of seconds between sending each packet for optimization", {'i'});
    args::ValueFlag<int> packetsToSend (parser, "number of packets", "Specify how many packets should be sent during the ping", {'p'});


    // error handling for the CLI library Args
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
    catch (args::ValidationError e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    // here is where the program calls the code to  sned ICMP "echo requests" in an infinite loop
    if (address) {

        std::cout << "ping " << args::get(address) << "...\n";

        while (ping) {
            if (ping(get(address).c_str(), get(timeToLive), get(ipv6), get(newInterval), get(packetsToSend)) == -1) {
                break;
            }
        }
    }

    return 0;
}
