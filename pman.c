/*
 *	pman.c
 *
 *	CSC360 Assignment #1
 *	Name:		Mackenzie Cooper
 *	Student #:	V00892515
 *	Fall 2022
 *	
 *	Note: I took this course previously.
 *	As a result, some code has been recycled from a previous submission
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <sys/mman.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "csc360_list.h"


#define BUFFER_SIZE 1000
#define USER_BUFFER_MAX_SIZE 100
#define HOST_BUFFER_MAX_SIZE 100
#define CWD_BUFFER_MAX_SIZE 80
#define PROMPT_SIZE 300

void update_process()
{
	int status;
	int pid = waitpid(-1, &status, WNOHANG|WUNTRACED|WCONTINUED);
	if(pid > 0) {
		if(WIFCONTINUED(status)) {
			Node *n = find_node(pid);
			if(n) {
				n->run_state = 1;
			} else  if (WIFSTOPPED(status)) {
				Node *n = find_node(pid);
				if(n) {
					n->run_state = 0;
				}
			} else if (WIFEXITED(status)) {
				remove_node(pid);
				printf("%d Finished\n", pid);
			} else if (WIFSIGNALED(status)) {
				remove_node(pid);
				printf("%d Terminated\n", pid);
			}

		}
	}
}

int process_exists(int pid)
{
	char dir_buffer[BUFFER_SIZE];
	sprintf(dir_buffer, "/proc/%d", pid);
	DIR *dir = opendir(dir_buffer);
	if(!dir) {
		if (ENOENT == errno) {
			printf("error: process %d does not exist.\n", pid);
		} else {
			printf("error: process %d is unaccessible.\n", pid);
		}
		return 0;
	}
	return 1;
}

void bg_entry(char **argv, int arglength)
{
    //Input:        char ** array containing string args for process to run in background
	//Output:       None
	//runs a process in the background while parent continues to read input. Adds pid to linkedlist datastructure
	pid_t pid;
	pid = fork();
	if (pid == 0) {
		//in child process
		if (execvp(argv[1], &argv[1]) < 0) {
			perror("Error on execvp");
			exit(-1);
            return;
		}
	} else if (pid > 0) {
		//parent Process
		//store into our LL
		if (errno != ENOENT) {
			usleep(1000);
			Node *n = new_node(pid, argv[1], 1);
			head = add_front(head, n);
		}
	} else {
		//error with fork (pid < 0)
		perror("fork failed");
		exit(EXIT_FAILURE);
	}
    update_process();
}


void bg_list()
{
    //writes information from linked list to console (pid, process_name)
	Node *curr = head;
	int process_counter = 0;
	while (curr != NULL) {
		process_counter++;
		printf("%i:\t%s\n", curr->pid, curr->process_name);
		curr = curr->next;
	}
	printf("Total background jobs:\t%i\n", process_counter);

}

void bg_start(int pid)
{
    //sends CONT signal to target <pid>
	if (!process_exists(pid)) 
		return;
	if (kill(pid, SIGCONT)) {
		printf("error: failed to start process %d\n", pid);
	}

}


void bg_kill(int pid)
{
    //sends TERM signal to target <pid>
	if (!process_exists(pid)) {
        printf("Process not found");
		return;
    }
	Node *target = find_node(pid);
	if (target && target->run_state == 0) {
		bg_start(pid);
	}
	usleep(1000);
	if (kill(pid, SIGTERM)) {
		printf("error: failed to killed process %d\n", pid);
	}
}


void bg_stop(int pid)
{
    //sends STOP signal to target <pid>
	if (!process_exists(pid)) 
		return;
	if (kill(pid, SIGSTOP)) {
		printf("error: failed to stop process %d\n", pid);
	}

}


void ls_command(char **argv, int arglength)
{ //Uses execvp to fetch the cwd and print it to the user
    char *argument_list[arglength + 1];
	int i;
	for(i = 0; i < arglength; ++i) {
		argument_list[i] = argv[i];
	}
	argument_list[arglength] = NULL;
	int status;
	pid_t pid;
	pid = fork();
	if (pid == 0) {
		status = execvp("ls", argument_list);
		if(status == -1) {
			printf("Terminated Incorrectly\n");
		}
	} else if (pid > 0) {
		wait(&status);
	} else {
		perror("fork failed");
		exit(EXIT_FAILURE);
	}

}

void change_dir(char **args)
{
	if (args[1] == NULL || strcmp(args[1],"~") == 0) {
		chdir(getenv("HOME"));
	} else if (strcmp(args[1], "..") == 0){
		chdir("..");
	} else { 
		if(chdir(args[1]) != 0) {
			perror("chdir() failed");
		}
	}

}

char **string_token(char *s)
{ 
    //Input:    a string with spaces
    //Output: 2d array with substrings from s
    //Helper function for p_stat
    char *ptr = strtok(s, " ");
    char **args = (char **)malloc(BUFFER_SIZE * sizeof(char *));
    int i = 0;
    while (ptr != NULL) {
        args[i] = ptr;
        ptr = strtok(NULL, " ");
        i++;
    }
    return args;
}

void pstat_write(char **args_stat, int vol, int nonvol)
{
    //Input:    Array containing p_stat args to write
    //Output:   Console line args with information regaring to target process of p_stat
    printf("comm:\t%s\n", args_stat[1]);
    printf("state:\t%s\n", args_stat[2]);
    printf("utime:\t%s\n", args_stat[13]);
    printf("stime:\t%s\n", args_stat[23]);
    //printf("rss:\t%s\n", 
    printf("voluntary_ctxt_switches:\t%d\n", vol);
    printf("nonvoluntary_ctxt_switches:\t%d\n", nonvol);
}

int isdigit_string(char input[]) {
    int length = strlen(input);
    for(int i = 0; i < length; i++) {
        if(!isdigit(input[i])) {
            return 0;
        }
    }
    return 1;
}
int verify_input(char input[]) {
    if(!input) {
        printf("job not specified\n");
        return -1;
    } else if (!isdigit_string(input)) {
        printf("please specify an integer for pid\n");
        return 0;
    } else {
        return 1;
    }
}

void dispatch_command(char **args, int length)
{
	//manages function calls based on console command
	if (!(strcasecmp(args[0], "bglist"))) {
		bg_list();
	}
	else if (!(strcasecmp(args[0], "bg"))) {
        bg_entry(args, length);
	}
	else if (!(strcasecmp(args[0], "ls"))) {
		ls_command(args, length);
	}
	else if (!(strcasecmp(args[0], "cd"))) {
        printf("change directory command\n");
        
		if(length > 2){
			printf("Too many arguments for cd\n");
		} else {
			change_dir(args);	
		}
        
	}
	else if (!(strcasecmp(args[0], "exit"))) {
		printf("Exiting the program\n");
		exit(0);
	} else if (!(strcasecmp(args[0], "bgstop"))) {
		bg_stop(atoi(args[1]));
        //printf("bgstop command\n");
	} else if (!(strcasecmp(args[0], "bgstart"))) {
		bg_start(atoi(args[1]));
        //printf("bgstart command\n");
	} else if (!(strcasecmp(args[0], "bgkill"))) {
		bg_kill(atoi(args[1]));
        //printf("bgkill command\n");
	} else {
		printf("Command not recognized\n");
	}
}



int main()
{
    const int max_args = 50;
    char user_input_str[50];
    while (1) {
      printf("Pman: > ");
      fgets(user_input_str, 50, stdin); 
      char *input = strtok(user_input_str, " \n");
      if(input == NULL){
        continue;
      }
      char *args[max_args];
      int index = 0; //Tracks the number of arguments in an input cmd
      args[index] = input;
      index++;
      while(input != NULL){
        input = strtok(NULL, " \n");
        args[index]=input;
        index++;
      }
      dispatch_command(args, index);
      printf("\n");
    }
    free_list(head);
  return 0;
    /*
    int i, j;
    char *token = " ";
    char *prompt = "pman:   >";
    const int max_args = 100;
    while (1) {
        i = 0;
        char *input = NULL;
        char *iterToken;
        char *args[max_args];
        input = readline(prompt);
        if (!strcmp(input, "")) 
            continue;
        iterToken = strtok(input, token);
        for(j = 0; j < max_args; j++) {
            if(iterToken)
                i++;                //Tracks number of arguments
            args[j] = iterToken;    //grabs token
            iterToken = strtok(NULL, token);
        }
        dispatch_command(args, i);

    for(int k = 0; k < i; k++) {
        printf("%d:\t%s\n", k, args[k]);
    }

    }
    update_process();
    free_list(head);
    */
}
