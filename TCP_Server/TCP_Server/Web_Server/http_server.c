//
//  http_server.c
//  TCP_Server
//
//  Created by Yash soni on 28/07/23.
//

#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/_select.h>
#include <errno.h>
#include <arpa/inet.h>
#include <memory.h>
#include <unistd.h>
#define SERVER_PORT 2000

char data_buffer[1024];

typedef struct student_
{
    char name[32];
    unsigned int roll;
    char dept[32];
    char hobby[32];
} student_t;

student_t students[5] = {
    {"Tom", 11, "Milk", "Running"},
    {"Jerry", 22, "Cheese", "Teasing"},
    {"Pikachu", 33, "Pokemon", "Lighting"},
    {"Ash", 44, "Fighting", "Catching_Pokemon"},
    {"Noddy", 55, "Solving", "Driving_Car"}
};


void string_space_trim(char *string)
{
    if(!string)
        return;
    char *ptr = string;
    int len = (int)strlen(ptr);
    if(!len)
        return;
    if(!isspace(ptr[0]) && !isspace(ptr[len - 1]))
        return;
    while (len-1 > 0 && isspace(ptr[len-1])) {
        ptr[--len] = 0;
    }
    while (*ptr && isspace(*ptr)) {
        ++ptr;
        --len;
    }
    
    memmove(string, ptr, len+1);
}

static char * process_GET_request(char *URL, unsigned int *response_len)
{
    printf("%s:(%u) called with URL = %s \n", __FUNCTION__, __LINE__, URL);
    char del[2] = {'?', '\0'};
    string_space_trim(URL);
    
    char *token[5] = {0};
    token[0] = strtok(URL, del);
    token[1] = strtok(0, del);
    del[0] = '&';
    token[2] = strtok(token[1], del);
    token[3] = strtok(0, del);
    
    printf("token[0] = %s, token[1] = %s, token[2] = %s, token[3] = %s\n",
           token[0], token[1], token[2], token[3]);
    
    del[0] = '=';
    char *roll_no_str = strtok(token[3], del);
    char *roll_no_val = strtok(0, del);
    printf("Roll no value = %s\n", roll_no_val);
    unsigned int roll_no = atoi(roll_no_val),
    i = 0;
    for(i=0; i < 5; ++i) {
        if(students[i].roll != roll_no)
            continue;
        break;
    }
    
    if(i == 5)
        return NULL;
    
    char *response = calloc(1, 1024);
    
    strcpy(response,
           "<html>"
           "<head>"
                "<title>HTML Response </title>"
                "<style>"
                "table, th, td {"
                    "border: 1px solid black;}"
                "</style>"
            "</head>"
            "<body"
            "<table>"
           "<tr>"
           "<td>");
    
    strcat(response, students[i].name);
    strcat(response,
           "</td></tr>");
    strcat(response,
           "</table>"
           "</body>"
           "</html>");
    unsigned int content_len_str = (uint)strlen(response);
    char *header = calloc(1, 248 + content_len_str);
    strcpy(header, "HTTP/1.1 200 OK\n");
    strcpy(header, "Server: My Personal HTTP Server\n");
    strcpy(header, "Connection: close\n");
    strcat(header, "Content-Type: text/html; charset=UTF-8\n");
    strcpy(header, "Content-Length: 2048\n");
    strcat(header, response);
    content_len_str = (uint)strlen(header);
    *response_len = content_len_str;
    free(response);
    return header;
}

static char*
process_POST_request(char *URL, unsigned int* response_len)
{
    return NULL;
}

void setup_tcp_server_communication(void)
{
    int master_socket_fd = 0,
        sent_recv_bytes = 0,
        addr_len = 0,
        opt = 1,
        comm_socket_fd = 0;
    
    struct sockaddr_in server_addr,
    client_addr;
    fd_set readfds;
    
    if((master_socket_fd = socket(AF_INET,
                                  SOCK_STREAM,
                                  IPPROTO_TCP)) == -1)
    {
        printf("Master socket creation failed\n");
        exit(EXIT_FAILURE);
    }
    
    if(setsockopt(master_socket_fd, SOL_SOCKET,
                                    SO_REUSEADDR,
                                    (char *)&opt, sizeof(opt)) < 0)
    {
        printf("TCP socket creation failed for multiple connections\n");
        exit(EXIT_FAILURE);
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    addr_len = sizeof(struct sockaddr);
    
    if(bind(master_socket_fd,
            (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("Socket binding failed\n");
        exit(EXIT_FAILURE);
    }
    
    if(listen(master_socket_fd, 5) < 0)
    {
        printf("Listening failed\n");
        return;
    }
    
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(master_socket_fd, &readfds);
        
        printf("\nBlocked on select sysytem call...\n");
        
        select(master_socket_fd + 1, &readfds, NULL, NULL, NULL);
        
        if(FD_ISSET(master_socket_fd, &readfds))
        {
            printf("New connection request recvd, accept the connection.\
                   Client and Server completes TCP-3 way handshake at this point\n");
            
            comm_socket_fd = accept(master_socket_fd,
                                    (struct sockaddr *) &client_addr,
                                    (uint *)&addr_len);
            
            if(comm_socket_fd < 0)
            {
                printf("accept error : errno = %d\n", errno);
                exit(0);
            }
            
            printf("Connection accepted from client : %s : %u \n",
                   inet_ntoa(client_addr.sin_addr),
                   ntohs(client_addr.sin_port));
            
            while (1) {
                printf("Server ready to serve the client\n");
                memset(data_buffer, 0, sizeof(data_buffer));
                sent_recv_bytes = (int)recvfrom(comm_socket_fd,
                                                (char *)data_buffer,
                                                sizeof(data_buffer),
                                                0,
                                                (struct sockaddr *)&client_addr,
                                                (uint *)&addr_len);
                printf("Server recvd %d bytes from client %s : %u\n",
                       sent_recv_bytes,
                       inet_ntoa(client_addr.sin_addr),
                       ntohs(client_addr.sin_port));
                if(sent_recv_bytes == 0)
                {
                    close(comm_socket_fd);
                    break;
                }
                
                
                
                
                printf("Msg received : %s \n", data_buffer);
                char *request_line = NULL;
                char del[2] = "\n",
                *method = NULL,
                *URL = NULL;
                
                request_line = strtok(data_buffer, del);
                del[0] = ' ';
                method = strtok(request_line, del);
                URL = strtok(NULL, del);
                printf("Method = %s\n", method);
                printf("URL = %s\n", URL);
                char *response = NULL;
                unsigned int response_length = 0;
                
                if(strcmp(method, "GET") == 0) {
                    response = process_GET_request(URL, &response_length);
                } else if(strcmp(method, "POST") == 0) {
                    response = process_POST_request(URL, &response_length);
                } else {
                    printf("Unsupported URL method request\n");
                    close(comm_socket_fd);
                    break;
                }
                
                if(response) {
                    printf("Response to be sent to client = \n%s", response);
                    sent_recv_bytes = (int)sendto(comm_socket_fd, response,
                                                  response_length,
                                                  0,
                                                  (struct sockaddr *)&client_addr,
                                                  sizeof(struct sockaddr));
                    free(response);
                    printf("Server sent %d bytes to the client\n ",
                           sent_recv_bytes);
                }
            }
        }
    }
}
int main(int argc, const char * argt[])
{
    setup_tcp_server_communication();
    return 0;
}
