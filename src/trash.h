#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h> // for waitpid
#include <sys/stat.h> // for stat()
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include <stdbool.h>

void the_code();
