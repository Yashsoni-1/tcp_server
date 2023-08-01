//
//  mupx_tcp_server.c
//  TCP_Server
//
//  Created by Yash soni on 27/07/23.
//

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/_select.h>
#include <arpa/inet.h>
#include <errno.h>
#include "common.h"

#define MAX_CLIENT_SUPPORTED 32
#define SERVER_PORT 2000

test_struct_t test_struct;
result_struct_t result;
char data_buffer[1024];
int monitored_fd_set[32];

static void initialize_monitor_fd_set(void)
{
    int i=0;
    for(; i < MAX_CLIENT_SUPPORTED; ++i)
        monitored_fd_set[i] = -1;
}

static void re_init_readfds(fd_set* fd_set_ptr)
{
    FD_ZERO(fd_set_ptr);
    int i = 0;
    for(; i < MAX_CLIENT_SUPPORTED; ++i) {
        if(monitored_fd_set[i] != -1) {
            FD_SET(monitored_fd_set[i], fd_set_ptr);
        }
    }
}

static void add_to_monitored_fd_set(int skt_fd)
{
    int i = 0;
    for(; i < MAX_CLIENT_SUPPORTED; ++i) {
        if(monitored_fd_set[i] != -1)
            continue;
        monitored_fd_set[i] = skt_fd;
        break;
    }
}

static void remove_from_monitored_set(int to_be_removed)
{
    for(int i=0; i < MAX_CLIENT_SUPPORTED; ++i)
        if (monitored_fd_set[i] == to_be_removed) {
            monitored_fd_set[i] = -1;
            break;
        }
}

static int get_max_fd(void)
{
    int i = 0;
    int max = -1;
    for(; i < MAX_CLIENT_SUPPORTED; ++i)
        if(monitored_fd_set[i] > max)
            max = monitored_fd_set[i];
    
    return max;
}

void setup_tcp_server_communication(void)
{
    int master_socket_fd = 0,
        sent_recv_bytes = 0,
        addr_len = 0,
        opt = 1;
    
    int comm_socket_fd = 0;
    fd_set readfds;
    
    struct sockaddr_in server_addr,
    client_addr;
    
    initialize_monitor_fd_set();
    
    if((master_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        printf("socket creation failed\n");
        exit(0);
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = SERVER_PORT;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    addr_len = sizeof(struct sockaddr);
    
    if(bind(master_socket_fd, (struct sockaddr *)&server_addr,
            sizeof(struct sockaddr)) == -1)
    {
        printf("Socket binding failed\n");
        return;
    }
    
    if(listen(master_socket_fd, 5) < 0)
    {
        printf("listen failed\n");
        return;
    }
    
    add_to_monitored_fd_set(master_socket_fd);
    
    while (1) {
        
        re_init_readfds(&readfds);
        
        printf("Blocked on system call...\n");
        
        select(get_max_fd() + 1, &readfds, NULL, NULL, NULL);
        
        if(FD_ISSET(master_socket_fd, &readfds))
        {
            printf("New connection request recvd, accept the connection.\
                   Client and Server completes TCP-3 way handshake at this point\n");
            
            comm_socket_fd = accept(master_socket_fd,
                                    (struct sockaddr *)&client_addr,
                                    (unsigned int *)&addr_len);
            
            if(comm_socket_fd < 0) {
                printf("accept error : errno = %d\n", errno);
                exit(0);
            }
            
            add_to_monitored_fd_set(comm_socket_fd);
            printf("Connection accepted from client : %s:%u:\n",
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        }
        else
        {
            int i=0, comm_socket_fd=-1;
            for (; i < MAX_CLIENT_SUPPORTED; ++i) {
                if(FD_ISSET(monitored_fd_set[i], &readfds)) {
                    comm_socket_fd = monitored_fd_set[i];
                    printf("Server ready to serve the client\n");
                    memset(data_buffer, 0, sizeof(data_buffer));
                    sent_recv_bytes = (int)recvfrom(comm_socket_fd,
                                                    (char *)data_buffer,
                                                    sizeof(data_buffer), 0,
                                                    (struct sockaddr*) &client_addr,
                                                    (uint *)&addr_len);
                    
                    printf("Received %d bytes from client %s : %d \n", sent_recv_bytes,
                           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                    
                    if(sent_recv_bytes == 0) {
                        close(comm_socket_fd);
                        remove_from_monitored_set(comm_socket_fd);
                        break;
                    }
                    
                    test_struct_t *client_data = (test_struct_t *)data_buffer;
                    
                    if(client_data->a == 0 && client_data->b == 0) {
                        close(comm_socket_fd);
                        remove_from_monitored_set(comm_socket_fd);
                        break;
                    }
                    
                    result_struct_t result;
                    result.c = client_data->a + client_data->b;
                    
                    sent_recv_bytes = (int)sendto(comm_socket_fd, (char *) &result,
                                                  sizeof(result_struct_t), 0,
                                                  (struct sockaddr *)&client_addr,
                                                  sizeof(struct sockaddr));
                    printf("Sent %d bytes to the client", sent_recv_bytes);
                }
            }
        }
    }
    
}

int main(int argc, const char *argv[]) {
    setup_tcp_server_communication();
    return 0;
}
