#include "trash.h"

volatile bool running = 0;
int active=1;
int logging=0;
int pipc;
//int isfile(char *path);
int entry(char **);
int ext(char **);
int logz(char **);
int unlog(char **);
int viewcmdlog(char **);
int viewoutlog(char **);
int changedir(char **);
int external(char **);
int do_command(char **, int );
void tee(const char*); 

int changedir(char **arg)
{
	if (arg[1] == NULL)
		fprintf(stderr, "\"changedir\" expects arguments\n");
	else if (chdir(arg[1]) != 0)//chdir defined in unistd, changes current working dir to args[1]
		perror("chdir failed");//defined in stdio, prints str : error
	return 1;
}

void tee(const char* fname) 
{
	int pipe_fd[2];

	pipe(pipe_fd);
	const pid_t pid = fork();
	if(!pid) 
	{
		close(pipe_fd[1]); // Close unused write end
		FILE* logFile = fname? fopen(fname,"a"): NULL;
		if(fname && !logFile)
    		printf("err\n");//fprintf(stderr,"cannot open log file \"%s\": %d (%s)\n",fname,0,strerror(errno));
    	char ch;
    	while(read(pipe_fd[0],&ch,1) > 0) 
		{
    		putchar(ch);
    		if(logFile)
    			fputc(ch,logFile);
    		if('\n'==ch) 
			{
    			fflush(stdout);
    			if(logFile)
    				fflush(logFile);
    		}
    	}
    	putchar('\n');
    	close(pipe_fd[0]);
    	if(logFile)
    		fclose(logFile);
    	exit(EXIT_SUCCESS);
    } 
	else 
	{
    	close(pipe_fd[0]); // Close unused read end
    	// redirect stdout and stderr
    	dup2(pipe_fd[1],STDOUT_FILENO);  
    	dup2(pipe_fd[1],STDERR_FILENO);  
    	close(pipe_fd[1]);  
    }
}

int logz(char **arg)
{
	logging=1;
	//dup2(sil,fileno(stdin));
}

int unlog(char **arg)
{
	logging=0;
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
	pipc=0;
	if (!buffer)
	{
		fprintf(stderr, "Memory allocation for input buffer failed\n");
		exit(-1);
	}

	while (1)
	{
		//fflush(stdout);
		//system("stty -echo");
		ch = getchar();
		//system("stty echo");
		if(ch==EOF && !running)
		{
			putchar('\n');
			exit(0);
		}
		if(ch=='|')
		{
			pipc++;
		}
		if (ch == '\n')
		{
			buffer[pos] = '\0';
			//printf("\n");
			return buffer;
		}
		else if(ch == '\t'){}
		else
		{
			buffer[pos] = ch;
			//printf("%c",ch);
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
	token=strsep(&input,"\t \a");//defined in string.h, used to split string into tokens

	while (token != NULL)
	{
		tokens[pos] = token;
		pos++;
		token=strsep(&input, "\t \n");
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
		return logz(arg);
	else if(strcmp(arg[0],"unlog")==0)
		return unlog(arg);
	else
		return do_command(arg, pipc);
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

    // This loop sets all of the pipes to NULL And creates an array of where the next Command starts

    while (args[k] != NULL)
	{
        if(!strcmp(args[k], "|"))
		{
            args[k] = NULL;
            commandStarts[s] = k+1;
            s++;
        }
        k++;
    }



    for (i = 0; i < commands; ++i) 
	{
        place = commandStarts[i];

        pid = fork();
        if(pid == 0) 
		{
            //if not last command
			if(i==pipes&&logging)
			{
				int fd = open("out.log", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
				dup2(fd, 1);
				dup2(fd, 2);
				close(fd);
			}
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

void intHandler(int dummy) {
	if(!running) {
		printf("\ntraSH $ ");
		fflush(stdout);
	}
	return;
}

void the_code(int argc, char **argv)
{
	char *input;
	char **arg;
	int ret;
	signal(SIGINT, intHandler);
	while(1)
	{
		printf("traSH $ ");
		input=in();
		arg=split(input);
		running = 1;
		ret=run(arg);
		running = 0;
		free(input);
		free(arg);
	}
}
