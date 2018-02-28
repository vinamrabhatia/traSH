#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h> // for waitpid
#include<sys/stat.h> // for stat()
#include<fcntl.h>

void teh_code(int, char **);
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