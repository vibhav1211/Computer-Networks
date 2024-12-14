#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <arpa/inet.h>


void error(const char* msg){
    perror(msg);
    exit(1);
}

int main(int argc,  char *argv[]){
    // CLI ARGUMENTS :- ./server, server1_port_number, server2_port_number
    // printf("Number of arguments are: %d\n", argc); 
    // for(int i=0; i<argc; i++){
    //     printf("%d argument is: %s\n", i, argv[i]); 
    // }

    if(argc <3){
        fprintf(stderr, "ALL Ports are not provided"); 
        exit(1); 
    }

    int server1_portno = atoi(argv[1]), server2_portno = atoi(argv[2]); 

    // Creating a new socket
    int sockfd1 = socket(AF_INET, SOCK_STREAM, 0); 
    if(sockfd1 < 0){
        error("Socket couldn't be opened"); 
    }
    int sockfd2 = socket(AF_INET, SOCK_STREAM, 0); 
    if(sockfd2 < 0){
        error("Socket couldn't be opened"); 
    }
    

    // Crearing server and client side addresses.
    struct sockaddr_in serverAddress1, serverAddress2, lbAddress; 

    // intialising with zero 
    memset(&serverAddress1, 0, sizeof(serverAddress1)); // go at the address of serverAddress variable and make sizeof(serverAddress) number of bits to be zero 
    memset(&serverAddress2, 0, sizeof(serverAddress2));

    // defining serverAddress structure 
    serverAddress1.sin_family = AF_INET; 
    serverAddress1.sin_port = htons(server1_portno); // Port number (converted to network byte order)
    serverAddress1.sin_addr.s_addr = INADDR_ANY; // Bind to any available interface

    // defining serverAddress structure 
    serverAddress2.sin_family = AF_INET; 
    serverAddress2.sin_port = htons(server2_portno); // Port number (converted to network byte order)
    serverAddress2.sin_addr.s_addr = INADDR_ANY; // Bind to any available interface

    // Now the newly created socket is getting bound by specific IP Address and Port which it can listen. 
    // One socket can only listen one Port 
    int binding = bind(sockfd1, (struct sockaddr *) &serverAddress1, sizeof(serverAddress1));
    if(binding<0){
        error("Binding Failed"); 
    }
    binding = bind(sockfd2, (struct sockaddr *) &serverAddress2, sizeof(serverAddress2));
    if(binding<0){
        error("Binding Failed"); 
    }
    listen(sockfd1, 1); 
    listen(sockfd2, 1); 
    socklen_t lbLen = sizeof(lbAddress); 

    while(1){
        srand(time(NULL));
        char buffer[255]; 
        memset(buffer, 0, sizeof(buffer)); 
        int newsockfd1 = accept(sockfd1, (struct sockaddr *) &lbAddress, &lbLen);
        if(newsockfd1 <0){
            error("No connection request in queue"); 
        }
        int n = recv(newsockfd1, buffer, sizeof(buffer), 0); 
        if(n<0) error("Error in recieving"); 
        if(strcmp(buffer, "SEND_LOAD")==0){
            // Generate a random number between 1 and 100
            int random_number1 = rand() % 100 + 1;
            printf("random_number1: %d\n", random_number1); 
            int n = send(newsockfd1, &random_number1, sizeof(int), 0);
            if(n<0) error("Recieving failed");
        }
        else{
            time_t t;
            time(&t);
            memset(buffer, 0, sizeof(buffer)); 
            strcpy(buffer, ctime(&t));
            int n = send(newsockfd, buffer, sizeof(buffer), 0); 
            if(n<0) error("Recieving failed");
        }

        close(newsockfd1); 


        memset(buffer, 0, sizeof(buffer)); 
        int newsockfd1 = accept(sockfd1, (struct sockaddr *) &lbAddress, &lbLen);
        if(newsockfd1 <0){
            error("No connection request in queue"); 
        }
        int n = recv(newsockfd1, buffer, sizeof(buffer), 0); 
        if(n<0) error("Error in recieving"); 
        if(strcmp(buffer, "SEND_LOAD")==0){
            // Generate a random number between 1 and 100
            int random_number1 = rand() % 100 + 1;
            printf("random_number1: %d\n", random_number1); 
            int n = send(newsockfd1, &random_number1, sizeof(int), 0);
            if(n<0) error("Recieving failed");
        }
        else{
            time_t t;
            time(&t);
            memset(buffer, 0, sizeof(buffer)); 
            strcpy(buffer, ctime(&t));
            int n = send(newsockfd, buffer, sizeof(buffer), 0); 
            if(n<0) error("Recieving failed");
        }

        close(newsockfd1); 

    }
    close(sockfd1); 
    close(sockfd2); 

}