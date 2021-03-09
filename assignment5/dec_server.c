#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>

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

// This function will modify the value of ct and key!
void decrypt(char *ct, char *key) {
    // Loop through each character
    for (int i = 0; i < strlen(ct); ++i) {
        // Replace space characters with values that make the decryption easier
        if (ct[i] == ' ') ct[i] = 'A' + 26;
        if (key[i] == ' ') key[i] = 'A' + 26;
        ct[i] = (ct[i] - key[i] < 0 ? 27 + ct[i] - key[i] : ct[i] - key[i]) + 'A';
        // Recognize and replace space characters if necessary
        if (ct[i] == 26 + 'A') ct[i] = ' ';
    }
}

// Receive an integer over a network socket
int recv_int(int connectionSocket) {
   int bytes_recvd = 0;
   uint32_t num = 0;
   char *buf = (char*)&num;
   // Need to receive 4 bytes
   while (bytes_recvd < 4) {
      int current_bytes_recvd = recv(connectionSocket, buf + bytes_recvd, 4 - bytes_recvd, 0);
      bytes_recvd += current_bytes_recvd < 0 ? 0 : current_bytes_recvd;
   }

   // Got all 4 bytes, now we can convert this back into an integer and return it
   return ntohl(num);
}

// Send a (maybe long) string over a network socket
void send_string(int connectionSocket, char *message, int msglen) {
   int bytes_sent = 0;

   while (bytes_sent < msglen) {
      // Split message up into 1000 byte chunks if it is too long to fit within 1000 bytes
      int bytes_to_send = msglen - bytes_sent > 1000 ? 1000 : msglen - bytes_sent;
      int current_bytes_sent = send(connectionSocket, message + bytes_sent, bytes_to_send, 0);
      bytes_sent += current_bytes_sent < 0 ? 0 : current_bytes_sent;
   }
}

// Receive a string over a network socket and store it into a preallocated string of length len
void recv_string(int connectionSocket, char *str, int len) {
   int bytes_recvd = 0;

   while (bytes_recvd < len) {
      int current_bytes_recvd = recv(connectionSocket, str + bytes_recvd, len - bytes_recvd, 0);
      bytes_recvd += current_bytes_recvd < 0 ? 0 : current_bytes_recvd;
   }
   // Add null terminator
   str[len] = '\0';
}

// Ensure that the correct client process is connecting
bool handshake(int connectionSocket) {
   // Send "dec" to start the handshake
   send_string(connectionSocket, "dec", 3);
   // Get response
   char response[4];
   recv_string(connectionSocket, response, 3);
   if (strcmp(response, "dec") != 0) {
      // Wrong client, exit
      return false;
   }
   // Success!
   return true;
}

int main(int argc, char *argv[]){
   int connectionSocket;
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
         // Make sure we're communicating with the correct client process
         if (!handshake(connectionSocket)) {
            fprintf(stderr, "Warning: refusing connection from invalid client\n");
            close(connectionSocket);
            // Close the listening socket
            close(listenSocket); 
            // This connection is done, exit
            exit(1);
         }
         // Receive the length of the key
         int keylen = recv_int(connectionSocket);
         // Receive the key itself
         char *key = malloc(sizeof(char) * (keylen + 1));
         recv_string(connectionSocket, key, keylen);

         // Receive the length of the plaintext
         int ctlen = recv_int(connectionSocket);
         // Receive the plaintext
         char *ct = malloc(sizeof(char) * (ctlen + 1));
         recv_string(connectionSocket, ct, ctlen);
         
         // Decrypt the message
         decrypt(ct, key);

         // Send the decrypted text back to the client
         send_string(connectionSocket, ct, ctlen);

         // Free memory
         free(ct);
         free(key);

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
