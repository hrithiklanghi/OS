#include<stdio.h>
#include<sys/types.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<fcntl.h>

char cmd[128];		//to store command entered
char *buffer[1024];		//array of string pointers to store history of commands
int cpid;		//to store process id
int count=0;		//stores number of commands entered
char line[128];		//takes input of command

void parse(){		//function to remove all spaces from command
	int i=0, j=0;
     while (line[i] != '\0') {		//if not end of string
          if (line[i]== ' ' || line[i] == '\t' || line[i] == '\n'){		//if space found increment inputed command
             	i++;
         	}
         else {cmd[j] = line[i];		//if not a space copy in 'cmd'
         	i++;
         	j++;
         	}
              }
	return;
}

void prompt(){		//function to take input from user
	printf("prompt>");
	scanf(" %[^\n]s", line);
	buffer[count]=malloc(sizeof(char) * strlen(line));		//'count' pointer from 'buffer' points to malloced memory of size of inputed command
	strcpy(buffer[count], line);		//copy inputed command in malloced momory
	count+=1;
	parse();
	return;
}

void fork_fun(){
	cpid=fork();
	if(cpid==0){
		char *args[2];
		args[0]=cmd;
		args[1]=NULL;
		if(execvp(cmd, args)<0){
			printf("Command not found : %s\n", cmd);
		};
		}
	else{wait(NULL);}
	return;
}

void input_redir(){
	char split_cmd[128];
	strcpy(split_cmd, cmd);		//make a copy of command
	
	char * token=strtok(split_cmd, "<");		//find '<' and point string before it
	token = strtok(NULL, " ");		//point string after '<' before end

	cpid=fork();
	if(cpid==0){
		char *split=strtok(cmd, "<");		//again find '<' and point string before it
		
		int fd=open(token, O_RDONLY);		//open inputed file with read permissions
		if(fd<0){
			printf("Error opening %s\n", token);
		}
		close(0);
		dup(fd);
		
				
		char *args[2];
		args[0]=split;
		args[1]=NULL;
		if(execvp(split, args)<0){
			printf("Command not found : %s\n", split);
		};	
		}
	else{wait(NULL);}
	return;
}

void output_redir(){	
	char split_cmd[128];
	strcpy(split_cmd, cmd);		//make a copy of command
	
	char * token=strtok(split_cmd, ">");		//find '>' and point string before it
	token = strtok(NULL, " ");		//point string after '>' before end

	cpid=fork();
	if(cpid==0){
		char *split=strtok(cmd, ">");		//again find '>' and point string before it
			
		int fd=open(token, O_WRONLY);		//open inputed file with write permissions
		if(fd<0){
			printf("Error opening %s\n", token);}
		else{
			close(1);
			dup(fd);
			char *args[2];
			args[0]=split;
			args[1]=NULL;
			if(execvp(split, args)<0){
				printf("Command not found : %s\n", split);
			};
			}	
		}
	else{wait(NULL);}
	return;
}

void pipe_fun(){		//function handles single pipe
	int arr[2];
	if(pipe(arr)<0){
		printf("Error initializing pipe");
	}
	else{
		int pid=fork();
		if(pid==0){
			char split_cmd[128];
			strcpy(split_cmd, cmd);
			char * token=strtok(split_cmd, "|");		
			
			printf("%s", token);
			char *args[2];
			args[0]=token;
			args[1]=NULL;
			printf("%s", args[0]);
			
			close(1);
			dup(arr[1]);
			close(arr[0]);
			
			execvp(token, args);
		}
		else{
			wait(NULL);
			char * split=strtok(cmd, "|");
			split = strtok(NULL, " ");
			printf("%s", split);
			
			char *args1[2];
			args1[0]=split;
			args1[1]=NULL;
			
			printf("%s", args1[0]);
			
			close(0);
			dup(arr[0]);
			close(arr[1]);
			execvp(split, args1);
		}
	}
}

int main(int argc, char* argv[]){
	while(1){
	prompt();
	if(strchr(cmd,'<')!=NULL){
		input_redir();
	}
	else if(strchr(cmd,'>')!=NULL){
		output_redir();
	}
	else if(strchr(cmd,'|')!=NULL){
		pipe_fun();
	}
	else{
		fork_fun();
	}
	}
}
