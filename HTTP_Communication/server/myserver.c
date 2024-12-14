#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <time.h>
#include <assert.h>

const int increment = 20; 
typedef struct{
    char *str; 
    int size; 
    int capacity; 
}dynamicstring; 

dynamicstring * init(){
    dynamicstring * str = (dynamicstring *)malloc(sizeof(dynamicstring)); 
    str->str = NULL; 
    str->size = 0; 
    str->capacity =0; 
    return str; 
}
dynamicstring * clear(dynamicstring *str){
    if(str->str!=NULL) free(str->str); 
    str->size=0; 
    str->capacity=0; 

}

dynamicstring * push_character(dynamicstring * str, char ch){
    if(str->capacity <= str->size +1){
        if(str->capacity==0){
            str->str = (char *) malloc(increment); 
        }
        else{
            str->str = (char *) realloc(str->str, increment + str->capacity); 
        }
        str->capacity+=increment;
    }
    str->str[str->size]=ch; 
    str->size++; 
    for(int i=str->size; i<str->capacity; i++) str->str[i]='\0'; 
    return str; 
}

dynamicstring* push_string(dynamicstring *str, char *c){
    for(int i=0; i<(int)strlen(c); i++){
        str = push_character(str, c[i]); 
    }
    return str; 
}

dynamicstring * take_line_input(dynamicstring *str){
    char ch; 
    while((ch=getchar()) != '\n'){
        str = push_character(str, ch); 
    }
    return str; 
}
int check(char ch){
    return (ch!=' ' && ch!='\t' && ch!='\0' && ch!='\n'); 
}

dynamicstring ** parse_words(dynamicstring *str, int *num_of_words){
    // let's count number of words

    *num_of_words =0; 
    int i=0; 
    while(i<str->size){
        if(check(str->str[i])) *num_of_words+=1; 
        int j=i; 
        while(check(str->str[j])){
            j++;
        }
        i = j+1; 
    }
    // now we can allocated memory for dynamicstring **; 
    dynamicstring ** res = (dynamicstring **) malloc(*num_of_words*sizeof(dynamicstring *)); 
    int k=0; 
    i=0; 
    while(i<str->size){
        dynamicstring* temp = init(); 
        int j=i; 
        while(check(str->str[j])){
            temp = push_character(temp, str->str[j]); 
            j++;
        }
        i = j+1; 
        if(temp->size){
            *(res+k) = temp; 
            k++; 
        }
    }
    return res; 
   
}
dynamicstring * get_url(dynamicstring * str){
    int flag=0; 
    dynamicstring * res = init(); 
    
    for(int i=7; i<str->size; i++){
        if(str->str[i]=='/') flag=1; 
        if(str->str[i]==':'){
            break;
        } 
        if(flag){
            res = push_character(res, str->str[i]); 
        }
    }
    return res; 

}

dynamicstring * get_ip_address(dynamicstring * str){

    dynamicstring * res = init();
    for(int i=7; i<str->size; i++){
        if(str->str[i]=='/') break; 
        res = push_character(res, str->str[i]);  
    }
    return res; 

}
dynamicstring * get_extension(dynamicstring *str){
    
    int dot =-1, colon = -1; 
    for(int i=str->size-1; i>=0; i--){
        if(str->str[i]==':') colon = i; 
        if(str->str[i]=='.'){
            dot = i; 
            break;
        }
    }
    dynamicstring* res = init(); 
    if(dot==-1){
        // no extension so default is text
        res = push_string(res, "text/*"); 
    }
    else{
        dynamicstring* store =init(); 
        for(int i = dot+1; i<((colon==-1)?str->size:colon); i++){
            store = push_character(store, str->str[i]);
        }
        if(strcmp(store->str, "html")==0){
            res = push_string(res, "text/html"); 
        }
        else if(strcmp(store->str, "pdf")==0){
            res = push_string(res, "application/pdf"); 
        }
        else if(strcmp(store->str, "jpg")==0){
            res = push_string(res, "image/jpeg"); 
        }
        else res = push_string(res, "text/*"); 
    }
    return res; 
    
}
int get_port(dynamicstring *str){
    int colon =-1; 
    for(int i=str->size-1; i>=0; i--){
        if(str->str[i]==':'){
            colon = i; 
            break; 
        }
    }
    if(colon==-1) return 8080;
    else{
        int num=0; 
        for(int i=colon+1; i<str->size; i++){
            num = num*10 + (str->str[i]-'0'); 
        }
        return num; 
    }
    
}

int is_400_error(dynamicstring ** words, int num_of_words){
    // 400 Means Bad Request

    if(strcmp(words[0]->str, "GET")!=0 && strcmp(words[0]->str, "PUT")!=0) return 1; 

    // check protocol version is present or not?
    int i=0; 
    for(; i<num_of_words; i++){
        if(strcmp(words[i]->str, "HTTP/1.1")==0) break; 
    }
    if(i==num_of_words) return 1; 
    
    i=0; 
    // check host is given or not
    for(; i<num_of_words; i++){
        if(strcmp(words[i]->str, "Host:")==0) break; 
    }
    if(i==num_of_words) return 1; 

    i=0;
    // check Connection is there or not
    for(; i<num_of_words; i++){
        if(strcmp(words[i]->str, "Connection:")==0) break; 
    }
    if(i==num_of_words) return 1; 

    i=0; 
    // Check for date header
    for (i = 0; i < num_of_words; i++)
    {
        if (strcmp(words[i]->str, "Date:") == 0)
            break;
    }
    if(i==num_of_words) return 1; 

    return 0; 
    
}
void logging(char* client_ip_address, int client_port_number, dynamicstring **words){

    FILE *fp = fopen("access_log.txt", "a+");
    if (fp == NULL)
    {
        perror("Error Opening Access Log File");
        exit(EXIT_FAILURE);
    }
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    fprintf(fp, "<%02d%02d%02d>:<%02d%02d%02d>:<%s>:<%d>:<%s>:<%s>\n", timeinfo->tm_mday, timeinfo->tm_mon, timeinfo->tm_year % 100, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, client_ip_address, client_port_number, words[0]->str, words[1]->str);
    fclose(fp);

}
dynamicstring * read_file(char * file_path){
    FILE * fd = fopen(file_path, "r"); 
    fseek(fd, 0, SEEK_END);
    int file_size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    dynamicstring * file_content = init(); 
    char c;
    for (int i = 0; i < file_size; i++)
    {
        c = fgetc(fd);
        file_content = push_character(file_content, c);
    }
    fclose(fd);

    return file_content; 

}
int PUT_content_size(dynamicstring **words, int num_of_words){
    for(int i=0; i<num_of_words; i++){
        if(strcmp(words[i]->str, "Content-Length:")==0){
            return atoi(words[i+1]->str);
        }
    }
}


void sendWrapper(int sockfd, char *str){
    printf("sending these content: %s\n", str);
    int n = send(sockfd, str, strlen(str)+1, 0); 

    if(n>0) printf("Number of bytes sent: %d\n", n); 
    else if(n==0) printf("Connection is closed by the other side\n"); 
    else printf("Error on sending\n");
}


void recieveWrapper(int sockfd, dynamicstring *str, char buffer[]){

    memset(buffer, 0, 1024); 
    int n = recv(sockfd, buffer, 1024, 0); 
    
    if(n>0) printf("Number of bytes recieved %d\n", n); 
    else if(n==0) printf("Connection closed by sender\n"); 
    else printf("error on receiveing\n");
    printf("Number of characters %lu \n", strlen(buffer)); 

    // printf("buffer contains: %s\n", buffer);
    for(int i=0; i<(int)strlen(buffer); i++){
        str = push_character(str, buffer[i]);
    }
    memset(buffer, 0, 1024); 
}

void send_file(int sockfd, FILE *fp)
{
    fseek(fp, 0L, SEEK_END);
    int file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET); 

    char *file_buffer = (char *)malloc(file_size+10);
    memset(file_buffer, 0, file_size+10);
    if (!file_buffer)
    {
        perror("Unable to allocate memory");
        return;
    }
    // read the content of file into file_buffer; 
    fread(file_buffer, 1, file_size, fp);
    // printf("sending these content: %s\n", file_buffer);
    sendWrapper(sockfd, file_buffer);

    free(file_buffer);
}

void recieve_file(int sockfd, char buffer[], char * filepath){

    // Save the content of the file to it's  given path 

    FILE * recfp = fopen(filepath, "w");

    dynamicstring * content = init(); 
    recieveWrapper(sockfd, content, buffer); 

    fprintf(recfp, "%s\n", content->str); 
    free(content); 
    fclose(recfp); 

}


int main(int argc,  char *argv[]){
    // CLI ARGUMENTS :- ./server port_number
    // printf("Number of arguments are: %d\n", argc); 
    // for(int i=0; i<argc; i++){
    //     printf("%d argument is: %s\n", i, argv[i]); 
    // }
    printf("PID: %d\n", (int)getpid()); 
    if(argc <2){
        fprintf(stderr, "Port is not provided"); 
        exit(1); 
    }

    int portno = atoi(argv[1]);

    // Creating a new socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if(sockfd < 0){
        perror("Socket couldn't be opened"); 
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
        perror("Binding Failed"); 
    }

    // By listening we make the socket passive, meanly accns it can oept upcoming connection requests from other sockets, 
    // it can not initiate it's own connection request to other sockets.
    listen(sockfd, 5); // by 5 we are restriction maximum number of connnection requests which can be queued

    // accept means fetching the first connectiomn request from the queue and creating new active socket for the communication
    socklen_t clientLen = sizeof(clientAddress); 
    

    
    int n; 
    char buffer[1024]; 
    while(1){
        int newsockfd = accept(sockfd, (struct sockaddr *) &clientAddress, &clientLen);

        if(newsockfd <0){
            perror("No connection request in queue"); 
        }
        // storing client IP address and Port number which would be helpful into logging data
        int IP_ADDRESS_SIZE = 100;
        char client_ip_address[IP_ADDRESS_SIZE];
        if (inet_ntop(AF_INET, &clientAddress.sin_addr, client_ip_address, sizeof(client_ip_address)) == NULL) {
            perror("inet_ntop failed");
            // Handle error
        }
        int client_port_number = ntohs(clientAddress.sin_port);

        // reading data from client
        dynamicstring * str = init();
        recieveWrapper(newsockfd, str, buffer);
 
        // printf("current length is: %lu\n", strlen(str->str));
        
        // printf("%s\n", str->str);

        int num_of_words = 0;
        dynamicstring ** words = parse_words(str, &num_of_words);

        printf("\n-----------------------Request -----------------------\n"); 
        // printf("number of words are: %d\n", num_of_words);
        printf("%s", str->str);
        printf("\n------------------------------------------------------\n"); 

        int is_400 = is_400_error(words, num_of_words); 

        int is_get=0, is_put =0;
        if(strcmp(words[0]->str, "GET")==0) is_get =1; 
        if(strcmp(words[0]->str, "PUT")==0) is_put =1; 
    
        dynamicstring * extension = get_extension(words[1]);

        dynamicstring *response = init(); 

        response = push_string(response, words[2]->str); 

        int status_code = 400; 

        if(is_400) response = push_string(response, " 400 Bad Request");
        else{

            // log this request into our database
            logging(client_ip_address, client_port_number, words); 


            if(is_get){
                // Some file is begin requested
                char * file_path = words[1]->str; 
                if (access(file_path, F_OK) == -1)
                {
                    status_code = 403;
                    response = push_string(response, " 403 Not Found");
                }
                else{
                    status_code = 200; 
                    response = push_string(response, " 200 OK");

                    dynamicstring * file_content = read_file(file_path); 

                    // Expire header
                    time_t rawtime;
                    struct tm *timeinfo;
                    time(&rawtime);
                    timeinfo = localtime(&rawtime);
            
                    struct tm *expire_timeinfo;
                    expire_timeinfo = timeinfo;
                    expire_timeinfo->tm_mday += 3; // 3 days expire
                    mktime(expire_timeinfo);
                    response = push_string(response, "\nExpires: ");
                    response = push_string(response, asctime(expire_timeinfo));

                    // Cache-Control header
                    response = push_string(response, "Cache-control: no-store");
                    // Content-Language header
                    response = push_string(response, "\nContent-Language: en-us");

                    // Content-length header
                    response = push_string(response, "\nContent-Length: ");
                    char content_length[100];
                    sprintf(content_length, "%d", file_content->size);
                    response = push_string(response, content_length);

                    // Content-Type header
                    response = push_string(response, "\nContent-Type: ");
                    response = push_string(response, extension->str);

                    // Last Modified header
                    response = push_string(response, "\nLast-Modified: ");
        
                    // Find the last modified time
                    struct stat file_stat;
                    stat(file_path, &file_stat);
                    struct tm *last_modified_timeinfo;
                    last_modified_timeinfo = localtime(&file_stat.st_mtime);
                    response = push_string(response, asctime(last_modified_timeinfo));

                    response = push_string(response, "\n\n");

                    free(file_content);
            

                }

            }

            if(is_put){
                // printf("inside_put\n");
                if(access(words[1]->str, F_OK)!=-1 && access(words[1]->str, W_OK)==-1){
                    status_code = 403; 
                    response  = push_string(response, " 403 Forbidden"); 
                }
                else{
                    // printf("inside_put\n");
                    status_code = 200; 
                    response  = push_string(response, " 200 OK"); 

                    recieve_file(newsockfd, buffer, words[1]->str);

                }


            }


        }
        // send the response 
        sendWrapper(newsockfd, response->str); 

        // if the request was GET request send the file
        if (is_get && status_code == 200)
        {
            // send file
            FILE *readfp = fopen(words[1]->str, "r");
            send_file(newsockfd, readfp);
            fclose(readfp);
        }


        printf("\n---------------------Response Sent---------------------\n");
        printf("%s", response->str);
        printf("\n------------------------------------------------------\n");

        // Free all memory blocks
        free(str->str); 
        free(str);

        for(int i=0; i<num_of_words; i++){
            free(words[i]->str); 
            free(words[i]); 
        }
        free(words);
        free(extension->str); 
        free(extension); 
        free(response->str); 
        free(response);
    
        close(newsockfd);
   
    }
    
    close(sockfd); 

}