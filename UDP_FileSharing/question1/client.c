#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>

void error(const char * msg){
    perror(msg); 
    exit(1);
}

int main(int argc, char *argv[]){
    // CLI ARGUMENTS :- ./client server_IP_Address, server_port_number
    if(argc <3){
        fprintf(stderr, "Not all arguments given"); 
        exit(0); 
    }
    
    int portno = atoi(argv[2]); 

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd <0){
        error("Error opening socket"); 
    }

    // It retrives host(server) information from the domain which is provided at 2nd argument in Command line argument    
    // hostent contains host domain name, host IP addres etc. Under the hood it utlises DNS Resolution
    struct hostent * server = gethostbyname(argv[1]); 
    if(server==NULL) fprintf(stderr, "No host exists"); 
   
    
    struct sockaddr_in serverAddress;  
    memset(&serverAddress, 0, sizeof(serverAddress)); 
    serverAddress.sin_addr.s_addr = AF_INET; 
    
    // bcopy(memory src, memory destination, number of bytes to copy)
    bcopy((char *) server->h_addr, (char *) &serverAddress.sin_addr.s_addr, server->h_length); 
    serverAddress.sin_port = htons(portno); 

    struct pollfd pollinfo; 
    pollinfo.fd = sockfd; 
    pollinfo.events = POLL_IN; 

    char *PING = "Client is demanding for date and time";
    char buffer[255]; 
    int flag = 0; 
    socklen_t serverLen = sizeof(serverAddress); 
    for(int i=0; i<5; i++){
        printf("%s\n", "Trying to connect with the server"); 
        int n = sendto(sockfd, PING, strlen(PING),0,(struct sockaddr *) &serverAddress, serverLen); 
        if(n<0) error("Error in sending PING"); 

        n = poll(&pollinfo, 1, 3000); 
        if(n<0) error("Poll failed"); 
        else if(n==0){
            printf("No response from the server, again trying...\n"); 
        }
        else{
            memset(buffer, 0, sizeof(buffer)); 
            // n = sendto(sockfd, 1, sizeof(int), 0, (struct sockaddr *) &serverAddress, &serverLen); 
            n = recvfrom(sockfd, buffer, sizeof(buffer), 0,(struct sockaddr *) &serverAddress, &serverLen); 
            if(n<0) error("recieving failed"); 
            printf("Server: %s", buffer);
            flag=1; 
            break;
        }
    }
    if(!flag){
        error("Time Out Error"); 
    }

    close(sockfd); 
    return 0; 

}