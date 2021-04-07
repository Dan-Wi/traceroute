#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h> 
#include <unistd.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <errno.h> 
#include <netinet/ip_icmp.h> 
#include <arpa/inet.h> 

#include "icmp_send.h"
#include "icmp_receive.h"
#define PACKET_COUNT 3
#define TTL_UPPER_LIMIT 31
#define USEC_IN_MS 1000L
#define MS_IN_SEC 1000L

void print_unique(int n, response arr[n]);
void print_time(int count, long total_wait_time);

int main(int argc, char* argv[]) {
    if(argc != 2) {
        fprintf(stderr, "Usage: sudo ./traceroute ip_address\n");
        return EXIT_FAILURE;
    }

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0) {
		fprintf(stderr, "socket error: %s\n", strerror(errno)); 
		return EXIT_FAILURE;
	}

    struct sockaddr_in recipient;
    bzero(&recipient, sizeof(recipient));
    recipient.sin_family = AF_INET;
    if(!inet_pton(AF_INET, argv[1], &recipient.sin_addr)) {
        fprintf(stderr, "Invalid ip number\n");
        return EXIT_FAILURE;
    }

    int pid = getpid();
    int ttl = 1;
    int reply_received = 0;
    struct timeval tv;
    response responses[PACKET_COUNT];
    while(ttl < TTL_UPPER_LIMIT && !reply_received) {
        if(send_packets(sockfd, recipient, pid, ttl, PACKET_COUNT)) {
            fprintf(stderr, "Failed to send packets\n");
            return EXIT_FAILURE;
        }
        
        long total_wait_time_ms = 0;
        gettimeofday(&tv, NULL);
        int count = get_responses(sockfd, pid, ttl, PACKET_COUNT, responses);

        for(int i=0; i < count; ++i) {
            if(responses[i].response_type == ICMP_ECHOREPLY) {
                reply_received = 1;
            }

            struct timeval diff;
            timersub(&responses[i].received_at, &tv, &diff);
            total_wait_time_ms += diff.tv_sec * MS_IN_SEC + diff.tv_usec / USEC_IN_MS;
        }

        printf("%d. ", ttl);
        print_unique(count, responses);
        print_time(count, total_wait_time_ms);
        ++ttl;
    }

    close(sockfd);
    return EXIT_SUCCESS;
}

void print_unique(int n, response arr[n]) {
    for(int i = 0; i < n; ++i) {
        int unique = 1;
        int j = i+1;
        while(j < n && (unique = strcmp(arr[i].sender_ip_str, arr[j].sender_ip_str))) {
            ++j;
        }

        if(unique) {
            printf("%s ", arr[i].sender_ip_str);
        }
    }
}

void print_time(int count, long total_wait_time) {
    if(count == 0) {
        printf("*\n");
    } else if(count == PACKET_COUNT){
        printf("%ldms\n", total_wait_time / count);
    } else {
        printf("???\n");
    }
}
