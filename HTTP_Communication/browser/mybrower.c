#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <time.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h> 

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
int connect_to_server(char* ip_address, int portno){

    int sockfd = socket(AF_INET6, SOCK_STREAM, 0); 
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Unable to create socket\n");
        return -1;
    }

    struct sockaddr_in serverAddress;  
    memset(&serverAddress, 0, sizeof(serverAddress)); 
    serverAddress.sin_addr.s_addr = AF_INET; 
    inet_aton(ip_address, &serverAddress.sin_addr);
    serverAddress.sin_port = htons(portno); 

    if(connect(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) <0){
        perror("error in Connection"); 
        return -1;
    }
    return sockfd; 

}

void GET_Headers(dynamicstring *http_request, dynamicstring ** words, dynamicstring* url, dynamicstring * ip_address, dynamicstring * extension){
    // URL AND HOST HEADERS
    http_request = push_string(http_request, words[0]->str); 
    http_request = push_string(http_request, " ");
    http_request = push_string(http_request, url->str);
    http_request = push_string(http_request, " HTTP/1.1\n");
    http_request = push_string(http_request, "Host: ");
    http_request = push_string(http_request, ip_address->str);
    http_request = push_string(http_request, "\nConnection: Close\nDate: ");

    // DATE HEADERS 
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    http_request = push_string(http_request, asctime(timeinfo));
    http_request = push_character(http_request, '\n'); 

    // ACCEPT HEADERS
    http_request = push_string(http_request, "Accept: " );
    http_request = push_string(http_request, extension->str); 
    http_request = push_string(http_request, "\nAccept-Language: ");
    http_request = push_string(http_request, "en-us");
    
    // if modified since HEADERS
    timeinfo->tm_mday -= 2;
    mktime(timeinfo);
    http_request = push_string(http_request, "\nIf-Modified-Since: ");
    http_request = push_string(http_request, asctime(timeinfo));
    http_request = push_string(http_request, "\n\n");

}
void PUT_Headers(dynamicstring *http_request, dynamicstring ** words, dynamicstring* url, dynamicstring * ip_address, dynamicstring * extension, int file_size){

    // GET/SET header along with Host, Connection
    http_request = push_string(http_request, words[0]->str);
    http_request = push_string(http_request, " ");
    http_request = push_string(http_request, url->str);
    http_request = push_string(http_request, "/");
    http_request = push_string(http_request, words[2]->str);
    http_request = push_string(http_request, " HTTP/1.1\n");
    http_request = push_string(http_request, "Host: ");
    http_request = push_string(http_request, ip_address->str);
    http_request = push_string(http_request, "\nConnection: Close\nDate: ");

    // Date header
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    http_request = push_string(http_request, asctime(timeinfo));

    // content language header
    http_request = push_string(http_request, "Content-language: en-us");
    http_request = push_string(http_request, "\nContent-Length: ");

    char buffer[10]; 
    sprintf(buffer, "%d", file_size); 
    http_request = push_string(http_request, buffer);
    http_request = push_string(http_request, "\nContent-type: ");
    http_request = push_string(http_request, extension->str);
    http_request = push_string(http_request, "\n\n");

}

void sendWrapper(int sockfd, char *str){
    printf("sending these content: %s\n", str);
    int n = send(sockfd, str, strlen(str)+1, 0); 

    if(n>0) printf("Number of bytes sent: %d\n", n); 
    else if(n==0) printf("Connection is closed by the other side\n"); 
    else printf("Error on sending\n");

    printf("Number of characters %lu \n", strlen(str)); 
}


void recieveWrapper(int sockfd, dynamicstring *str, char buffer[]){

    memset(buffer, 0, 1024); 
    int n = recv(sockfd, buffer, 1024, 0); 

    if(n>0) printf("Number of bytes recieved %d\n", n); 
    else if(n==0) printf("Connection closed by sender\n"); 
    else printf("error on receiveing\n");

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
    sendWrapper(sockfd, file_buffer);

    free(file_buffer);
}


int main(int argc,  char *argv[]){
    printf("PID: %d\n", (int)getpid()); 
    printf("This is your Browser\n"); 
    char buffer[1024]; 
    while(1){
        // taking input from the user; 
        dynamicstring * str = init(); 
        str = take_line_input(str); 

        // parsing the input string
        int num_of_words;
        dynamicstring ** words = parse_words(str, &num_of_words); 

        int is_get=0, is_put=0; 
        // printf("given method is: %s\n", words[0]->str);
        if(strcmp(words[0]->str, "GET")==0) is_get =1; 
        if(strcmp(words[0]->str, "PUT")==0) is_put =1; 
        if(strcmp(words[0]->str, "QUIT")==0){
            printf("You have Quit\n"); 
            exit(1); 
        }
        // printf("%d %d\n", is_get, is_put);

        // parsing informations from words
        dynamicstring * ip_address = get_ip_address(words[1]); 
        dynamicstring * url = get_url(words[1]); 
        printf("URL : %s\n", url->str);
        dynamicstring * extension = get_extension((is_get?words[1]:words[2])); 
        int portno = get_port(words[1]); 

        int sockfd = connect_to_server(ip_address->str, portno); 

        if(sockfd==-1){

            free(str->str); 
            free(str); 
            free(ip_address->str); 
            free(ip_address); 
            free(url->str); 
            free(url); 
            free(extension->str); 
            free(extension); 

            for(int i=0; i<num_of_words; i++){
                free(words[i]->str); 
                free(words[i]); 
            }
            free(words); 

            continue; 

        }

        /// -================================= Sending Request-==================================
        dynamicstring * http_request = init(); 
        int n; 
        
        if(is_get){

            GET_Headers(http_request, words, url, ip_address, extension); 
            
            // send the request header to the server
            sendWrapper(sockfd, http_request->str); 
            
            // printf("Sending http request: \n%s\n",http_request->str);
            // printf("content size: %d\n", http_request->size);
            if(n<0){
                perror("Error in sending data"); 
            }

        }

        if(is_put){
            // printf("inside put operation\n");
            
            FILE * fd = fopen(words[2]->str, "r");
            if(fd<0){
                perror("Error in opening file");
            }
            fseek(fd, 0, SEEK_END);
            int file_size = ftell(fd);
            fseek(fd, 0, SEEK_SET);
            // printf("file size is: %d\n", file_size);

            PUT_Headers(http_request, words, url, ip_address, extension,  file_size); 
            // printf("%s\n",http_request->str);
            printf("Sending: %s\n",http_request->str);
            // send the request header to the server
            sendWrapper(sockfd, http_request->str); 
            // printf("Sending http request: \n%s\n",http_request->str);
            // printf("content size: %d\n", http_request->size);

            if(n<0){
                perror("Error in sending data"); 
            }

            // send the file
            send_file(sockfd, fd); 

            fclose(fd);

        }

        /// -================================= Recieving Response-==================================
        dynamicstring * response  = init(); 
        recieveWrapper(sockfd, response, buffer); 

        printf("-------------------- RESPONSE RECIEVED --------------------\n");
        printf("%s\n", response->str); 
        printf("\n-----------------------------------------------------------\n");

        // check response code 
        int num_of_words2 =0; 
        dynamicstring ** response_words = parse_words(response, &num_of_words2); 

        int response_code = atoi(response_words[1]->str);
        if(response_code==200){
            printf("Request has succeeded\n");

            if(is_get){
                // recieve the file 
                // we will save the file in same directory and open it. 
                char *file_name = "file_recieved.txt"; 
                //  Save the file
                FILE * recfp = fopen(file_name, "w");

                dynamicstring * content = init(); 
                recieveWrapper(sockfd, content, buffer); 

                fprintf(recfp, "%s\n", content->str); 
                free(content); 
                fclose(recfp); 

                // Fork a child process to open the file using the default application on macOS
                if (fork() == 0) {  // Child process
                    freopen("/dev/null", "w", stdout);  // Redirect stdout to /dev/null
                    freopen("/dev/null", "w", stderr);  // Redirect stderr to /dev/null
                    char *args[] = {"open", file_name, NULL};  // Use 'open' for macOS
                    execvp(args[0], args);  // Execute 'open' with file name
                    perror("Error executing open");  // If execvp fails
                    exit(1);  // Exit if the exec fails
                }

            }
            if (is_put)
            {
                printf("File has been uploaded\n");
            }
        }
        else if (response_code == 400)
        {
            printf("Not Modified\n");
        }
        else if (response_code == 403)
        {
            printf("Not Found\n");
        }
        else if (response_code == 404)
        {
            printf("Server cannot find the requested resource.\n");
        }
        else
        {
            printf("Unknown error\n");
        }

        // Free all memory 

        free(str->str); 
        free(str); 
        free(ip_address->str); 
        free(ip_address); 
        free(url->str); 
        free(url); 
        free(extension->str); 
        free(extension); 

        for(int i=0; i<num_of_words; i++){
            free(words[i]->str); 
            free(words[i]); 
        }
        free(words);

        free(response->str); 
        free(response); 

        for(int i=0; i<num_of_words2; i++){
            free(response_words[i]->str); 
            free(response_words[i]); 
        }
        free(response_words);

        free(http_request->str);
        free(http_request);

        close(sockfd); 
 

    }



}