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
/*//trashed function to search path for execurtable. Instead execvp returning -1 will cause the program to try ./executable
int isfile(char *path)//checks if argument is a file
{
	struct stat buf;
	stat(path, &buf);
	return S_ISREG(buf.st_mode)
}

int exec_in_path(char *name)
{
	char *path = getenv("PATH");
	char *item = NULL;
	int found	= 0;
	if (!path) 
		return 0;
	path = strdup(path);
	char fullpath[4096];
	for (item = strtok(path, ":"); (!found) && item; item = strtok(NULL, ":"))
		{
			sprintf(real_path, "%s/%s", item, name);
			// printf("Testing %s\n", real_path);
			if ( is_file(real_path) && !(access(real_path, F_OK) || access(real_path, X_OK))) // check if the file exists and is executable
			{
				found = 1;
			}
		}

	free(path);
	return found;
}
*/

/*int log(char **arg)
{
	dup2(sil,fileno(stdin));
}*/

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
		//printf("no exec");
		strcat(temp,arg[0]);
		printf("temp : %s\targ0 : %s\n",temp,arg[0]);
		if(execvp(temp,arg)==-1)
			perror("external exec error");
	}
		exit(-1);
	}
	else if (pid<0)
		perror("forking error");
	else do
	{
		waitpid(pid, &status, 0);
	} while(!WIFSIGNALED(status)&&!WCOREDUMP(status)&&!WIFEXITED(status));
	return 1;
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
		return external(arg);
}

int main(int argc, char **argv)
{
	char *input;
	char **arg;
	int ret;
	if(sil < 0)
	{
		fprintf(stderr,"\nsil file open ret -ve\n");
		return 1;
	}
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
