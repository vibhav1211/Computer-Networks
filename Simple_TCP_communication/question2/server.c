#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char* msg){
    perror(msg);
    exit(1);
}

int main(int argc,  char *argv[]){
    // CLI ARGUMENTS :- ./server port_number
    // printf("Number of arguments are: %d\n", argc); 
    // for(int i=0; i<argc; i++){
    //     printf("%d argument is: %s\n", i, argv[i]); 
    // }

    if(argc <2){
        fprintf(stderr, "Port is not provided"); 
        exit(1); 
    }

    int portno = atoi(argv[1]);

    // Creating a new socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if(sockfd < 0){
        error("Socket couldn't be opened"); 
    }

    // Crearing server and client side addresses.
    struct sockaddr_in serverAddress, clientAddress; 

    // intialising with zero 
    memset(&serverAddress, 0, sizeof(serverAddress)); // go at the address of serverAddress variable and make sizeof(serverAddress) number of bits to be zero 

    // defining serverAddress structure 
    serverAddress.sin_family = AF_INET; 
    serverAddress.sin_port = htons(portno); // Port number (converted to network byte order)
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Bind to any available interface

    // Now the newly created socket is getting bound by specific IP Address and Port which it can listen. 
    // One socket can only listen one Port 
    int binding = bind(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    if(binding<0){
        error("Binding Failed"); 
    }

    // By listening we make the socket passive, meanly accns it can oept upcoming connection requests from other sockets, 
    // it can not initiate it's own connection request to other sockets.
    listen(sockfd, 5); // by 5 we are restriction maximum number of connnection requests which can be queued


    char buffer[255]; // this will hold data which will get send or recieved


    while(1){
        memset(buffer, 0, sizeof(buffer)); 
        socklen_t clientLen = sizeof(clientAddress); 
        int newsockfd = accept(sockfd, (struct sockaddr *) &clientAddress, &clientLen);

        if(newsockfd <0){
            error("No connection request in queue"); 
        }
        printf("New client connected\n"); 
        int n = recv(newsockfd, buffer, sizeof(buffer) - 1, 0);
        if (n > 0) {
            printf("Client: %s\n", buffer);
        } else if (n == 0) {
            printf("Client disconnected\n");
        } else {
            perror("recv failed");
        }
        int res = 10; 
        write(newsockfd, &res, sizeof(res)); 
        close(newsockfd); 
    }
    close(sockfd); 

}