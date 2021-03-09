#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#include <stdbool.h>

/**
 * Client code
 * 1. Create a socket and connect to the server specified in the command arugments.  * 2. Prompt the user for input and send that input as a message to the server.  * 3. Print the message received from the server and exit the program.
 */

// Error function used for reporting issues
void error(const char *msg) { 
   perror(msg); 
   exit(0); 
} 

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, 
      int portNumber, 
      char* hostname){

   // Clear out the address struct
   memset((char*) address, '\0', sizeof(*address)); 

   // The address should be network capable
   address->sin_family = AF_INET;
   // Store the port number
   address->sin_port = htons(portNumber);

   // Get the DNS entry for this host name
   struct hostent* hostInfo = gethostbyname(hostname); 
   if (hostInfo == NULL) { 
      fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
      exit(0); 
   }
   // Copy the first IP address from the DNS entry to sin_addr.s_addr
   memcpy((char*) &address->sin_addr.s_addr, 
         hostInfo->h_addr_list[0],
         hostInfo->h_length);
}

// Get the plaintext and key from the given files
bool get_text_from_files(char *ptfilename, char *keyfilename, char **pt, char **key) {
   // Open files
   FILE *ptfile = fopen(ptfilename, "r");
   if (ptfile == NULL) {
      fprintf(stderr, "Error: could not open plaintext file %s\n", ptfilename);
      return false;
   }

   FILE *keyfile = fopen(keyfilename, "r");
   if (keyfile == NULL) {
      fprintf(stderr, "Error: could not open key file %s\n", keyfilename);
      return false;
   }

   // Get text from files
   size_t n;
   getline(pt, &n, ptfile);
   getline(key, &n, keyfile);
   fclose(ptfile);
   fclose(keyfile);

   int pt_len = strlen(*pt) - 1;
   int key_len = strlen(*key) - 1;

   // Strip newlines
   (*pt)[pt_len] = '\0';
   (*key)[key_len] = '\0';

   // Check for bad characters
   for (int i = 0; i < pt_len; ++i) {
      if ((*pt)[i] != ' ' && ((*pt)[i] < 'A' || (*pt)[i] > 'Z')) {
         fprintf(stderr, "enc_client error: input contains bad characters\n");
         free(*pt);
         free(*key);
         return false;
      }
   }

   // Verify length of key
   if (key_len < pt_len) {
      fprintf(stderr, "Error: key '%s' is too short\n", keyfilename);
      free(*pt);
      free(*key);
      return false;
   }
   return true;
}

// Send an integer over a network socket
void send_int(int connectionSocket, int to_send) {
   int bytes_sent = 0;
   uint32_t num = htonl(to_send);
   char *buf = (char*)&num;
   // Need to send 4 bytes
   while (bytes_sent < 4) {
      int current_bytes_sent = send(connectionSocket, buf + bytes_sent, 4, 0);
      bytes_sent += current_bytes_sent < 0 ? 0 : current_bytes_sent;
   }
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

// Ensure that the correct client process is communicating with the correct server process
bool handshake(int connectionSocket) {
   // Wait for the server to send its first message, expect "enc"
   char response[4];
   recv_string(connectionSocket, response, 3);
   if (strcmp(response, "enc") != 0) {
      // If this is the incorrect server process, send "bad"
      send_string(connectionSocket, "bad", 3);
      return false;
   }
   // If this is the correct server process, send back "enc" so it knows to continue
   send_string(connectionSocket, "enc", 3);
   return true;
}

int main(int argc, char *argv[]) {
   int socketFD;
   struct sockaddr_in serverAddress;
   // Check usage & args
   if (argc != 4) {
      fprintf(stderr, "Usage: %s plaintext key port\n", argv[0]);
      return 1;
   }

   char *pt = NULL;
   char *key = NULL;
   if (!get_text_from_files(argv[1], argv[2], &pt, &key)) {
      // An error occurred, exit
      return 1;
   }

   // Create a socket
   socketFD = socket(AF_INET, SOCK_STREAM, 0); 
   if (socketFD < 0){
      error("CLIENT: ERROR opening socket");
   }

   // Set up the server address struct
   setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

   // Connect to server
   if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
      error("CLIENT: ERROR connecting");
   }

   // Make sure we're connecting to a valid encryption server
   if (!handshake(socketFD)) {
      fprintf(stderr, "Error: not a valid encryption server\n");
      close(socketFD);
      return 1;
   }

   // Send key
   int keylen = strlen(key);
   send_int(socketFD, keylen);
   send_string(socketFD, key, keylen);

   // Send plaintext
   int ptlen = strlen(pt);
   send_int(socketFD, ptlen);
   send_string(socketFD, pt, ptlen);

   // Get the ciphertext back from the server and store it in pt
   recv_string(socketFD, pt, ptlen);

   // Print encrypted text to stdout
   printf("%s\n", pt);

   // Free memory
   free(pt);
   free(key);

   // Close the socket
   close(socketFD); 
   return 0;
}
