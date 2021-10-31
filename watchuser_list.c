#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utmpx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "watchuser_list.h"

void init_thread(){
    pthread_create(&watch_user, NULL, watch_thread, "watch thread created");
}

void *watch_thread(){
    while(1){
    checkList();
    sleep(20);
    }
}

void checkList(){
    struct nodeC *temp=headC;
    pthread_mutex_lock(&lock);
    while(temp!=NULL){
        checkuser(temp->data);
        temp=temp->next;
    }
    struct nodeC *tempList=userDisplayList;
    while(tempList!=NULL){
        checkLogOff(tempList->data);
        tempList=tempList->next;
    }
    pthread_mutex_unlock(&lock);
}

void checkuser(char *username)
{
  struct utmpx *up;

  setutxent();			/* start at beginning */
  while (up = getutxent() )	/* get an entry */
  {
    if ( up->ut_type == USER_PROCESS )	/* only care about users */
    {
    if(strcmp(up->ut_user, username)==0) { /*check if user being watched matches*/
        struct nodeC *tempList=userDisplayList;
        int flag = 0;
        if(tempList==NULL){
            flag=0;
            } else {
                while(tempList!=NULL){
                    if(strcmp(username,tempList->data)==0){
                    flag=1;
                    }
                    tempList=tempList->next;
                }
            }
            if(!flag){
                insertC2(up->ut_user);
                printf("%s has logged on %s from %s \n", up->ut_user, up->ut_line, up ->ut_host);
            }
        }
          
          
      }
    }
    return;
}

void checkLogOff(char *username){
  struct utmpx *up;

  setutxent();                  /* start at beginning */
  int flag = 0;
  while (up = getutxent() )     /* get an entry */
  {
    if ( up->ut_type == USER_PROCESS )  /* only care about users */
    {
      if(strcmp(up->ut_user, username)==0) { /*check if user being watched matches*/
          flag=1;
      }
    }
  }
  if(!flag) {
    printf("%s has logged off\n",username);
    deleteC2(username);
  }
  return;
}

