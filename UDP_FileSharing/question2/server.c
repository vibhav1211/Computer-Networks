#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <ctype.h>
#include <dirent.h>

void error(const char* msg){
    perror(msg);
    exit(1);
}
void executeCommand(int newsockfd, char buffer[], int size, int CHUNK_SIZE);
void sendWrapper(int newsockfd, char buffer[], int CHUNK_SIZE);
void recieveWrapper(int newsockfd, char buffer[], int CHUNK_SIZE);


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
    int CHUNK_SIZE = 3; 

    while(1){
        memset(buffer, 0, sizeof(buffer)); 
        socklen_t clientLen = sizeof(clientAddress); 
        int newsockfd = accept(sockfd, (struct sockaddr *) &clientAddress, &clientLen);

        if(newsockfd <0){
            error("No connection request in queue"); 
        }
        printf("New client connected: \n"); 

        strcpy(buffer, "LOGIN"); 
        sendWrapper(newsockfd, buffer, CHUNK_SIZE); 
        
        // recieveing username  
        memset(buffer, 0, sizeof(buffer)); 
        char username[255]; 
        memset(username, 0, sizeof(username));
        recieveWrapper(newsockfd, username, CHUNK_SIZE); 


        // Authentication
        int flag = 0; 
        FILE * file = fopen("users.txt", "r"); 
        while(fgets(buffer, sizeof(buffer), file)!=NULL){
            buffer[strcspn(buffer, "\n")] = '\0';
            if(strcmp(buffer, username)==0){
                flag=1; 
            }
        }
        fclose(file);

        memset(buffer, 0, sizeof(buffer)); 
        if(flag) strcpy(buffer, "FOUND"); 
        else strcpy(buffer, "NOT-FOUND"); 

        sendWrapper(newsockfd, buffer, CHUNK_SIZE);

        if(flag){
            
            while(1){
                memset(buffer, 0, sizeof(buffer)); 
                recieveWrapper(newsockfd, buffer, CHUNK_SIZE); 
    
                if(strcmp(buffer, "EXIT")==0){
                    printf("Client has exit: %s\n", buffer);
                    break; 
                }
                printf("Client shell command: %s\n", buffer);
                executeCommand(newsockfd, buffer, sizeof(buffer), CHUNK_SIZE);  
            }
            
        }

        close(newsockfd);
    }
    
    close(sockfd); 

}

void sendWrapper(int newsockfd, char buffer[], int CHUNK_SIZE){
    int sz = strlen(buffer); 
    int CHUNKS = (sz+CHUNK_SIZE-1)/CHUNK_SIZE; 
    // sneding number of CHUNKS
    int n = send(newsockfd, &CHUNKS, sizeof(CHUNKS), 0); 
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
        int n = send(newsockfd, temp, strlen(temp)+1,0); 
        if(n<0) error("Error in sending buffer in chunks"); 
    
    }
    printf("--------------------------\n"); 
}
void recieveWrapper(int newsockfd, char buffer[], int CHUNK_SIZE){
    int CHUNKS; 
    // recieving number of chunks
    int n = recv(newsockfd,&CHUNKS, sizeof(CHUNKS), 0); 
    char temp[CHUNK_SIZE+1]; 
    int ind=0; 
    printf("--------RECIEVED------------\n"); 
    printf("Number of chunks: %d\n", CHUNKS);
    for(int i=0; i<CHUNKS; i++){
        // printf("andr\n");
        memset(temp, 0, sizeof(temp)); 
        n = recv(newsockfd, temp, sizeof(temp),0); 
        printf("CHUNK %d: %s\n", i, temp);
        for(int j =0; j<n-1; j++){
            buffer[ind++] = temp[j]; 
        }
    }
    printf("--------------------------\n"); 
}

void executeCommand(int newsockfd, char buffer[], int size, int CHUNK_SIZE){
    printf("COMMAND %lu: %s\n", strlen(buffer), buffer);
    if(strncmp(buffer, "pwd", 3)==0){
        memset(buffer, 0, size); 
        char* result = getcwd(buffer, size); 
        printf("Command Result: %s\n", result);
        if(result==NULL) error("Error in finding present working directory");

        sendWrapper(newsockfd, buffer,CHUNK_SIZE);  

    }
    else if(strncmp(buffer, "dir", 3)==0){
        DIR *dir;
        struct dirent *ent;
        memset(buffer, 0, size); 

        if ((dir = opendir(".")) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                strcat(buffer, ent->d_name);
                strcat(buffer, "\n");
            }
            closedir(dir);
            printf("list of items: %s\n", buffer);

            sendWrapper(newsockfd, buffer, CHUNK_SIZE);  

        } else {
            char errorMsg[255]; 
            strcpy(errorMsg, "Could not open directory"); 
            sendWrapper(newsockfd, errorMsg, CHUNK_SIZE);  
            
        }
    }
    // CD command
    else if (strncmp(buffer, "cd ", 3) == 0) {
        char *path = buffer + 3;  // Skip "cd " to get the path
        if (chdir(path) == 0) {

            char successMsg[255]; 
            strcpy(successMsg, "Directory changed successfully"); 
            sendWrapper(newsockfd, successMsg, CHUNK_SIZE);  
        
        } else {
            char errorMsg[256];
            snprintf(errorMsg, sizeof(errorMsg), "Failed to change directory: %s", strerror(errno));
            sendWrapper(newsockfd, errorMsg, CHUNK_SIZE);  
           
        }
    }
    else {
        char errorMsg[255]; 
        strcpy(errorMsg, "Unknown command"); 
        sendWrapper(newsockfd, errorMsg, CHUNK_SIZE);  
       
    }
}

