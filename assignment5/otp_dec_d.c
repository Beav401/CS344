#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>


void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues


//much of the code in the main function was taken from the server.c file that was provided
int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[1000];
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

  while(1) {  //repeating loop
  	// Accept a connection, blocking if one is not available until one connects
  	sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
  	establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
  	if (establishedConnectionFD < 0) error("ERROR on accept");

    //creating a new child process whenever a new connection is made
		pid_t parentpid = getpid();
    pid_t spawnpid = -5;
    spawnpid = fork();
    if(parentpid != getpid()) {  //making sure we are in the child process

  	  // Get the message from the client and display it
      char message[10000];
      memset(message, '\0', 10000);
      memset(buffer, '\0', 1000);

      charsRead = recv(establishedConnectionFD, buffer, 1, 0); // Reading the first character sent in the message
      if (charsRead < 0) error("ERROR reading from socket");
      //printf("SERVER: I received this from the client: \"%s\"\n", buffer);
      if (buffer[0] == 'd') {  //checks that it is communicating with the correct file

        while(strstr(message, "##") == NULL) {  //## is used to signal the end of the message
          memset(buffer, '\0', 1000); //making sure our buffer is empty
        	charsRead = recv(establishedConnectionFD, buffer, 999, 0); // Read the client's message from the socket
        	if (charsRead < 0) error("ERROR reading from socket");
          strcat(message, buffer);
          //printf("SERVER: I received this from the client: \"%s\"\n", buffer);
        }

        char* message_part = strtok(message, "\n");   //this takes in the plaintext part of the message
        char* message_part2 = strtok(NULL, "#"); //this takes in the key part of the message
        char* plaintext = malloc(strlen(message_part) * sizeof(char));
        memset(plaintext, '\0', sizeof(plaintext));
        strcpy(plaintext, message_part);  //assigning plaintext

        char* key = malloc(strlen(message_part2) * sizeof(char));
        memset(key, '\0', sizeof(key));
        strcpy(key, message_part2); //assigning the key

        //here we start making the ciphertext
        int i=0;
        char* ciphertext = malloc((strlen(plaintext)+4) * sizeof(char));  //making ciphertext teh same size of plaintext with room for dc and ##
        memset(ciphertext, '\0', sizeof(ciphertext));
        strcat(ciphertext, "dc");  //addind dc to the beggining
        int char_val;
        int key_val;
        char output;
        //this loop is in charge of taking the plaintext and key and encripting the message
        for(i=0; i<strlen(plaintext); i++) {
          if(plaintext[i] == ' ') {   //changing our ' ' ascii value
            char_val = 91;
          }
          else {  //all other capital letters have the correct value
            char_val = plaintext[i];
          }
          char_val = char_val - 65;   //capitals go from 65 - 90 so this makes the range 0-26

          if(key[i] == ' ') {
            key_val = 91;
          }
          else {
            key_val = key[i];
          }
          key_val = key_val - 65;  //making range go from - 0-26

          char_val = char_val - key_val;  //subtracting key from the original value
          char_val = (char_val) % 27;  //modding by 27
          //printf("mod: %d\n", char_val);
          if(char_val < 0) {  //this takes into consideration negative numbers becasue otherewise it just returns the origiinal char_val
            char_val = 27 + char_val;
          }
          char_val = char_val + 65;  //changing ascii values back to capital letter range
          if(char_val == 91) {  //checking for spaces
            output = ' ';
          }
          else {
            output = char_val;
          }
          ciphertext[2+i] = output;  //setting the decripted character in cipher text
        }
        strcat(ciphertext, "##"); //adding ## to end


        int totalCharsRead = 0;
        while(totalCharsRead < strlen(ciphertext)) {
        	// Sending ciphertext back to the client
        	charsRead = send(establishedConnectionFD, ciphertext, 1000, 0);
        	if (charsRead < 0) error("ERROR writing to socket");
          totalCharsRead = totalCharsRead + charsRead;
        }
        close(establishedConnectionFD); // Close the existing socket which is connected to the client
      	close(listenSocketFD); // Close the listening socket
      	return 0;
      }
      else {
        //if first char isnt e
        charsRead = send(establishedConnectionFD, "JJ", 2, 0);
        if (charsRead < 0) error("ERROR writing to socket");
        close(establishedConnectionFD); // Close the existing socket which is connected to the client
        close(listenSocketFD); // Close the listening socket
        return 0;
      }
    }
  }
}
