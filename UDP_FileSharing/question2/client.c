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
void sendWrapper(int sockfd, char buffer[], int CHUNK_SIZE);
void recieveWrapper(int sockfd, char buffer[], int CHUNK_SIZE);


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
    int CHUNK_SIZE = 3;
    
    memset(buffer, 0, sizeof(buffer)); 
    recieveWrapper(sockfd, buffer, CHUNK_SIZE); 

    printf("%s\n", buffer); 
    printf("Enter your username: \n");
    memset(buffer, 0, sizeof(buffer)); 
    char ch='1'; 
    int i=0;
    while(ch!='\n'){
        scanf("%c", &ch); 
        buffer[i++] = ch; 
    } 

    buffer[strcspn(buffer, "\n")] = '\0';
    
    sendWrapper(sockfd, buffer, CHUNK_SIZE); 
   
    memset(buffer, 0,sizeof(buffer));
    recieveWrapper(sockfd, buffer, CHUNK_SIZE); 


    if(strcmp(buffer, "NOT-FOUND")==0){
        printf("Invalid username\n");
    }
    else{
        printf("Login Succesfull.. \n"); 
        while(1){
            printf("Enter your shell command or chose EXIT: \n");
            memset(buffer, 0, sizeof(buffer)); 
            fgets(buffer, sizeof(buffer), stdin); 
            buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character
            
            sendWrapper(sockfd, buffer, CHUNK_SIZE); 

            if(strcmp(buffer, "EXIT")==0){
                printf("You have choosen exit option\n");
                close(sockfd); 
                return 0; 
            }

            memset(buffer, 0, sizeof(buffer)); 
            recieveWrapper(sockfd, buffer, CHUNK_SIZE); 
            printf("Result: %s\n", buffer);
        }
        
        
    }


    close(sockfd); 
    return 0; 

}


void sendWrapper(int sockfd, char buffer[], int CHUNK_SIZE){
    int sz = strlen(buffer); 
    int CHUNKS = (sz+CHUNK_SIZE-1)/CHUNK_SIZE; 
    // sneding number of CHUNKS
    int n = send(sockfd, &CHUNKS, sizeof(CHUNKS), 0); 
    int ind =0; 
    char temp[CHUNK_SIZE+1]; 
    printf("--------SENT------------\n"); 
    printf("Number of chunks: %d\n", CHUNKS);
    for(int i=0; i<CHUNKS; i++){
        memset(temp, 0, sizeof(temp)); 
        int k=0; 
        for(int j=ind; j-ind+1<=CHUNK_SIZE; j++){
            temp[k++] = buffer[j]; 
        }
        ind+=CHUNK_SIZE; 
        printf("CHUNK %d: %s\n", i, temp);

        int n = send(sockfd, temp, strlen(temp)+1,0); 
        if(n<0) error("Error in sending buffer in chunks"); 
    
    }
    printf("--------------------------\n"); 
}

void recieveWrapper(int sockfd, char buffer[], int CHUNK_SIZE){
    int CHUNKS; 
    // recieving number of chunks
    int n = recv(sockfd,&CHUNKS, sizeof(CHUNKS), 0); 
    char temp[CHUNK_SIZE+1]; 
    int ind=0; 
    printf("--------RECIEVED------------\n"); 
    printf("Number of chunks: %d\n", CHUNKS);
    for(int i=0; i<CHUNKS; i++){
        // printf("andr\n");
        memset(temp, 0, sizeof(temp)); 
        n = recv(sockfd, temp, sizeof(temp),0); 
        printf("CHUNK %d: %s\n", i, temp);
        for(int j =0; j<n-1; j++){
            buffer[ind++] = temp[j]; 
        }
    }
    printf("--------------------------\n"); 
}