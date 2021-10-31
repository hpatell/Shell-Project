#include "get_path.h"

char *where(char *command, struct pathelement *p)
{
  char cmd[64], *ch;
  int  found;

  found = 0;
  while (p) {
    sprintf(cmd, "%s/%s", p->element, command);
    if (access(cmd, X_OK) == 0) {
      printf("%s\n", cmd);
      found = 1;
    }
    p = p->next;
  }
  if (found) {
    ch = malloc(strlen(cmd)+1);
    strcpy(ch, cmd);
    return ch;
  }
  else
    return (char *) NULL;
}

