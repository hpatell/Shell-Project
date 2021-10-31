#include<stdio.h>
#include<stdlib.h>
#include "linked_list.h"
 
int countC();

void deleteC2(char* num)
{
    struct nodeC **indirect=&userDisplayList;
    struct nodeC *temp=NULL;
    while(*indirect!=NULL){
        if(strcmp((*indirect)->data,num)==0){
            temp=*indirect;
            *indirect=(*indirect)->next;
            free(temp);
        } else{
            indirect=&((*indirect)->next);
        }
    }

    return;
}
 
void insertC(char *name)
{
  struct nodeC *temp, *mp3;
  mp3 = (struct nodeC *) malloc(sizeof(struct nodeC));        // malloc space for MP3
  mp3->data = (char *) malloc(strlen(name) + 1);  // malloc space for name
  strcpy(mp3->data, name);                        // "assign" name via copy
  mp3->next = NULL;

  if (headC == NULL)
  {
    headC = mp3;               // add the first MP3
  }
  else
  {
    temp = headC;
    while (temp->next != NULL)
      temp = temp->next;
    temp->next = mp3;         // append to the tail/end
  }
}
  
void deleteC(char* num)
{
    struct nodeC **indirect=&headC;
    struct nodeC *temp=NULL;
    while(*indirect!=NULL){
        if(strcmp((*indirect)->data,num)==0){
            temp=*indirect;
            *indirect=(*indirect)->next;
            free(temp);
        } else{
            indirect=&((*indirect)->next);
        }
    }

    return;
}
 
void  displayC(struct nodeC *r)
{
    r=headC;
    if(r==NULL)
    {
    return;
    }
    while(r!=NULL)
    {
    printf("%s ",r->data);
    r=r->next;
    }
    printf("\n");
}
 
int countC()
{
    struct nodeC *n;
    int c=0;
    n=headC;
    while(n!=NULL)
    {
    n=n->next;
    c++;
    }
    return c;
}

void freeallC(){
    struct nodeC *temp=headC;
    while(headC!=NULL){
        temp=headC;
        headC=headC->next;
        free(temp->data);
        free(temp);
    }
    struct nodeC *tempList=userDisplayList;
    while(userDisplayList!=NULL){
        tempList=userDisplayList;
        userDisplayList=userDisplayList->next;
        free(tempList->data);
        free(tempList);
    }
}

void insertC2(char *name)
{
  struct nodeC *temp, *mp3;
  mp3 = (struct nodeC *) malloc(sizeof(struct nodeC));        // malloc space for MP3
  mp3->data = (char *) malloc(strlen(name) + 1);  // malloc space for name
  strcpy(mp3->data, name);                        // "assign" name via copy
  mp3->next = NULL;

  if (userDisplayList == NULL)
  {
    userDisplayList = mp3;               // add the first MP3
  }
  else
  {
    temp = userDisplayList;
    while (temp->next != NULL)
      temp = temp->next;
    temp->next = mp3;         // append to the tail/end
  }
}

