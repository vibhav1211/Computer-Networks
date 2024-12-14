#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>

void error(char * msg){
    perror(msg); 
    exit(1); 
}

int main(int argc,  char *argv[]){
   if(argc <2){
        error("Port is not given"); 
   }
   int portno = atoi(argv[1]); 

   int sockfd = socket(AF_INET, SOCK_STREAM, 0); 
   if(sockfd<0){
        error("error in opening socket"); 
   }

   struct sockaddr_in serverAddress, clientAddress; 

   memset(&serverAddress, 0, sizeof(serverAddress));

   serverAddress.sin_family = AF_INET; 
   serverAddress.sin_port = htons(portno); 
   serverAddress.sin_addr.s_addr = INADDR_ANY; 
   
   int n = bind(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)); 
   if(n<0){
    error("error in binding"); 
   }
   listen(sockfd ,5); 

   socklen_t clientLen = sizeof(clientAddress); 
   int newsockfd = accept(sockfd, (struct sockaddr *) & clientAddress, &clientLen); 
   if(newsockfd <0){
    error("Error in connection or no pending cleint request"); 
   }

   char buffer[255]; 

   FILE *f = fopen("file_on_server_side.txt", "r"); 
   int number_of_words =0; 

   if(f!=NULL){
      while(fscanf(f, "%s", buffer)==1){
          number_of_words++; 
      }
   }
   else error("file does not exists"); 
   write(newsockfd, &number_of_words, sizeof(number_of_words)); 
   printf("number of words: %d", number_of_words); 
   rewind(f); 

   while(number_of_words>0){
      fscanf(f, "%s", buffer); 
      write(newsockfd, buffer, sizeof(buffer)); 
      number_of_words--;
   }

   printf("the file has been succesfully sent with name as file_on_client_side"); 


   close(sockfd); 
   close(newsockfd); 


   return 0; 

}