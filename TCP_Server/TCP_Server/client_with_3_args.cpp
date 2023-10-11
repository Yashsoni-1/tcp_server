#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>


void set_up_tcp_client(char *server_ip, uint16_t server_port)
{
    struct sockaddr_in server_det;
    int addr_len = sizeof(sockaddr_in);
    int sock_fd {}, ret{};

    
    sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if(sock_fd == -1)
    {
        std::cerr << "Socket creation failed\n";
        exit(1);
    }
    
    server_det.sin_family = AF_INET;
    server_det.sin_port = server_port;
    
    hostent *dest = (hostent *)gethostbyname(server_ip);
    server_det.sin_addr = *((in_addr *)dest->h_addr);
    
    ret = connect(AF_INET, (struct sockaddr *)&server_det, addr_len);
    
    if(ret == -1)
    {
        std::cerr << "Couldn't connect to server\n";
        exit(1);
    } else {
        std::cout << "Connected\n";
    }

    return;    
}
int main(int argc, const char * argv[]) {
    
    if(argc != 3)
    {
        std::cerr << "Please enter <server ip> <server port> \n";
        exit(1);
    }
    
    set_up_tcp_client(argv[1], atoi(argv[2]));
    
    
    return 0;
}
