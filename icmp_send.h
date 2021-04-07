#ifndef INCLUDE_ICMP_SEND_H
#define INCLUDE_ICMP_SEND_H
#include <netinet/in.h> 

/*
*   @param ttl Will also be used as icd_seq
*   @return 0 on success, -1 for errors
*/
int send_packets(int sockfd, struct sockaddr_in recipient, int id, int ttl, int n);

#endif