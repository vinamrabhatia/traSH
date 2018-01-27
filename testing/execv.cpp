#include<unistd.h>
#include<stdio.h>

int main(int argc, char* argv[])
{
	printf("argc : %d\nargv[0] : %s\nargv[1] : %s\nargv[2] : %s\nargv[3] : %s\n",argc,argv[0], argv[1], argv[2], argv[3]);
	char *args[]={"lp","-l",NULL};
	execv("/bin/ls.exe",argv);
}

