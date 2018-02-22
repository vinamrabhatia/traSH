#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h> // for waitpid
#include<sys/stat.h> // for stat()
#include<fcntl.h>

bool active=0;
bool logging=0;

int isfile(char *path);
int entry(char **arg);
int ext(char **arg);
int log(char **arg);
int unlog(char **arg);
int viewcmdlog(char **arg);
int viewoutlog(char **arg);
int changedir(char **arg);
int external(char **arg);
int do_command(char **args, int pipes);

/*int sil = open("stdinlog.txt", O_APPEND | O_WRONLY);
int sol = open("stdoutlog.txt", O_APPEND | O_WRONLY);
int sel = open("stderrlog.txt", O_APPEND | O_WRONLY);*/

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
/*//replaced by function with piping support
int external(char **arg)
{
	pid_t pid;
	int status;
	pid=fork();
	char temp[64]="./";
	if(pid==0)
	{
		if(execvp(arg[0],arg)==-1)//execvp searches PATH for arg[0]
		{
		strcat(temp,arg[0]);
		if(execvp(temp,arg)==-1)
			perror("traSH error ");
	}
		exit(-1);
	}
	else if (pid<0)
		perror("trash ");
	else do
	{
		waitpid(pid, &status, 0);
	} while(!WIFSIGNALED(status)&&!WCOREDUMP(status)&&!WIFEXITED(status));
	return 1;
}*/

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

int main(int argc, char **argv)
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