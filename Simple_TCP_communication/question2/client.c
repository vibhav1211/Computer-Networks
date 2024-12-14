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
    // CLI ARGUMENTS :- ./client server_IP_Address, server_port_number
    if(argc <3){
        fprintf(stderr, "Not all arguments given"); 
        exit(0); 
    }
    
    int portno = atoi(argv[2]); 

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
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


    // let's connect out client socket to the server socker 
    if(connect(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) <0){
        error("Connection failed"); 
    }


    // let's have conversation 
    char buffer[255];
    int i=0; 
    char ch = '1'; 
    while(ch!='\n'){
        scanf("%c", &ch); 
        buffer[i++] = ch; 
    }
    printf("entered text is: %s\n", buffer);
    int n = send(sockfd, buffer, strlen(buffer), 0);
    
    int res; 
    n = read(sockfd, &res, sizeof(res)); 
    printf("Server: %d\n", res); 
    close(sockfd); 
    return 0; 

}