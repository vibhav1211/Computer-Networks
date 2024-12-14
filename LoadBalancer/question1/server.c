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
    if(argc <2){
        fprintf(stderr, "Port is not provided"); 
        exit(1); 
    }
    int portno = atoi(argv[1]); 

    int sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if(sockfd < 0){
        error("Socket couldn't be opened"); 
    }
    struct sockaddr_in serverAddress, clientAddress; 
     // defining serverAddress structure 
    serverAddress.sin_family = AF_INET; 
    serverAddress.sin_port = htons(portno); // Port number (converted to network byte order)
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Bind to any available interface

    int binding = bind(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    if(binding<0){
        error("Binding Failed"); 
    }
    listen(sockfd, 5); 
    srand(time(NULL));
    while(1){
        socklen_t cliLen = sizeof(clientAddress); 
        int newsockfd = accept(sockfd, (struct sockaddr *) &clientAddress, &cliLen);

        char buffer[255]; 
        memset(buffer, 0, sizeof(buffer)); 
        int n = recv(newsockfd, buffer, sizeof(buffer),0); 
        if(n<0) error("Error in recieving"); 

        if(strcmp(buffer, "SENDLOAD")==0){
            int random_number= rand() % 100 + 1;
            printf("random_number: %d\n", random_number); 
            int n = send(newsockfd, &random_number, sizeof(int), 0);
            if(n<0) error("sending failed");
        }
        else{
            time_t t;
            time(&t);
            memset(buffer, 0, sizeof(buffer)); 
            strcpy(buffer, ctime(&t));
            int n = send(newsockfd, buffer, strlen(buffer)+1, 0); 
            if(n<0) error("sending failed");
        }
        close(newsockfd); 
    }
    close(sockfd);    
    

}