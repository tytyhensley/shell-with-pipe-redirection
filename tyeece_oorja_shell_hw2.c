/*readInput and parseInput functions were taken from assignemnt 1 (https://github.com/tytyhensley/shell/blob/master/yousra_tyeece_shell_hw1.c), 
the piped fuction was implemeted with the help of this geekforgeeks tutorial (https://www.geeksforgeeks.org/making-linux-shell-c/)*/

#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h>
#include <sys/stat.h>
#include<readline/readline.h> 
#include<readline/history.h> 
#include <errno.h>
#include <dirent.h>
#include <ctype.h>
#include <fcntl.h>


void readInput();
int parseInput(char* input_array[100]);
int* checkRedir(char* user_input[80], int len);
int checkPipe (char* input_array[100], int len);
void runExit();

char user_input[80];
int redir[2]; // stores result of redirect check

int main(int argc, char const *argv[])
{
	int stat, numPipe;
	pid_t pid, p1, p2;
	char*  input_array[100];
	char* cmp = "|";
	char* ext ="exit";
	char* cd = "cd";
	printf("********UNIX SHELL*********\n");
	do{
		readInput();
		int elems = parseInput(input_array);

		if (strcmp(input_array[0],ext)==0){
			runExit();
		}
		checkRedir(input_array, elems);
		numPipe = checkPipe(input_array, elems);
		if (strcmp(input_array[0],cd)==0){
			if(chdir(input_array[1])!=0){
				printf("Failed to change directory\n");
			}
		}else if ((pid=fork())==0){//forks a new child process to deal with user comand
			if (redir[0] != 0) {
				int fd;
				// output redirection
				if (redir[0] == 1) { 
					if ((fd = open(input_array[redir[1]], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR)) < 0) {
						perror("error opening the file");
					};
					dup2(fd, 1); // replace stdout with new fd
					close(fd);
				}
				// input redirection
				else if (redir[0] == 2) {
					int fd = open(input_array[redir[1]], O_RDONLY);
					dup2(fd, 0); // replace stdin with new fd
					close(fd);
				}
				// error redirection
				else if (redir[0] == 3) {
					int fd = open(input_array[redir[1]], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
					dup2(fd, 2); // replace stderr with new fd
					close(fd);
				}
				char* cmd[100]; 
				// remove last two words from array (">" or "<" and name of fd)
				for (int i = 0; i < redir[1]-1; i++) { 
					cmd[i] = input_array[i];
				}
				cmd[redir[1]] = NULL; // end the array
				execvp(cmd[0], cmd);
				printf("execvp failed to run, check if command was misspelt or does not exist\n");
				exit(0);//exits the child process if execvp fails 
			}
			//pipes commands
			else if(numPipe != 0){//piped function
				int a=0, b=0;
				for (int x =0; x < numPipe; x++){
					int fd[2], cnt=0;// 1 is the write end and 0 is the read end 
					if (pipe(fd) < 0) { // creates a pipe of the int array fd
        				printf("Pipe could not be initialized"); 
        				exit(0); 
   					 }
   					 if ((p1=fork())==0){//forks child to deal with write end of pipe
   					 	close(fd[0]); //closes the read end 
        				dup2(fd[1], 1); 
        				close(fd[1]); 

        				char* cmd2[100];
        				while(strcmp(input_array[b], cmp) != 0){//parses the commands based on the pipes
        					cmd2[cnt] = input_array[b];
        					b++;
        					cnt++;
        				}
        				b=b+2;//skips the pipe and goes to next command
        				cmd2[cnt]=NULL;// ends the array
        				execvp(cmd2[0], cmd2);
        				printf("execvp failed to run command %d\n", x);
        				exit(0);//exits the child process if execvp fails 
					 }else if ((p2=fork())==0){// forks child to deal with the read end of the pipe
					 	close(fd[1]); //closes the write end 
        				dup2(fd[0], 0); 
        				close(fd[0]); 

        				a=b;//sets counter to the other piped command 
        				cnt=0;
        				char* cmd3[100];
        				while(strcmp(input_array[a], cmp) != 0){//parses the commands based on the pipes
        					cmd3[cnt] = input_array[a];
        					a++;
        					cnt++;
        				}
        				cmd3[cnt]=NULL;// ends the array
        				execvp(cmd3[0], cmd3);
        				printf("execvp failed to run command %d\n", x+1);
        				exit(0);//exits the child process if execvp fails 
					 } else{//wait for two children
					 	wait(NULL); 
            			wait(NULL);
            			exit(0);//exits the child process if execvp fails 
					 }
				}
			}
			// no redirection
			else {
					execvp(input_array[0], input_array);
					printf("execvp failed to run, check if command was misspelt or does not exist\n");//will be not be printed if execvp is done correctly
					exit(0);//exits the child upon execvp failure
				}
		}else{
			wait(&stat);//makes parent process wait until the child process is completely terminated
		}

	}while(1);
	return 0;
}

 /* take in command as user input*/
void readInput(){
  printf(">>: ");
  if (fgets(user_input,80,stdin) == NULL){//gets the line of input from user and checks if ctr+D was pressed; if yes: exits 
    printf("\nGoodbye!");
    exit(0);
  }
  user_input[strcspn(user_input, "\n")] = 0;//gets rid of any trailing new line character
}

 /* parse through user input and stores them in an array*/
int parseInput(char* input_array[100]){
  char * input_tokens = strtok(user_input," ");
  int element_counter = 0;

  while( input_tokens != NULL ) {
    input_array[element_counter] = input_tokens;
    element_counter++ ;

    input_tokens = strtok(NULL, " ");
  }
  input_array[element_counter]=NULL;//adds the null charcater to the end of array so it can work with execvp
  /*for (int i = 0; i<element_counter; i++) {
  	printf("%s\n", input_array[i] );
  }*/
  return element_counter; // return number of words to be used in other helper functions
}

int* checkRedir(char* input_array[100], int len) {// takes the array containing parsed command
	//int len = sizeof(input_array)/sizeof(input_array[0]);
	//int result[2]; 
	// **** returns an array with first element indicating direction, second element indicating location of fd
	char* outp = ">";
	char* inp = "<";
	char* err = "2>";
	int i;
	for (i = 0; i < len; i++) {
		if (strcmp(input_array[i], outp) == 0) {
			redir[0] = 1;
			redir[1] = i+1; // index of fd (located right after the "<" or ">")
			return redir;
		}
		if (strcmp(input_array[i], inp) == 0) {
			redir[0] = 2;
			redir[1] = i+1;
			return redir;
		}
		if (strcmp(input_array[i], err) == 0) {
			redir[0] = 3;
			redir[1] = i+1;
			return redir;
		}
	}
	redir[0] = 0;
	redir[1] = 0;
	return redir;
}

/*checks if there are any pipes in the command and returns the number of them present*/
int checkPipe (char* input_array[100], int len){
	char* cmp = "|";
	int i, numPipe=0;
	for (i =0;i < len; i++){
		if (strcmp(input_array[i], cmp) == 0){
			numPipe++;
		}
	}
	return numPipe;
}

void runExit(){//exits the shell when called
  printf("Goodbye!\n");
  exit(0);
}











