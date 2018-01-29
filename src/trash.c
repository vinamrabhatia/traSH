#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>

bool active=0;
bool logging=0;

int entry(char **arg);
int ext(char **arg);
int log(char **arg);
int unlog(char **arg);
int viewcmdlog(char **arg);
int viewoutlog(char **arg);
int changedir(char **arg);

int changedir(char **arg)
{
  if (arg[1] == NULL)
    fprintf(stderr, "\"changedir\" expects arguments\n");
  else if (chdir(arg[1]) != 0)//chdir defined in unistd, changes current working dir to args[1]
    perror("chdir failed");//defined in stdio, prints str : error
  return 1;
}

int ext(char **arg)
{
  active=0;
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

int main(int argc, char **argv)
{
  char *input;
  char **arg;
  while(1)
  {
    printf("traSH $ ");
    input=in();
  }
}
