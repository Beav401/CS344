#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


struct Room {   //this is the struct room template I am using
  char name[8];
  char room_type[10];
  char connections[6][9];
  int connection_num;
};

//This function creates all the new files in the rooms directory as well as inseting the names into the rooms array of the 7 selected rooms
void create_room_file(struct Room* room_array, char* directory) {
  int test = 0;
  char rooms[10][9] = {"Red", "Blue", "Green", "Yellow", "Orange", "Black", "Pink", "White", "Purple", "Brown"}; //all possible names
  int random;
  char file_name[29] = "";
  int i=0;

  for (i = 0; i < 7; i++) {  //this will run 7 times for th 7 different files that need to be made

    test = 0;
    while(test == 0){  //this loop checks to make sure that no names are repeated in the 7
      random = (rand() % 10);  //comes up with random # between 0-9
        if(strcmp(rooms[random], "\0") != 0)  { //true if room is still available
          strcpy(room_array[i].name, rooms[random]);
          strcpy(rooms[random], "\0"); //this is used to get rid of used names
          test=1;
        }
        else {
          test = 0;
        }
    }

    //these are the hard-coded statements that write the file names
    if(random == 0) {
      sprintf(file_name,"%s/red_room", directory);  //adds the file name to the end of the directory
        open(file_name, O_CREAT, 0755);  //opens file with 0755 permission code
    }
    else if(random == 1) {
      sprintf(file_name,"%s/blue_room", directory);
        open(file_name, O_CREAT, 0755);
    }
    else if(random == 2) {
      sprintf(file_name,"%s/green_room", directory);
        open(file_name, O_CREAT, 0755);
    }
    else if(random == 3) {
      sprintf(file_name,"%s/yellow_room", directory);
        open(file_name, O_CREAT, 0755);
    }
    else if(random == 4) {
      sprintf(file_name,"%s/orange_room", directory);
        open(file_name, O_CREAT, 0755);
    }
    else if(random == 5) {
      sprintf(file_name,"%s/black_room", directory);
        open(file_name, O_CREAT, 0755);
    }
    else if(random == 6) {
      sprintf(file_name,"%s/pink_room", directory);
        open(file_name, O_CREAT, 0755);
    }
    else if(random == 7) {
      sprintf(file_name,"%s/white_room", directory);
        open(file_name, O_CREAT, 0755);
    }
    else if(random == 8) {
      sprintf(file_name,"%s/purple_room", directory);
        open(file_name, O_CREAT, 0755);
    }
    else if(random == 9) {
      sprintf(file_name,"%s/brown_room", directory);
        open(file_name, O_CREAT, 0755);
    }
  }
}
//since rooms were randomly assigned to array it is random enough to just assign the first element the Start and last the End
void set_room_type(struct Room* room_array) {
  int i=0;
  for(i;i<7;i++) {
    if(i==0) //first element
      strcpy(room_array[i].room_type, "START_ROOM");
    else if(i==6) //last element
      strcpy(room_array[i].room_type, "END_ROOM");
    else //all other elements
      strcpy(room_array[i].room_type, "MID_ROOM");
  }

}




//Many of the function prototypes were taken from the 2.2 outline page

// Returns 1 if all rooms have 3 to 6 outbound connections, 0 otherwise
int IsGraphFull(struct Room* room_array) {
  int i=0;
  for(i; i<7; i++) {    //goes through all Rooms
    if(room_array[i].connection_num < 3) //checks if there are two few connections
      return 0;

  }
  return 1;
}

// Returns a random Room, does NOT validate if connection can be added
struct Room* GetRandomRoom(struct Room* room_array){
  int random = (rand() % 7);  //will be between 0-6
  return &room_array[random];  //randomly generated 7 room array that was made earlier
}

// Connects Rooms x and y together, does not check if this connection is valid
void ConnectRoom(struct Room* x, struct  Room* y) {
  strcpy((*x).connections[(*x).connection_num], (*y).name);  //adds connection name to connections

  (*x).connection_num++; //increases number of connections
}

// Adds a random, valid outbound connection from a Room to another Room
void AddRandomConnection(struct Room* room_array) {
  struct Room* A;  // initializing two struct ptr's
  struct Room* B;

  while(1)  {   //will repeat until break is reached
    A = GetRandomRoom(room_array);

    if (CanAddConnectionFrom(A) == 1) {  //connection can be added
      break;
    }
  }

  do  {
    B = GetRandomRoom(room_array);
  }while(CanAddConnectionFrom(B) == 0 || IsSameRoom(A, B) == 1 || ConnectionAlreadyExists(A, B) == 1);
      //checks that this would create a valid connection

  ConnectRoom(A, B);  // Makes first Connection
  ConnectRoom(B, A);  // Makes connection back to first room
}

// Returns 1 if a connection can be added from Room x (< 6 outbound connections), 0 otherwise
int CanAddConnectionFrom(struct Room* x) {
  if( (*x).connection_num < 6)  //checks that max number of connections hasn't been reached
    return 1;
  else
    return 0;
}

// Returns 1 if a connection from Room x to Room y already exists, 0 otherwise
int ConnectionAlreadyExists(struct Room* x, struct Room* y) {
  int i;
  int comparison;
  for (i = 0; i < (*x).connection_num; i++) {   //goes through all rooms
    comparison = strcmp((*x).connections[i], (*y).name);
    if (comparison == 0) {  //true if the connection already exists
      return 1;
    }
  }
  return 0;
}

// Returns 1 if Rooms x and y are the same Room, 0 otherwise
int IsSameRoom(struct Room* x, struct Room* y) {
  int copy = strcmp((*x).name, (*y).name); //compares room names
  if(copy == 0) // true if names are the same
    return 1;
  else
    return 0;
}

//This populates the file that is sent in as an argument
void populate_single(struct Room* room_array, char* file_name, int i) {

  FILE* current_file;
  current_file = fopen(file_name, "w");  //open file with w permission
  fprintf(current_file, "Room Name: %s\n", room_array[i].name);  //printign room Name
  int j=0;
  for(j=0; j<room_array[i].connection_num; j++) { //for number of connections
    fprintf(current_file, "Connection %d: %s\n", (j+1), room_array[i].connections[j]); //print Connection
  }

  fprintf(current_file, "Room Type: %s\n", room_array[i].room_type); //print room Type
  fclose(current_file);  //close file
}



//checks whick rooms are in th room_array and then creates the file name based on those rooms
void populate_files(struct Room* room_array, char* directory) {
  int i=0;
  char file_name[29] = "";
  for( i=0; i<7; i++) { //for every room
    if(strcmp(room_array[i].name, "Red") == 0) {  //checks the name of the room
      sprintf(file_name,"%s/red_room", directory);  //creates the correct file name based on the room name
        populate_single(room_array, file_name, i);
    }
    //the following statements are the same but are just used for different room names
    else if(strcmp(room_array[i].name, "Blue") == 0) {
      sprintf(file_name,"%s/blue_room", directory);
        populate_single(room_array, file_name, i);
    }
    else if(strcmp(room_array[i].name, "Green") == 0) {
      sprintf(file_name,"%s/green_room", directory);
      populate_single(room_array, file_name, i);
    }
    else if(strcmp(room_array[i].name, "Yellow") == 0) {
      sprintf(file_name,"%s/yellow_room", directory);
      populate_single(room_array, file_name, i);
    }
    else if(strcmp(room_array[i].name, "Orange") == 0) {
      sprintf(file_name,"%s/orange_room", directory);
      populate_single(room_array, file_name, i);
    }
    else if(strcmp(room_array[i].name, "Black") == 0) {
      sprintf(file_name,"%s/black_room", directory);
      populate_single(room_array, file_name, i);
    }
    else if(strcmp(room_array[i].name, "Pink") == 0) {
      sprintf(file_name,"%s/pink_room", directory);
      populate_single(room_array, file_name, i);
    }
    else if(strcmp(room_array[i].name, "White") == 0) {
      sprintf(file_name,"%s/white_room", directory);
      populate_single(room_array, file_name, i);
    }
    else if(strcmp(room_array[i].name, "Purple") == 0) {
      sprintf(file_name,"%s/purple_room", directory);
      populate_single(room_array, file_name, i);
    }
    else if(strcmp(room_array[i].name, "Brown") == 0) {
      sprintf(file_name,"%s/brown_room", directory);
      populate_single(room_array, file_name, i);
    }

  }
}






void main() {
  char pid[6] = "";  //will store process id
  srand(time(NULL));  //allows for random generator
  char directory[20] = "";
  char placeholder[10] ="";

  pid_t pidNum = getpid();  //gets process ID
  sprintf(pid, "%d", pidNum);  //turns process id into a char array
  sprintf(directory, "boeshanr.rooms.%s", pid);  //appends if to end of directory name
  mkdir(directory, 0755); //Creates the directory that the files will be created in

  struct Room* room_array = malloc(7*sizeof(struct Room));  //generates our room_array on the heap
  create_room_file(room_array, directory); //initiates the files to be created
  set_room_type(room_array); //sets Start, mid, and end
  while (IsGraphFull(room_array) == 0) {     //creating the connection graph
    AddRandomConnection(room_array);
  }
//  strcpy(placeholder, room_array[0].connections[0]);
  //strcpy(room_array[0].room_type, "START_ROOM");  //sets the first room in the array as the start room which was randomly generated
  //strcpy(room_array[0].connections[0], placeholder);
  populate_files(room_array, directory);  //writes information to files
  free(room_array);
}
