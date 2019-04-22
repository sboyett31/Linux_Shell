#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

void analyzeInput(char*);
void executeSingleCommand(char*);
void executeRedirectionCommand(char*);
void executePipeCommand(char*);
void executeMultiCommand(char*);

int main(int argc, char* argv[])
{
	char userInput[100];
	
	while(strcmp(userInput, "exit") != 0)
	{		
		printf("sb0115$ ");
		gets(userInput);
		if(strcmp(userInput, "") != 0)
		{
			if(fork()==0)
			{
				analyzeInput(userInput);
			}
			else
			{	
				wait(0);
			}
		}

	}
	return 0;
}

void analyzeInput(char* userInput)
{	
	if(strchr(userInput, '|') == NULL && strchr(userInput, '>') == NULL)
	{
		//neither character is present
		executeSingleCommand(userInput);
	}
	else if(strchr(userInput, '>') != NULL && strchr(userInput, '|') == NULL)
	{
		//only '>' character is present
		executeRedirectionCommand(userInput);
	}
	else if(strchr(userInput, '>') == NULL && strchr(userInput, '|') != NULL)
	{
		//only '|' character is present
		executePipeCommand(userInput);
	}
	else if(strchr(userInput, '>') != NULL && strchr(userInput, '|') != NULL)
	{
		//both characters are present
		executeMultiCommand(userInput);
	}
	exit(0);
}


void executeSingleCommand(char* command)
{
	char* brokenInput[10];
	int counter = 0;	
	brokenInput[counter] = strtok(command, " ");
	
	while(brokenInput[counter] != NULL)
	{
		counter+=1;
		brokenInput[counter] = strtok(NULL, " ");
	}
	
	brokenInput[counter] = NULL;
	
	execvp(brokenInput[0], brokenInput);
}

void executeRedirectionCommand(char* command)
{
	char* splitInput[2];
	char* splitCommand[25];
	int counter = 0;
	
	splitInput[counter] = strtok(command, ">");
	while(splitInput[counter] != NULL)
	{
		counter+=1;
		splitInput[counter] = strtok(NULL, ">");
	}

	splitInput[counter] = NULL;
	
	//resetting counter in order to split first half of user input to use as command
	counter = 0;	
	splitCommand[counter] = strtok(splitInput[0], " ");
	while(splitCommand[counter] != NULL)
	{
		counter+=1;
		splitCommand[counter] = strtok(NULL, " ");
	}

	splitCommand[counter] = NULL;
	
	//remove leading whitespace of filename
	int i = 0;
	
	while(splitInput[1][i] != NULL)
	{
		splitInput[1][i] = splitInput[1][i + 1];
		i++;
	}
	
	//redirect childs stdout to second half of user input
	int outfile = open(splitInput[1], O_CREAT|O_RDWR|O_TRUNC,0644);
	dup2(outfile, 1);
	execvp(splitCommand[0], splitCommand);
	close(outfile);

}

void executePipeCommand(char* command)
{
	char* splitInput[2];
	char* splitCommand[10];
	char* splitCommand2[10];
	int myPipingDescriptors[2];
	int counter = 0;
	
	splitInput[counter] = strtok(command, "|");
	while(splitInput[counter] != NULL)
	{
		counter+=1;
		splitInput[counter] = strtok(NULL, "|");
	}

	splitInput[counter] = NULL;
	
	//resetting counter in order to split first half of user input to use as command
	counter = 0;	
	splitCommand[counter] = strtok(splitInput[0], " ");
	while(splitCommand[counter] != NULL)
	{
		counter+=1;
		splitCommand[counter] = strtok(NULL, " ");
	}

	splitCommand[counter] = NULL;
	
	counter = 0;
	splitCommand2[counter] = strtok(splitInput[1], " ");
	while(splitCommand2[counter] != NULL)
	{
		counter+=1;
		splitCommand2[counter] = strtok(NULL, " ");
	}
	
	splitCommand2[counter] = NULL;
	
	//Create pipe
	if(pipe(myPipingDescriptors)==-1)
	{
		printf("Error in calling the piping function\n");
		exit(0);
	}
	
	if(fork()==0)
	{	
		//redirect childs stdout to the write end of the pipe and close the read end
		dup2(myPipingDescriptors[1], 1);
		close(myPipingDescriptors[0]);
		//execute left half of user input
		execvp(splitCommand[0], splitCommand);
	}
	else
	{	
		if(fork()==0)
		{
			//redirect process stdin to the read end of the pipe and close the write end
			dup2(myPipingDescriptors[0], 0);
			close(myPipingDescriptors[1]);
			//execute right half of user input
			execvp(splitCommand2[0], splitCommand2);
		}
	}
	wait(0);
	close(myPipingDescriptors[0]);
	close(myPipingDescriptors[1]);
}


void executeMultiCommand(char* command)
{

	char* splitInput[3];
	char* leftInput[20];
	char* midInput[20];
	int counter = 0;
	
	int myPipingDescriptors[2];
	//Create pipe
	if(pipe(myPipingDescriptors)==-1)
	{
		printf("Error in calling the piping function\n");
		exit(0);
	}
	

	
	splitInput[counter] = strtok(command, "|>");
	while(splitInput[counter] != NULL)
	{
		counter+=1;
		splitInput[counter] = strtok(NULL, "|>");
	}
	
	splitInput[counter] = NULL;
	
	//resetting counter in order to split first part of user input to use as command
	counter = 0;	
	leftInput[counter] = strtok(splitInput[0], " ");
	while(leftInput[counter] != NULL)
	{
		counter+=1;
		leftInput[counter] = strtok(NULL, " ");
	}
	leftInput[counter] = NULL;
	
	//resetting counter in order to split middle part of user input to use as command
	counter = 0;
	midInput[counter] = strtok(splitInput[1], " ");
	while(midInput[counter] != NULL)
	{
		counter+=1;
		midInput[counter] = strtok(NULL, " ");
	}
	midInput[counter] = NULL;
	
	if(fork()==0)	//create child process
	{	

		//redirect childs stdout to the write end of the pipe and close the read end
		dup2(myPipingDescriptors[1], 1);
		//execute left half of user input
		execvp(leftInput[0], leftInput);
	}
	else{
		if(fork()==0)
		{
			//redirect processes stdin to the read end of the pipe and close the write end
			dup2(myPipingDescriptors[0], 0);
			close(myPipingDescriptors[1]);

			//remove leading whitespace of filename
			int i = 0;
			while(splitInput[2][i] != NULL)
			{
				splitInput[2][i] = splitInput[2][i + 1];
				i++;
			}

			//redirect processes stdout to the file specified in right part of input
			int outfile = open(splitInput[2], O_CREAT|O_RDWR|O_TRUNC,0644);
			dup2(outfile, 1);

			//execute middle part of user input
			execvp(midInput[0], midInput);
			close(outfile);
			
		}
		wait(0);
		close(myPipingDescriptors[0]);
		close(myPipingDescriptors[1]);
	}

}

