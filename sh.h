#include "get_path.h"


#include <dirent.h>
#include "watchuser_list.h"
#include "linked_list.h"


int pid;
char *which(char *command, struct pathelement *pathlist);
void list(char *dir);
void printenv(char **envp);

char *where(char *command, struct pathelement *pathlist);

#define PROMPTMAX 64
#define MAXARGS   16
#define MAXLINE   128
