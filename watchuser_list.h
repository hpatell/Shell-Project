#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utmpx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linked_list.h"

pthread_mutex_t lock;
void *watch_thread();
void checkList();
void init_thread();
void checkuser(char *username);
void checkLogOff(char *username);
