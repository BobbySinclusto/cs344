#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_CONNECTIONS 5

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
} 

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char *argv[]){
  int connectionSocket, charsRead;
  char buffer[256];
  pid_t connection_ids[MAX_CONNECTIONS] = {0};
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  // Check usage & args
  if (argc < 2) { 
    fprintf(stderr,"USAGE: %s port\n", argv[0]); 
    exit(1);
  } 
  
  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    error("ERROR opening socket");
  }

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port
  if (bind(listenSocket, 
          (struct sockaddr *)&serverAddress, 
          sizeof(serverAddress)) < 0){
    error("ERROR on binding");
  }

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(listenSocket, 5); 
  
  // Accept a connection, blocking if one is not available until one connects
  while(1){
    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket, 
                (struct sockaddr *)&clientAddress, 
                &sizeOfClientInfo); 
    if (connectionSocket < 0){
      error("ERROR on accept");
    }

    // Check if there is another process available. if not, we'll have to wait for one of them to finish before we can continue.
    int available_idx = -1;
    for (int i = 0; i < MAX_CONNECTIONS; ++i) {
      if (connection_ids[i] == 0) {
        available_idx = i;
      }
      else {
        // Clean up zombie processes if any exist
        int child_status;
        if (waitpid(connection_ids[i], &child_status, WNOHANG) != 0) {
          connection_ids[i] = 0;
        }
      }
    }
    if (available_idx == -1) {
      // No available processes, wait until one of them finishes
      int child_status;
      int child_pid = waitpid(-1, &child_status, 0);
      // Remove the one that finished from the array
      available_idx = 0;
      for (int i = 0; i < MAX_CONNECTIONS; ++i) {
        if (connection_ids[i] == child_pid) {
          connection_ids[i] = 0;
          available_idx = i;
        }
      }
    }

    pid_t child_pid = fork();
    connection_ids[available_idx] = child_pid;

    if (child_pid == 0) {
      // Receive data from the client
      printf("SERVER: Connected to client running at host %d port %d\n", 
                            ntohs(clientAddress.sin_addr.s_addr),
                            ntohs(clientAddress.sin_port));

      // Get the message from the client and display it
      memset(buffer, '\0', 256);
      // Read the client's message from the socket
      charsRead = recv(connectionSocket, buffer, 255, 0); 
      if (charsRead < 0){
        error("ERROR reading from socket");
      }
      printf("SERVER: I received this from the client: \"%s\"\n", buffer);

      // Send a Success message back to the client
      charsRead = send(connectionSocket, 
                      "I am the server, and I got your message", 39, 0); 
      if (charsRead < 0){
        error("ERROR writing to socket");
      }
      // Close the connection socket for this client
      close(connectionSocket);
      // Close the listening socket
      close(listenSocket); 
      // This connection is done, exit
      exit(0);
    }
    else {
      // Parent process can continue normally
      close(connectionSocket);
    }    
  }
  // Close the listening socket
  close(listenSocket); 
  return 0;
}
