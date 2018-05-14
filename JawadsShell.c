/**
  * @author Md Jawad Mashrur Rahman
  */

/**
  * The following program constructs the author's unique version of a UNIX Shell. It accepts commands from the user and initiates them.
  */
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>

#define BUFSIZE 1024
#define BUFSIZE2 64
#define DELIM " \t\r\n\a"

char cwd[1024];
int background_flag =1;
char* userprompt = "308sh> ";
/**
 * Below are all the built-in commands that run on this shell
 */
int cd(char **args);
int my_exit(char **args);
int pid(char **args);
int ppid(char **args);
int pwd(char **args);
int get(char **args);
int set(char **args);

char *builtin_str[] = 
{
	"cd",
	"exit",
	"pid",
	"ppid",
	"pwd",
	"get",
	"set"
};

int (*builtin_func[]) (char **) = 
{
  	&cd,
  	&my_exit,
	&pid,
	&ppid,
	&pwd,
	&get,
	&set
};

int builtin_num() 
{
	return sizeof(builtin_str) / sizeof(char *);
}

/**
  * This function perfoms the change of directory, when cd is entered on the shell
  * if there are no arguments after cd, the shell takes the user to the home directory, 
  * else it takes the user to the desired directory according to the input
  */

int cd(char **args)
{
	char directory[100] = "";	
  	if (args[1] == NULL) 
	{
		chdir(getenv("HOME"));	
  	} 
	else 
	{
    		if (chdir(args[1]) != 0) 
	{
      		perror("err");
    	}
  }
  return 1;
}

/**
  * This function exits the shell when 'exit' is entered. 
  */

int my_exit(char **args)
{
	exit (EXIT_FAILURE);
	
}

/**
  * This function returns the process id of the shell, when 'pid' is entered.  
  */

int pid(char **args)
{
	printf("The PID of the shell is %i \n", getpid());
	return 1;
}

/**
  * This function returns the process id of the parent of the shell, when 'pid' is entered.  
  */

int ppid(char **args)
{
	printf("The PPID of the shell is %i \n", getppid());
	return 1;
}

/**
  * This function shows the current working directory, when 'pid' is entered. 
  * If the environment variable is NULL, returns -1.   
  */
int pwd(char **args)
{
	char * environment;
  	if((environment = getenv("PWD")) == NULL)
  	{
      		fprintf(stderr,"%s: Fail pwd \n",args[0]);
      		return -1;
  	}
  	else
	{
      		printf("%s \n",environment);
	}
	return 1;
}

/**
  * Returns the environment variable 
  */

int get(char **args)
{
	if(args[1] == NULL)
	{
		printf("please enter a variable");
	}
	if(getenv(args[1]) == NULL)
	{
		printf("unable to find variable\n");
	}
	else
	{
		printf("%s\n", getenv(args[1]));
	}
	return 1;
}
/**
  * Sets an environment variable  
  */
int set(char **args)
{
	if(args[1] == NULL)
	{
		printf("please enter a variable");
	}
	else
	{
		setenv(args[1], args[1], 1);
	}
	return 1;
} 

/**
  * Reads the line from the command prompt
  */

void *readline(void)
{	//just calling the getline method
	char *line = NULL;
	ssize_t bffrsize = 0;
  	getline(&line, &bffrsize, stdin);
  	return line;
}

/**
  * Splits the line from the command prompt into various arguments. 
  */

char **splitline(char *line)
{
  	int bffrsize = BUFSIZE2; 
	int pos = 0;
  	char **delims = malloc(bffrsize * sizeof(char*));
  	char *delim;

  	if (!delims) 
	{
    		fprintf(stderr, "allocation error\n");
    		exit(EXIT_FAILURE);
  	}

  	delim = strtok(line, DELIM);
  	while (delim != NULL) 
	{
    		delims[pos] = delim;
    		pos++;

    	if (pos >= bffrsize) 
	{
      		bffrsize += BUFSIZE2;
      		delims = realloc(delims, bffrsize * sizeof(char*));
      	if (!delims) 
	{
        	fprintf(stderr, "allocation error\n");
        	exit(EXIT_FAILURE);
      	}
    }

    	delim = strtok(NULL, DELIM);
  }
  	delims[pos] = NULL;
  	return delims;
}

/* This function is used for functions that are not builtin */

int launch(char **args)
{
 	pid_t pid;
	pid_t wpid;
  	int status;

  	pid = fork();
  	if (pid == 0)
	{
    // Child process
    		if (execvp(args[0], args) == -1) 
		{
      			perror("err");
    		}
    		exit(EXIT_FAILURE);
  	}
	else if (pid < 0) 
	{
    	// Error forking
    		perror("err");
  	} 
	else 
	{
   	 // Parent process
    	do 
	{
      		wpid = waitpid(pid, &status, WUNTRACED);
    	} while (!WIFEXITED(status) && !WIFSIGNALED(status));
  	}

  	return 1;
}


int execute(char **args)
{
	//CHECK FOR BUILT IN COMMAND AND IF NOT DO REGULAR COMMANDS
  	int i;

  	if (args[0] == NULL) 
	{
    	//empty command
    		return 1;
  	}

  	for (i = 0; i < builtin_num(); i++) 
	{
    		if (strcmp(args[0], builtin_str[i]) == 0) 
		{
      			return (*builtin_func[i])(args);
    	}
  	}
	//not builtin commands
  return launch(args);
}

/**
  * This is the command loop. In the infinite loop, it reads the line from the shell, 
  * splits the line into arguments and executes those arguemtents by calling various methods. 
  */

void cmd_loop(void)
{
	char *line;
  	char **args;
  	int status;
	
	
  while(1) 
{
		printf("%s" , userprompt);
    	line = readline();
    	args = splitline(line);
    	status = execute(args);

    	free(line);
    	free(args);
  }
}


int main(int argc, char **argv)
{

	if(argc == 3)
	{
		if((strncmp("-p", argv[1], 2) != 0)||(strlen(argv[1]) != 2))
		{
			printf("Invalid arguments\n");
			return 0;
		}
		
		userprompt = argv[2];
	}
	//incomplete or to many arguments
	if(argc != 3 && argc != 1)
	{
		printf("Invalid number of arguments \n");
		return 0;
	}

	cmd_loop();
	return EXIT_SUCCESS;
}

