#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

struct Room {   //this is the struct room template I am using
  char name[8];
  char room_type[10];
  char connections[6][9];
  int connection_num;
};

//this function will assign directory the string of the most recent directory made
//this function was taken from slide 7 in everything is a file slideshow. some modifications were made
void getdirectory(char* directory) {
  int newestDirTime = -1; // Modified timestamp of newest subdir examined
    char targetDirPrefix[32] = "boeshanr.rooms."; // Prefix we're looking for
    char newestDirName[256]; // Holds the name of the newest dir that contains prefix
    memset(newestDirName, '\0', sizeof(newestDirName));

    DIR* dirToCheck; // Holds the directory we're starting in
    struct dirent *fileInDir; // Holds the current subdir of the starting dir
    struct stat dirAttributes; // Holds information we've gained about subdir

    dirToCheck = opendir("."); // Open up the directory this program was run in

    if (dirToCheck > 0) { // Make sure the current directory could be opened
      while ((fileInDir = readdir(dirToCheck)) != NULL) { // Check each entry in dir
        if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) { // If entry has prefix
          stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry

          if ((int)dirAttributes.st_mtime > newestDirTime) { // If this time is bigger
            newestDirTime = (int)dirAttributes.st_mtime;
            memset(newestDirName, '\0', sizeof(newestDirName));
            strcpy(newestDirName, fileInDir->d_name);
          }
        }
      }
    }

    closedir(dirToCheck); // Close the directory we opened
    strcpy(directory, newestDirName); //copies over to our directory variable

}

//this function will populate one of teh elements in the array based on which room file it is given
void populate_room(struct Room* room_array, char* directory, char* room_file, int i) {
  char file_path[40] = "";    //initializing char*'s to hold input from read
  char name[8];
  char connection[11];
  char connection_name[8] = "";
  char one_letter[2] = "\0\0";
  char room_type[12] = "";
  char room_type_holder[10] ="";

  room_array[i].connection_num = 0; //making sure connection_num is 0 as there shouldn't be any connections added yet
  memset(name, '\0', sizeof(name));
  memset(connection, '\0', sizeof(connection));

  int length = (strlen(room_file)-5); //this get the length of the name of teh room without the _room ending


  sprintf(file_path, "%s/%s", directory, room_file); //makes file path
  int current_file;
  current_file = open(file_path, O_RDONLY);  //open with read only
  lseek(current_file, 11, SEEK_SET);      //moves past Room Name:

  read(current_file, name, length); //reads in name
  strcpy(room_array[i].name, name);  //copies to array name
  lseek(current_file, 1, SEEK_CUR); //moves past newline character
  int j;
  for(j=0; j<6; j++) {  //for max number of connections
    read(current_file, connection, 10);
    if(strcmp(connection, "Connection") != 0)  //checks to make sure we don't get to "Room Type:", if we do it breaks out of loop as there are no more connections
      break;
    else {   //connection is there
      lseek(current_file, 4, SEEK_CUR);  //moves to begining of connection name
      read(current_file, one_letter, 1);  //reads first letter
            while (strcmp(one_letter, "\n") != 0) {   //reads one letter of name at a time since names are different lengths looking for the newline
        sprintf(connection_name, "%s%s", connection_name, one_letter); //appends letter to end of connetion Name
        read(current_file, one_letter, 1);
      }
      strcpy(room_array[i].connections[room_array[i].connection_num], connection_name);  //once name is completed it is added to one of the connections
      room_array[i].connection_num++;  //increase connection size
      strcpy(connection_name, "");
    }
    if(j==5) {
      lseek(current_file, 10, SEEK_CUR);
    }

  }

  //now we have moved past "Room Type:"
  lseek(current_file, 1, SEEK_CUR); //moves past space character
  read(current_file, room_type, 3); //reads first 3 characters as START is a different length than mid and end
  if(strcmp(room_type, "STA") == 0) {   //statement if the room type is start
    read(current_file, room_type_holder, 7);
    sprintf(room_type, "%s%s", room_type, room_type_holder);
    strcpy(room_array[i].room_type, "");  //inputs room_type into room_array


  }
  else {   //for mid and end rooms
    read(current_file, room_type_holder, 5);
    sprintf(room_type, "%s%s", room_type, room_type_holder);
    strcpy(room_array[i].room_type, room_type);  //inputs room_type into room_array

  }
  close(current_file); //close file

}



//this function will open the directory and check the files calling another function to populate teh array based on what files were found
void populate_array(struct Room* room_array, char* directory) {
  DIR* dirToCheck; // Holds the directory we're starting in
  struct dirent *fileInDir; // Holds the current subdir of the starting dir
  int i = -2;


  dirToCheck = opendir(directory); // Open up the directory this program was run in

  if (dirToCheck > 0) { // Make sure the current directory could be opened
    while ((fileInDir = readdir(dirToCheck)) != NULL) { // Check each entry in dir
      if (strstr(fileInDir->d_name, "red_room") != NULL) { // If entry is red_room
        populate_room(room_array, directory, "red_room", i);
      }
      else if (strstr(fileInDir->d_name, "blue_room") != NULL) { // If entry is blue_room
        populate_room(room_array, directory, "blue_room", i);
      }
      else if (strstr(fileInDir->d_name, "green_room") != NULL) { // If entry is green_room
        populate_room(room_array, directory, "green_room", i);
      }
      else if (strstr(fileInDir->d_name, "yellow_room") != NULL) { // If entry is yellow_room
        populate_room(room_array, directory, "yellow_room", i);
      }
      else if (strstr(fileInDir->d_name, "orange_room") != NULL) { // If entry is orange_room
        populate_room(room_array, directory, "orange_room", i);
      }
      else if (strstr(fileInDir->d_name, "black_room") != NULL) { // If entry si black_room
        populate_room(room_array, directory, "black_room", i);
      }
      else if (strstr(fileInDir->d_name, "pink_room") != NULL) { // If entry is pink_room
        populate_room(room_array, directory, "pink_room", i);
      }
      else if (strstr(fileInDir->d_name, "white_room") != NULL) { // If entry is white_room
        populate_room(room_array, directory, "white_room", i);
      }
      else if (strstr(fileInDir->d_name, "purple_room") != NULL) { // If entry is purple_room
        populate_room(room_array, directory, "purple_room", i);
      }
      else if (strstr(fileInDir->d_name, "brown_room") != NULL) { // If entry is brown_room
        populate_room(room_array, directory, "brown_room", i);
      }
      i++;
    }
  }
  closedir(dirToCheck); // Close the directory we opened

}



//checks to make sure user inputs a valid room connection returns 0 if valid and 1 if its not
int valid_input(struct Room* room_array, char* input, int current_num) {
  int i=0;
  for(i=0; i<room_array[current_num].connection_num; i++) {     //coes through all possible connetions
    if(strcmp(input, room_array[current_num].connections[i]) == 0)   //compares the input string with the connection name
      return 0;
  }
  return 1;
}


//this function is responsible for updating the path array as the user selects connectinos
char** get_path(char* input, int num_rooms_path, char** path) {
  char** new_path;
  int i=0;

  new_path = malloc((num_rooms_path+1) * sizeof(char*));   //dynamically allocating memory that is 1 element larger for teh added connection
  for (i = 0; i < (num_rooms_path+1); i++){
    new_path[i] = malloc((9) * sizeof(char));
  }
  for(i=0; i<num_rooms_path; i++) {
    strcpy(new_path[i], path[i]);   //copies original path into the new path
  }
  strcpy(new_path[num_rooms_path], input);  //adds new connection to end of path array
  free(path);   //free's memory from original path as we have created a new pointer in this functino
  return new_path;
}



//this function returns the index of the room that the player is currently at
int get_new_current(char* input, struct Room* room_array){
  int i=0;
  for(i=0; i<7;i++) {
    if(strcmp(input, room_array[i].name) == 0)   //checks what room mathces the input and returns that index
      return i;
  }

}


//This is the main function that controls the game
void game(struct Room* room_array/*, time_t t, struct tm* tmp*/) {

  time_t t;       //this is used so the system can print out the time if requested
  struct tm* tmp;
  time(&t);

  int num_rooms_path =  0; //number of connections player has passed through
  char* input;
  size_t input_size = 26;
  char** path;      //array that lists the rooms that have been visited in order
  int i, k, j, valid, repeat = 0;
  int current_num;
  char current_time[45];
  for(i=0;i<7;i++) {    //checks to find the start room so the game can start there
    if(strcmp(room_array[i].room_type, "MID_ROOM") != 0 && strcmp(room_array[i].room_type, "END_ROOM") != 0)
      current_num = i;
  }

  while (strcmp(room_array[current_num].room_type, "END_ROOM") != 0) {   //while the current room isn't the end room
    repeat = 0;
    while (repeat == 0) {   //while there is invalid input or the first time
      printf("\nCURRENT LOCATION: %s\n", room_array[current_num].name);
      printf("POSSIBLE CONNECTIONS:");
      for(i=0; i<room_array[current_num].connection_num; i++) {
        printf(" %s", room_array[current_num].connections[i]);    //prints connections
        if(i == (room_array[current_num].connection_num -1)) {    //checks for when we have reached the final connection so we can end with a "."
            printf(".\n");
        }
        else
          printf(",");
      }
      printf("WHERE TO? >");
      getline(&input, &input_size, stdin);  //this takes the user input and stores in the input variable
      k = 0;
      while((input)[k] != '\0') {   //this is used to get rid of the neww line character at the end of the input
              k++;
      }
      (input)[k-1] = '\0';  //turns new line into a end of line character
      valid = valid_input(room_array, input, current_num);   //check validity

      if(strcmp(input, "time") == 0) {  //if user requests time

        int current_file;
        //current_file = fopen("currentTime.txt", "w");  //open file with w permission
        current_file = open("currentTime.txt", O_CREAT | O_WRONLY, 0755);  //open file and creates it if it doesnt exist
        tmp = localtime(&t);
        strftime(current_time, 45, "%l:%M, %A, %B %d, %Y", tmp);  //this is how the time is laid out
        printf("\n%s\n", current_time);   //prints to user
        write(current_file, current_time, strlen(current_time));    //writes to the file
        close(current_file);
        repeat = 0;
      }

      else if(valid == 0) {  //if valid connection input
        if(num_rooms_path == 0) {  //when this is the first connection from the start room
          path = malloc(1 * sizeof(char*));
          path[0] = malloc((9) * sizeof(char));
          strcpy(path[0], input);
        }
        else {
          path = get_path(input, num_rooms_path, path);  //path now points to a new array that includes the added connetion
        }

        current_num = get_new_current(input, room_array);  //assigns new current room
        num_rooms_path++;   //add one to number of connections
        repeat = 1;
      }
      else {  //bad input
        printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
        repeat =0;
      }

    }

  }
  printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");  //if while loop is completed this will be printed first
  printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", num_rooms_path);
  for(i=0;i<num_rooms_path;i++){  //prints teh path to victory
    printf("%s\n", path[i]);
  }

free(path);  //freeing allocated memory
}


void main() {


//used for time request
  time_t t;
  struct tm* tmp;
  time(&t);


/*  pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_lock(myMutex);
  pthread_t* thread;
  pthread_create(time_thread, NULL, time, NULL);
*/

  char directory[40] = "";
  getdirectory(directory);  //assigns directory the newest directory made
  struct Room* room_array = malloc(7*sizeof(struct Room));  //generates our room_array on the heap
  populate_array(room_array, directory);  //populates the array that is made in the adventure file
  int i=0;
  int j=0;

  game(room_array);  //game functionality begins
  exit(0);   //exit with status code 0

}
