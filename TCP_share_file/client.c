#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

void error(const char * msg){
    perror(msg); 
    exit(1);
}

int main(int argc, char *argv[]){
    if(argc <3){
        error("port or IP address is missing"); 
    }
    int server_port_number = atoi(argv[2]); 
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if(sockfd<0){
        error("error in opening socket"); 
    }

    struct hostent *server = gethostbyname(argv[1]); 
    if(server==NULL){
        error("No host exists"); 
    }

    struct sockaddr_in serverAddress, clientAddress; 
    serverAddress.sin_family = AF_INET; 
    serverAddress.sin_port = htons(server_port_number); 
    bcopy((char *) server->h_addr, (char *) &serverAddress.sin_addr.s_addr, server->h_length); 

    int n = connect(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)); 
    if(n<0){
        error("Error in connecting"); 
    }

    // connection established so lets interact; 

    char buffer[255]; 
    int number_of_words; 
    read(sockfd, &number_of_words, sizeof(number_of_words)); 
    printf("Number of words in the server file is: %d", number_of_words); 

    FILE *f = fopen("file_on_client_side.txt", "a"); 
    while(number_of_words>0){
        read(sockfd, buffer, sizeof(buffer));
        fprintf(f, "%s", buffer); 
        number_of_words--; 
    }

    close(sockfd); 

   
    return 0;
}