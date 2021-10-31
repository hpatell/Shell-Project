#ifndef header
#define header
 
#include <string.h>
#include <stdio.h>
pthread_t watch_user;
int countC();
struct nodeC
{
    char* data;
    struct nodeC *next;
}*headC;
struct nodeC *userDisplayList;
void appendC(char* num);
void addC( char* num );
void addafterC(char* num, int loc);
void insertC(char* num);
void deleteC(char* num);
void  displayC(struct nodeC *r);
void insertC2(char *name);
void deleteC2(char *name);
void freeallC();

#endif
