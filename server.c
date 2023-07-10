#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>

#define MAX_CLIENTS 100
#define BUFFER_SZ 2048

#define NAME_LEN 32

static _Atomic unsigned int cli_count = 0;
static int uid = 10;


//CLient Structure 

typedef struct{
  
  struct sockaddr_in address;
  int sockfd;
  int uid;
  char name[NAME_LEN];
}client_t;

client_t *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

 void str_overwrite_stdout(){
    
    printf("\r%s", "> ");
    fflush(stdout);
 
 }
 
 void str_trim_lf(char* arr, int length){
  
  for(int i=0; i<length; i++){
    
    if(arr[i] == '\n'){
    
        arr[i]  = '\0';
        
        break;
    
    }
  
  }
 
 }
 
 
 void queue_add(client_t *cl){
  
  pthread_mutex_lock(&clients_mutex);
  
  for(int i=0; i<MAX_CLIENTS; i++){
	
	if(!clients[i]){
	 
	 clients[i] = cl;
	 break;

         }  
  }
  
  pthread_mutex_unlock(&clients_mutex);
 
 }
 
 
 // uid will be used to remove the client 
 
 void queue_remove(int uid){
  
  pthread_mutex_lock(&clients_mutex);
  
  for(int i=0; i<MAX_CLIENTS; i++){
	
	if(clients[i] -> uid == uid){
	 
	  clients[i] = NULL;
	  break;

         }  
  }

  pthread_mutex_unlock(&clients_mutex);
 
 }
 
 // function to send message to all the clients accept the sender itself 
 
  void send_message(char *s, int uid){
  
     pthread_mutex_lock(&clients_mutex);  
   
     
     for(int i=0;i<MAX_CLIENTS; i++){
     
      if(clients[i]){
     
         if(clients[i]->uid != uid){
          if(write(clients[i]->sockfd , s, strlen(s)) < 0){
             printf("ERROR: write to descriptor failed\n");
             break;
           }
         }
       }
     }
    
    
     pthread_mutex_unlock(&clients_mutex); 
  
  } 
 
 

 void print_ip_addr(struct sockaddr_in addr){
 
  printf("%d.%d.%d.%d", 
        addr.sin_addr.s_addr & 0xff,
        (addr.sin_addr.s_addr & 0xff00) >> 8,
        (addr.sin_addr.s_addr & 0xff0000) >> 16,
        (addr.sin_addr.s_addr & 0xff000000) >> 24);	
 }


void *client_handle(void *arg){
   
   char buffer[BUFFER_SZ];
   char name[NAME_LEN];
   int leave_flag = 0;
   cli_count++;
   
   client_t *cli = (client_t*)arg;
   
   
   // The recv() function receives data on a socket with descriptor socket and stores it in a buffer
   
   if(recv(cli->sockfd, name, NAME_LEN, 0) <=0 ||strlen(name) < 2 ||
    
    strlen(name) >= NAME_LEN -1){
      
      printf("\nPlease enter the name correctly: \n");
      leave_flag = 1;
      
    } else {
      strcpy(cli->name, name);
      sprintf(buffer, "%s has joined\n", cli->name);
      printf("\nNumber of Clients Connected: %d ", cli_count);
      printf("%s", buffer);
      send_message(buffer, cli -> uid);
    }
    
    bzero(buffer,BUFFER_SZ);
    
    while(1)  
    {
       
       if(leave_flag){
       
        break;
       }
       
     int receive = recv(cli->sockfd, buffer, BUFFER_SZ, 0);
     
     
     if (receive > 0){

	 if(strlen(buffer) > 0){
	 send_message(buffer, cli->uid);
	 str_trim_lf(buffer, strlen(buffer));
	 printf("%s\n", buffer);
       }
       
     } 
      
     else if (receive == 0 ||strcmp(buffer, "exit") == 0){
         sprintf(buffer, "%s has left\n", cli->name);
         printf("\n%s" , buffer);
         send_message(buffer, cli->uid);
         leave_flag = 1;
     }  else {
     
        printf("Error: -1\n");
        leave_flag = 1;
     }
     
     bzero(buffer, BUFFER_SZ);
     
     
     }
     
     
     close(cli->sockfd);
     queue_remove(cli->uid);
     free(cli);
     cli_count--;
     pthread_detach(pthread_self());
     
     return NULL;
 
 }
 

int main(int argc, char **argv) {

  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    return EXIT_FAILURE;
  }

  char *ip = "127.0.0.1";
  int port = atoi(argv[1]);

  int option = 1;
  int listenfd = 0, connfd = 0;
  struct sockaddr_in serv_addr;
  struct sockaddr_in cli_addr;
  pthread_t tid;


  // sock stream uses the tcp to establish a connection
  // socket(int domain, int type, int protocol);
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(ip);
  serv_addr.sin_port = htons(port);

  // signals
  signal(SIGPIPE, SIG_IGN);

  // set sock is used to control the behaviour of socket
  
  //int setsockopt(int socket_descriptor, int level, int option_name, const void *value_of_option, socklen_t option_length);

  if (setsockopt(listenfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char*)&option, sizeof(option)) < 0) {
    printf("ERROR: setsockopt\n");
    return EXIT_FAILURE;
  }

  // bind
  // The bind() function in socket programming in C is used to assign an address to a socket created using the socket() function
  if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    printf("ERROR: bind\n");
    return EXIT_FAILURE;
  }

  // The listen() function in socket programming is used to make the server node wait and listen for connections from the client node on the port and address specified by the bind() function.
  if (listen(listenfd, 10) < 0) {
    printf("ERROR: listen\n");
    return EXIT_FAILURE;
  }

  printf("==== WELCOME TO CHATROOM ======\n");

   
   while(1){
    
    //accept the connection
    
    //The accept() function is used to establish a connection between the server and the client nodes for the transfer of data. 
    // int accept(int socket_descriptor, struct sockaddr *restrict address, socklen_t *restrict length_of_address);
    socklen_t clilen = sizeof(cli_addr);
    connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);
   
    //Check for MAX_CLIENTS
   
    
    if ((cli_count + 1) == MAX_CLIENTS){
    
     printf("Maximum Clients Connected. Connection Rejected\n");
     print_ip_addr(cli_addr);
     close(connfd);
     continue;
    }     
   
   // Client Settings
   
   client_t *cli =(client_t *)malloc(sizeof(client_t));
   cli->address = cli_addr;
   cli->sockfd = connfd;
   cli->uid = uid++;
   
   //Add client to queue
   
   queue_add(cli);
   pthread_create(&tid, NULL, &client_handle, (void*)cli);
   
   //Reduce CPU usage 
   sleep(1);
   
   
   }
   
   return EXIT_SUCCESS;  
   
   
   
}
