#include "trash.h"

int active=0;
int logging=0;
//int isfile(char *path);
int entry(char **);
int ext(char **);
int log(char **);
int unlog(char **);
int viewcmdlog(char **);
int viewoutlog(char **);
int changedir(char **);
int external(char **);
int do_command(char **, int );

int changedir(char **arg)
{
	if (arg[1] == NULL)
		fprintf(stderr, "\"changedir\" expects arguments\n");
	else if (chdir(arg[1]) != 0)//chdir defined in unistd, changes current working dir to args[1]
		perror("chdir failed");//defined in stdio, prints str : error
	return 1;
}

int log(char **arg)
{
	logging=1;
	//dup2(sil,fileno(stdin));
}

int ext(char **arg)
{
	active=0;
}

int entry(char **arg)
{
	if(active)
	{
		printf("Already authenticated\n");
		return 0;
	}
	printf("Authentication required. Enter password:\n");
	char pass[80];
	gets(pass);
	if(strcmp(pass, "password")==0)
	{
		active=1;
		return 1;
	}
	return 0;
}

char *in(void)//reads input to buffer
{
	int bsz = 512;
	int pos = 0;
	char *buffer = malloc(sizeof(char) * bsz);
	int ch;

	if (!buffer)
	{
		fprintf(stderr, "Memory allocation for input buffer failed\n");
		exit(-1);
	}

	while (1)
	{
		ch = getchar();
		if (ch == EOF || ch == '\n')
		{
			buffer[pos] = '\0';
			return buffer;
		}
		else
		{
			buffer[pos] = ch;
		}
		pos++;
		if (pos >= bsz)
		{
			bsz += 512;
			buffer = realloc(buffer, bsz);
			if (!buffer)
			{
				fprintf(stderr, "extending memory allocation for input buffer failed\n");
				exit(-1);
			}
		}
	}
}

char **split(char *input)
{
	int bsz = 64;
	int pos=0;
	char *token;
	char **tokens = malloc(bsz * sizeof(char*));
	if(!tokens)
	{
		fprintf(stderr, "malloc fail\n");
		exit(-1);
	}
	strtok(input,"\t \a");//defined in string.h, used to split string into tokens
	while (token != NULL)
	{
		tokens[pos] = token;
		pos++;
		token=strtok(NULL, "\t \n");
	}
	tokens[pos] = NULL;
	return tokens;
}

int run(char **arg)
{
	if(!active&&strcmp(arg[0],"entry")!=0)
	{
		printf("Command line interpretter not started\n");
		return 1;
	}
	else if(arg[0] == NULL)
		return 1;
	else if(strcmp(arg[0],"changedir")==0)
		return changedir(arg);
	else if(strcmp(arg[0],"exit")==0)
		return ext(arg);
	else if(strcmp(arg[0],"entry")==0)
		return entry(arg);
	else if(strcmp(arg[0],"log")==0)
		return log(arg);
	else
		return do_command(arg, 1);
}

int do_command(char **args, int pipes) 
{
    // The number of commands to run
    const int commands = pipes + 1;
    int i = 0;

    int pipefds[2*pipes];

    for(i = 0; i < pipes; i++)
	{
        if(pipe(pipefds + i*2) < 0) 
		{
            perror("Couldn't Pipe");
            exit(EXIT_FAILURE);
        }
    }

    int pid;
    int status;

    int j = 0;
    int k = 0;
    int s = 1;
    int place;
    int commandStarts[10];
    commandStarts[0] = 0;

    // This loop sets all of the pipes to NULL
    // And creates an array of where the next
    // Command starts

    while (args[k] != NULL)
	{
        if(!strcmp(args[k], "|"))
		{
            args[k] = NULL;
            // printf("args[%d] is now NULL", k);
            commandStarts[s] = k+1;
            s++;
        }
        k++;
    }



    for (i = 0; i < commands; ++i) 
	{
        // place is where in args the program should
        // start running when it gets to the execution
        // command
        place = commandStarts[i];

        pid = fork();
        if(pid == 0) 
		{
            //if not last command
            if(i < pipes)
			{
                if(dup2(pipefds[j + 1], 1) < 0)
				{
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            //if not first command&& j!= 2*pipes
            if(j != 0 )
			{
                if(dup2(pipefds[j-2], 0) < 0)
				{
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            int q;
            for(q = 0; q < 2*pipes; q++)
			{
                    close(pipefds[q]);
            }       
            if( execvp(args[place], args+place) < 0 )
			{
                    perror(*args);
                    exit(EXIT_FAILURE);
            }
        }
        else if(pid < 0)
		{
            perror("error");
            exit(EXIT_FAILURE);
        }

        j+=2;
    }

    for(i = 0; i < 2 * pipes; i++)
	{
        close(pipefds[i]);
    }

    for(i = 0; i < pipes + 1; i++)
	{
        wait(&status);
    }
}

void teh_code(int argc, char **argv)
{
	char *input;
	char **arg;
	int ret;
	while(1)
	{
		printf("traSH $ ");
		input=in();
		arg=split(input);
		ret=run(arg);
		free(input);
		free(arg);
	}
}