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
    // CLI ARGUMENTS :- ./client, server_port_number
    if(argc <2){
        fprintf(stderr, "Not all arguments given"); 
        exit(0); 
    }
    
    int portno = atoi(argv[1]); 

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd <0){
        error("Error opening socket"); 
    }

    // It retrives host(server) information from the domain which is provided at 2nd argument in Command line argument    
    // hostent contains host domain name, host IP addres etc. Under the hood it utlises DNS Resolution
    
    struct sockaddr_in serverAddress;  
    memset(&serverAddress, 0, sizeof(serverAddress)); 
    serverAddress.sin_addr.s_addr = AF_INET; 
    inet_aton("127.0.0.1", &serverAddress.sin_addr);
    serverAddress.sin_port = htons(portno); 
   
    // let's connect out client socket to the server socker 
    if(connect(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) <0){
        error("Connection failed"); 
    }
    char buffer[255]; 

    // write "date?" to get date and time
    // write "exit" to exit; 
    printf("Write 'date?' to get date/time and 'exit' to exit\n"); 
    while(1){
        memset(buffer, 0, sizeof(buffer)); 
        char ch = '1'; 
        int i=0; 
        while(ch!='\n'){
            scanf("%c", &ch); 
            buffer[i++] = ch; 
        }
        buffer[strcspn(buffer, "\n")] = '\0'; 
        if(strncmp(buffer, "exit", 4)==0){
            printf("You have chosen to exit\n"); 
            break; 
        }
        int n = send(sockfd, buffer, strlen(buffer),0); 
        if(n<0) error("Error in sending data");
        memset(buffer, 0, sizeof(buffer));
        n = recv(sockfd, buffer, sizeof(buffer), 0); 
        if(n<0) error("Error in recieving data");   
        printf("Current date and time: %s\n", buffer); 
    }

    close(sockfd); 
    return 0; 

}