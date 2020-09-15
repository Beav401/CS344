#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//this is our main function which prints the random characters to stdout
int main(int argc, char* argv[]) {
  srand(time(0));  //gets time so that rand() produces different outputs each run
  int num_chars = 0;  //initializing variables
  int i=0;
  char random_char;
  int random_num = 0;


  num_chars = atoi(argv[1]);  //this changes the string of number into the correct int
  for(i=0; i<num_chars; i++) {  //for the number of characters chosen
    random_num = rand();  //take random number
    random_num = (random_num % 27) + 65;   //this gets that random number int the range of 65-91
    if(random_num == 91) { //ascii value 91 isnt ' ' so we set that here
      printf(" ");
    }
    else {  //this is for all capital letters
      random_char = random_num;   //turning int to char based on ascii table
      printf("%c", random_char);
    }
  }
  printf("\n");  //adding new line to end
  return 0;
}
