#ifndef INCLUDE_ICMP_RECEIVE_H
#define INCLUDE_ICMP_RECEIVE_H
#include <stdint.h> 
#include <bits/types/struct_timeval.h>

#define MAX_WAIT_TIME_USEC 1000000L
#define MAX_WAIT_TIME_SEC 0

typedef struct {
    struct timeval received_at;
    uint16_t response_type;
    char sender_ip_str[20];
} response;


/*
*   @param buf A container to save valid responses
*   @return Number of valid responses saved in buf
*/
int get_responses(int sockfd, int id, int seq, int n, response buf[n]);

#endif