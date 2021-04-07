#include "icmp_send.h"
#include <stdint.h> 
#include <netinet/ip_icmp.h> 
#include <netinet/ip.h> 
#include <arpa/inet.h> 
#include <strings.h> 
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>


struct icmp create_header(int id, int ttl);
uint16_t compute_icmp_checksum (const void *buff, int length);
int send_packet(int sockfd, struct sockaddr_in recipient, struct icmp header);

int send_packets(int sockfd, struct sockaddr_in recipient, int id, int ttl, int n) {
    for(int i = 0; i < n; ++i) {
        struct icmp header = create_header(id, ttl);

        if(setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int))) {
            fprintf(stderr, "sendto error: %s\n", strerror(errno));
            return -1;
        }

        if(send_packet(sockfd, recipient, header)) {
            return -1;
        }
    }
    return 0;
}

struct icmp create_header(int id, int ttl) {
    struct icmp header;
    header.icmp_type = ICMP_ECHO;
    header.icmp_code = 0;
    header.icmp_hun.ih_idseq.icd_id  = (uint16_t) id;
    header.icmp_hun.ih_idseq.icd_seq = (uint16_t) ttl;
    header.icmp_cksum = 0;
    header.icmp_cksum = compute_icmp_checksum ((uint16_t*) &header, sizeof(header));

    return header;
}

uint16_t compute_icmp_checksum (const void *buff, int length) {
	uint32_t sum;
	const uint16_t* ptr = buff;
	assert(length % 2 == 0);
	for (sum = 0; length > 0; length -= 2)
		sum += *ptr++;
	sum = (sum >> 16) + (sum & 0xffff);
	return (uint16_t)(~(sum + (sum >> 16)));
}


int send_packet(int sockfd, struct sockaddr_in recipient, struct icmp header) {
    ssize_t bytes_sent = sendto(sockfd, &header, sizeof(header), 0,
        (struct sockaddr*) &recipient, sizeof(recipient));
    
    if(bytes_sent < 0) {
        fprintf(stderr, "sendto error: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}


