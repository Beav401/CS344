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

//this function returns 0 if the string contains bad chars and 1 if it doesn't
int check_chars(char* string) {
  int i=0;
  for(i=0; i < (strlen(string)-1); i++) {  //loop through all chars
    if(string[i] == ' '){}
    else if((int)string[i] <= 90 && (int)string[i] >= 65){}
    else {  //bad character is presesnt
      return 0;
    }
  }
  return 1;
}

//much of this code was taken from client.c
int main(int argc, char *argv[]) {
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[1000];
  int plaintext_valid = 0, key_valid = 0;

  if (argc < 4) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args

  char plaintext[10000];   //making a string to holp plaintext
  size_t plaintext_size = sizeof(plaintext);
  int plaintextFD;
  plaintextFD = open(argv[1], O_RDONLY);
  memset(plaintext, '\0', sizeof(plaintext));  //emptying string
  read(plaintextFD, plaintext, plaintext_size);  //storing what is in file to the string
  close(plaintextFD);


  char key[strlen(plaintext)+1];   //making a string for key that is just 1 bigger than the plaintext as we dont need more chars than that
  size_t key_size = sizeof(key);
  int keyFD;
  keyFD = open(argv[2], O_RDONLY);
  memset(key, '\0', sizeof(key));
  read(keyFD, key, key_size);  //reading from file and storing into the key string
  close(keyFD);

  if((strlen(key)-1) < (strlen(plaintext)-1)) {  //checking if the key is shorter than plaintext
    fprintf(stderr, "CLIENT: ERROR, key length is shorter than plaintext\n");
    exit(1);
  }
  plaintext_valid = check_chars(plaintext);   //checking if either plaintext or the key contains bad characters
  key_valid = check_chars(key);
  if(plaintext_valid == 0 || key_valid == 0) {
    fprintf(stderr, "CLIENT: ERROR, key or plaintext contains invalid characters\n");
    exit(1);
  }

  key[strlen(key)-1] = '\0';  //ending key with '\0'

  //printf("before strcat\n" );
  char* sendMessage = malloc((strlen(key) + strlen(plaintext) + 5) * sizeof(char));
  memset(sendMessage, '\0', sizeof(sendMessage));
  strcat(sendMessage, "e");  //message starts with an e for encript
  strcat(sendMessage, plaintext); //followed by plaintext with a newline at the end
  strcat(sendMessage, key);  //followed by key
  strcat(sendMessage, "##");  //ended with ##


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
  while(totalCharsWritten < strlen(sendMessage)) {  //makes sure all characters are sent to teh server in case of large strings
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

  if(buffer[0] == 'e' && buffer[1] == 'c') {  //checks that message recieved is from otp_enc_d

    while(strstr(message, "##") == NULL) {  //runs till teh end of the string has bee reached
      //printf("in while send\n");
      memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
      charsRead = recv(socketFD, buffer, 999, 0); // Read data from the socket, leaving \0 at end
    	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
    	//printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
      strcat(message, buffer);
    }
    char* ciphertext = strtok(message, "#");   //removing # from the end of the ciphertext

    printf("%s\n", ciphertext);  //printing to stdout

  }
  else {   //used if message recieved starts with JJ indicating it is from teh wrong server
    fprintf(stderr,"Error, could not connect to opt_enc_d on this port\n");
    exit(2);
  }
	close(socketFD); // Close the socket
  free(sendMessage);
	return 0;
}
