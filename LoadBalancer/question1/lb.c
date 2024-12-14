#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/time.h>
#include <fcntl.h>

void error(const char * msg){
    perror(msg);
    exit(1);
}
int connect_to_server(int server_port);
void update_loads(int *current_server1_load, int *current_server2_load, int server1_portno, int server2_portno); 
int get_difference(struct timeval start_time, struct timeval end_time);
void getDateTime(char buffer[], int sz,  int serverport); 

long long timeInMilliseconds()
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (((long long)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}

int reset_receive_buffer(int sockfd) {
    char buffer[1024];
    ssize_t bytes_read;
    int total_bytes = 0;

    // Set socket to non-blocking mode
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    // Read and discard data until buffer is empty
    do {
        bytes_read = recv(sockfd, buffer, sizeof(buffer), 0);
        if (bytes_read > 0) {
            total_bytes += bytes_read;
        }
    } while (bytes_read > 0);

    // Set socket back to blocking mode
    fcntl(sockfd, F_SETFL, flags);

    printf("Cleared %d bytes from receive buffer\n", total_bytes);

    return total_bytes;
}


int main(int argc, char *argv[]){
    // CLI ARGUMENTS :- ./lb, server1_port_number, server2_port_number, lb_port_number
    if(argc < 4){
        fprintf(stderr, "Not all arguments given\n");
        exit(1);
    }
    int server1_portno = atoi(argv[1]), server2_portno = atoi(argv[2]), lb_portno = atoi(argv[3]);

    // this sockfd will make connection with clients;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) error("Error opening socket");

    struct sockaddr_in lbAddress, clientAddress;

    memset(&lbAddress, 0, sizeof(lbAddress));

    lbAddress.sin_family = AF_INET;
    lbAddress.sin_port = htons(lb_portno); // Port number (converted to network byte order)
    lbAddress.sin_addr.s_addr = INADDR_ANY; // Bind to any available interface

    int binding = bind(sockfd, (struct sockaddr *) &lbAddress, sizeof(lbAddress));
    if(binding < 0){
        error("Binding Failed");
    }

    listen(sockfd, 2); // making it passive

    socklen_t clientLen = sizeof(clientAddress);
    int to_client_sockfd = accept(sockfd, (struct sockaddr *) &clientAddress, &clientLen);
    if(to_client_sockfd < 0) error("Error in client connection");

    // get load data periodically
    int current_server1_load = 0, current_server2_load = 0;
    char buffer[255];
    memset(buffer, 0, sizeof(buffer));

    struct pollfd pollinfo;
    pollinfo.fd = to_client_sockfd;
    pollinfo.events = POLLIN;
    // disjoint intervals
    while(1) {

        int st = 0, end = 5000;
        // recursion 
        while(1){
            // printf("start: %d, end: %d\n", st, end);
            if(abs(st-end)<500) break; 

            long long start_time = timeInMilliseconds(); 
            // printf("Start time: %lld\n", start_time); 
            int m = poll(&pollinfo, 1, end - st);
            long long end_time = timeInMilliseconds(); 
            // printf("End time: %lld\n", start_time); 
            int time_difference = end_time - start_time;
            // printf("time difference is: %d\n", time_difference);
            st += time_difference; 

            if(m < 0) error("Poll failed");
            else if(m == 0){
                // update the loads in both the servers
                // printf("Getting the loads\n"); 
                update_loads(&current_server1_load, &current_server2_load, server1_portno, server2_portno);
                printf("{Load-> Server1: %d, Server2: %d}\n",current_server1_load,current_server2_load ); 
                break;
            }
            else{
                // send the data
                if(current_server1_load <= current_server2_load){
                    getDateTime(buffer, 255, server1_portno);
                    printf("Fetching data from Server 1\n");
                }
                else{
                    getDateTime(buffer, 255, server2_portno);
                    printf("Fetching data from Server 2\n");
                }

                int n = send(to_client_sockfd, buffer, strlen(buffer)+1, 0);
                if(n < 0) error("Error in sencding date and time to client");

                // we have to clear the receive side buffer, otherwise next time the poll can detect the previous stale data
                n = reset_receive_buffer(to_client_sockfd); 

            }
        }
    }
    return 0;
}

int connect_to_server(int server_port){
    // CREATING A SOCKET AT CLIENT SIDE
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if(sockfd<0) error("error in opening socket"); 
    struct sockaddr_in serverAddress; 

    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    inet_aton("127.0.0.1", &serverAddress.sin_addr); // IP ADDRESS OF SERVER
    serverAddress.sin_port = htons(server_port); // PORT NUMBER OF SERVER

    // let's connect out client socket to the server socker 
    if(connect(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) <0){
        error("Connection failed"); 
    }
    return sockfd; 
}

void update_loads(int *current_server1_load, int *current_server2_load, int server1_portno, int server2_portno){
    int newsockfd = connect_to_server(server1_portno); 
    char buffer[255]; 
    memset(buffer, 0, sizeof(buffer)); 
    strcpy(buffer, "SENDLOAD"); 
    // printf("Getting server1 load\n"); 
    int n = send(newsockfd, buffer, strlen(buffer)+1,0); 
    if(n<0) error("Error in sending"); 
    int temp=0; 
    n = recv(newsockfd, &temp, sizeof(int), 0); 
    if(n<0) error("Error in recieving"); 
    *current_server1_load = temp; 
    // printf("------------------\n"); 
    // printf("Server1 load: %d\n", *temp); 
    close(newsockfd); 

    newsockfd = connect_to_server(server2_portno); 
    memset(buffer, 0, sizeof(buffer)); 
    strcpy(buffer, "SENDLOAD"); 
    // printf("Getting server2 load\n"); 
    n = send(newsockfd, buffer, strlen(buffer)+1,0); 
    if(n<0) error("Error in sending"); 
    temp =0; 
    n = recv(newsockfd, &temp, sizeof(int), 0); 
    if(n<0) error("Error in recieving");
    *current_server2_load = temp; 
    // printf("Server2 load: %d\n", temp); 
    // printf("------------------\n"); 
    close(newsockfd); 

}

void getDateTime(char buffer[], int sz, int serverport){
    int newsockfd = connect_to_server(serverport); 
    memset(buffer, 0, sz); 
    strcpy(buffer, "SENDDATETIME"); 
    int n = send(newsockfd, buffer, strlen(buffer)+1,0); 
    if(n<0) error("Error in sending"); 
    
    memset(buffer, 0, sz); 
    n = recv(newsockfd, buffer, sz, 0); 
    if(n<0) error("Error in recieving"); 

    close(newsockfd); 
}