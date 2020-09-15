#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>


void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues


//This function is used to check that all characters are eiter capital or ' '.  It returns 1 if they are and 0 if there is a bad char
int check_chars(char* string) {
  int i=0;
  for(i=0; i < (strlen(string)-1); i++) {
    if(string[i] == ' '){}  //if char is' '
    else if((int)string[i] <= 90 && (int)string[i] >= 65){}  //if char is a capital
    else {  //bad char
      return 0;
    }
  }
  return 1;
}

//this is the main function that acts as the client (much of this code was taken from the client.c file provided for us)
int main(int argc, char *argv[]) {
  //initializing client and server info
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[1000];
  int plaintext_valid = 0, key_valid = 0;

  if (argc < 4) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args

  char plaintext[10000];  //max size of plaintext
  size_t plaintext_size = sizeof(plaintext);
  int plaintextFD;
  plaintextFD = open(argv[1], O_RDONLY);   //opening plaintext file provided
  memset(plaintext, '\0', sizeof(plaintext));  //setting the string to end of string chars
  read(plaintextFD, plaintext, plaintext_size);   //store whats in file to plaintext string
  close(plaintextFD);

  char key[strlen(plaintext)+1];   //making a key that is just bigger than the plaintext
  size_t key_size = sizeof(key);
  int keyFD;
  keyFD = open(argv[2], O_RDONLY);  //opening key file
  memset(key, '\0', sizeof(key));
  read(keyFD, key, key_size);    //storing necessary amount of key file into the string
  close(keyFD);

  if((strlen(key)-1) < (strlen(plaintext)-1)) {  //keylenght is shorter than plaintext
    fprintf(stderr, "CLIENT: ERROR, key length is shorter than plaintext\n");
    exit(1);
  }
  plaintext_valid = check_chars(plaintext);   //checking to make sure both string contain teh correct characters
  key_valid = check_chars(key);
  if(plaintext_valid == 0 || key_valid == 0) {
    fprintf(stderr, "CLIENT: ERROR, key or plaintext contains invalid characters\n");
    exit(1);
  }

  key[strlen(key)-1] = '\0';   //making sure the end of key has a end of string character

  char* sendMessage = malloc((strlen(key) + strlen(plaintext) + 5) * sizeof(char));  //making new string that will hold both the text and key
  memset(sendMessage, '\0', sizeof(sendMessage));
  strcat(sendMessage, "d");  //d is for decrementing and should only be accessed by dec server
  strcat(sendMessage, plaintext);
  strcat(sendMessage, key);
  strcat(sendMessage, "##");  //string is ended with ## so we can use a while loop to read in the message on the server

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");


	// Send message to server
  int totalCharsWritten = 0;
  while(totalCharsWritten < strlen(sendMessage)) {  //checks to see if all chars of message have been written
  	charsWritten = send(socketFD, sendMessage, 1000, 0); // Write to the server
  	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
    totalCharsWritten += charsWritten;

  }


	// Get return message from server
  char message[10000];
  memset(message, '\0', 10000);
  memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse

  charsRead = recv(socketFD, buffer, 2, 0); // Read data from the socket, leaving \0 at end
  if (charsRead < 0) error("CLIENT: ERROR reading from socket");

  if(buffer[0] == 'd' && buffer[1] == 'c') {   //checks if message is comming from the dec server

    while(strstr(message, "##") == NULL) { //while we havent recieved the whole message
      memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
      charsRead = recv(socketFD, buffer, 999, 0); // Read data from the socket, leaving \0 at end
    	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
    	//printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
      strcat(message, buffer);
    }
    char* ciphertext = strtok(message, "#");  //this gets rid of the ## at the end of the string

    printf("%s\n", ciphertext);  //printing to stdout

  }
  else { //when the message begins with JJ that means it was talking to the wrond server
    fprintf(stderr,"Error, could not connect to opt_dec_d on this port\n");
    exit(2);
  }
	close(socketFD); // Close the socket
  free(sendMessage);  //freeing dynammic memory
	return 0;
}
