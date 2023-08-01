//
//  client_server.c
//  TCP_Server
//
//  Created by Yash soni on 27/07/23.
//

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "common.h"

#define DEST_PORT 2000
#define SERVER_IP_ADDRESS "127.0.0.1"

test_struct_t client_data;
result_struct_t result;

void setup_tcp_communication(void)
{
    int sockfd = 0,
        sent_recv_bytes = 0,
        addr_len = 0;
    
    addr_len = sizeof(struct sockaddr);
    
    struct sockaddr_in dest;
    
    dest.sin_family = AF_INET;
    dest.sin_port = DEST_PORT;
    
    struct hostent *host = (struct hostent *)gethostbyname(SERVER_IP_ADDRESS);
    dest.sin_addr = *((struct in_addr *)host->h_addr);
    
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    connect(sockfd, (struct sockaddr *)&dest, sizeof(struct sockaddr));
    
PROMT_USER:
    
    printf("Enter a: ?\n");
    scanf("%u", &client_data.a);
    printf("Enter b: ?\n");
    scanf("%u", &client_data.b);
    
    sent_recv_bytes = (int)sendto(sockfd, &client_data, sizeof(test_struct_t), 0, (struct sockaddr *)&dest, sizeof(struct sockaddr));
    
    printf("No. of bytes sent = %d\n", sent_recv_bytes);
    sent_recv_bytes = (int)recvfrom(sockfd, (char *)&result, sizeof(result_struct_t), 0, (struct sockaddr *)&dest, (unsigned int *)&addr_len);
    
    printf("No. of bytes received = %d\n", sent_recv_bytes);
    
    printf("result received = %u\n", result.c);
    goto PROMT_USER;
}

int main(int argc, const char * argv[]) {
    
    setup_tcp_communication();
    printf("Exiting the application...\n");
    
    return 0;
}
