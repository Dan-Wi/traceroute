#include "icmp_receive.h"
#include <sys/select.h> 
#include <stddef.h>  
#include <sys/time.h> 
#include <netinet/ip.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netinet/ip_icmp.h> 
#include <string.h> 
#include <errno.h>
#include <stdio.h>

#define ICMP_HEADER_TEXCEEDED_SIZE 8

int select_next_packet(int sockfd, struct timeval* tv);
int receive_all(int sockfd, int id, int seq, int n, response buf[n]);
int is_valid(struct icmp* icmp_header, int id, int seq);

int get_responses(int sockfd, int id, int seq, int n, response buf[n]) {
    struct timeval wait_time = {MAX_WAIT_TIME_SEC, MAX_WAIT_TIME_USEC};
    int valid_responses = 0;
    
    while(wait_time.tv_usec > 0 && valid_responses < n) {
        int ready = select_next_packet(sockfd, &wait_time);

        if(ready < 0) {
            fprintf(stderr, "select() error: %s\n", strerror(errno));
            break;
        } else if(ready == 0) {
            break;
        } else {
            int count = receive_all(
                sockfd, id, seq, n-valid_responses, buf+valid_responses);
            valid_responses += count;
        }
    }

    return valid_responses;
}

int select_next_packet(int sockfd, struct timeval* tv) {
    fd_set descriptors;
    FD_ZERO(&descriptors);
    FD_SET(sockfd, &descriptors);
    return select(sockfd+1, &descriptors, NULL, NULL, tv);    
}

int receive_all(int sockfd, int id, int seq, int n, response buf[n]) {
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    uint8_t buffer[IP_MAXPACKET];
    int count = 0;
    ssize_t packet_len;

    while((packet_len = recvfrom(sockfd, buffer, IP_MAXPACKET, MSG_DONTWAIT, 
    (struct sockaddr*) &sender, &sender_len)) >= 0 && count < n) {
        struct ip* ip_header = (struct ip*) buffer;
        uint8_t* icmp_packet = buffer + 4 * ip_header->ip_hl;
        struct icmp* icmp_header = (struct icmp*) icmp_packet;

        if(is_valid(icmp_header, id, seq)) {
            gettimeofday(&buf[count].received_at, NULL);
            buf[count].response_type = icmp_header->icmp_type;
            inet_ntop(AF_INET, &(sender.sin_addr), buf[count].sender_ip_str, 
                sizeof(buf[count].sender_ip_str));
            ++count;
        }
    }

    if(packet_len < 0 && errno != EWOULDBLOCK) {
        fprintf(stderr, "recvfrom() error: %s\n", strerror(errno));
    }
    return count;
}

int is_valid(struct icmp* icmp_header, int id, int seq) {
    if(icmp_header->icmp_type == ICMP_TIME_EXCEEDED) {
        uint8_t* ptr1 = ((uint8_t*) icmp_header) + ICMP_HEADER_TEXCEEDED_SIZE;
        uint8_t ip_hdr_len = 4*((*(ptr1)) & 0x0f);
        icmp_header = (struct icmp*) (ptr1 + ip_hdr_len);
    }

    return icmp_header->icmp_id == (uint16_t) id && 
        icmp_header->icmp_seq == (uint16_t) seq;
}
