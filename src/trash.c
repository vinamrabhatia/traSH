#include<stdio.h>

bool active=0;
bool logging=0;

int entry(char **arg);
int ext(char **arg);
int log(char **arg);
int unlog(char **arg);
int viewcmdlog(char **arg);
int viewoutlog(char **arg);
int changedir(char **arg);
