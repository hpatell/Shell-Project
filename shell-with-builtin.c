#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <glob.h>
#include <sys/wait.h>
#include "sh.h"
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include <sys/stat.h>
#include <fcntl.h>



void sig_handler(int sig)
{
  fprintf(stdout, "\n>> ");
  fflush(stdout);
}

void piping(char **right, char **left, char c);

int
main(int argc, char **argv, char **envp)
{
	char	buf[MAXLINE];
	char    *arg[MAXARGS];  // an array of tokens
	char    *ptr;
        char    *pch;
	pid_t	pid;
	int	status, i, arg_no, background, noclobber;
	int     redirection, append, pipe, rstdin, rstdout, rstderr;
	
	struct pathelement *pa, *tmp;
        char *cmd;
	char *cwd;
	char prompt[MAXLINE] = "";
	char	*eofChar;
	noclobber = 0;

	char **right, **left;

	int thread = 0;

        signal(SIGINT, sig_handler);
	signal(SIGTSTP, SIG_IGN);

	again:
	
	cwd = getcwd(NULL, 0);
  	if(strcmp(prompt, "") != 0)
  	{
    	  printf("%s ", prompt);
  	}
  	printf("[%s]", cwd);
  	free(cwd);

	fprintf(stdout, " >> "); /* print prompt (printf requires %% to print %) */
	fflush(stdout);
	while ((eofChar = fgets(buf, MAXLINE, stdin)) != NULL) {
		if (strlen(buf) == 1 && buf[strlen(buf) - 1] == '\n')
		  goto nextprompt;  // "empty" command line

		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = 0; /* replace newline with null */


		// no redirection or pipe
		redirection = append = pipe = rstdin = rstdout = rstderr = 0;
                // check for >, >&, >>, >>&, <, |, and |&
                if (strstr(buf, ">>&"))
                  redirection = append = rstdout = rstderr = 1;
                else
                if (strstr(buf, ">>"))
                  redirection = append = rstdout = 1;
                else
                if (strstr(buf, ">&"))
                  redirection = rstdout = rstderr = 1;
                else
                if (strstr(buf, ">"))
                  redirection = rstdout = 1;
                else
                if (strstr(buf, "<"))
                  redirection = rstdin = 1;
                else
                if (strstr(buf, "|&"))
                  pipe = rstdout = rstderr = 1;
                else
                if (strstr(buf, "|"))
                  pipe = rstdout = 1;


		// parse command line into tokens (stored in buf)
		arg_no = 0;
                pch = strtok(buf, " ");
                while (pch != NULL && arg_no < MAXARGS)
                {
		  arg[arg_no] = pch;
		  arg_no++;
                  pch = strtok (NULL, " ");
                }
		arg[arg_no] = (char *) NULL;

		if (arg[0] == NULL)  // "blank" command line
		  goto nextprompt;

		background = 0;      // not background process
                if (arg[arg_no-1][0] == '&')
                  background = 1;    // to background this command
		
		/*
		// print tokens
		for (i = 0; i < arg_no; i++)
		  printf("arg[%d] = %s\n", i, arg[i]);
                */
                
	        // built-in command exit	
                if (strcmp(arg[0], "exit") == 0)  
		{
		  printf("Executing built-in [exit]\n");

		  pthread_cancel(watch_user);
		  pthread_join(watch_user,NULL);

		  exit(0);
		}

		// built-in command pwd
                else
                if (strcmp(arg[0], "pwd") == 0) 
		{ 
		  printf("Executing built-in [pwd]\n");
	          ptr = getcwd(NULL, 0);
                  printf("%s\n", ptr);
                  free(ptr);
	        }

		// built-in command which
		else
                if (strcmp(arg[0], "which") == 0) 
		{                    
		  printf("Executing built-in [which]\n");

		  if (arg[1] == NULL) {  // "empty" which
		    printf("which: Too few arguments.\n");
		    goto nextprompt;
                  }

		  pa = get_path();
           /***/
		  tmp = pa;
		  while (tmp) {      // print list of paths
		    printf("path [%s]\n", tmp->element);
		    tmp = tmp->next;
                  }
           /***/

                  cmd = which(arg[1], pa);
                  if (cmd) {
		    printf("%s\n", cmd);
                    free(cmd);
                  }
		  else               // argument not found
		    printf("%s: Command not found\n", arg[1]);

		  while (pa) {   // free list of path values
		     tmp = pa;
		     pa = pa->next;
		     free(tmp->element);
		     free(tmp);
                  }
	        }
               		
		// built-in command where
                else
                if (strcmp(arg[0], "where") == 0) 
		{
		  printf("Executing built-in [where]\n");

                  if (arg[1] == NULL) {  // "empty" where
                    printf("where: Too few arguments.\n");
                    goto nextprompt;
                  }

                  pa = get_path();

		  tmp = pa;
                  while (tmp) {      // print list of paths
                    printf("path [%s]\n", tmp->element);
                    tmp = tmp->next;
                  }
		  
		  cmd = where(arg[1], pa);
		  if (cmd) {
                    printf("%s\n", cmd);
                    free(cmd);
                  }

                  while (pa) {   // free list of path values
                     tmp = pa;
                     pa = pa->next;
                     free(tmp->element);
                     free(tmp);
                  }
                }

		// built-in command cd
		else
		if (strcmp(arg[0], "cd") == 0)
                {
		  printf("Executing built-in [cd]\n");
		  char *pa;
		  
		  if(arg_no == 1)
                  {
                    chdir(getenv("HOME"));
                  }
		  else if(arg_no == 2)
		  {
		    if(strcmp(arg[1], "-") == 0)
		    {
		      chdir("..");  // previous directory
		    }
		    else
                    {
		      pa = arg[1];
                      if(chdir(pa) < 0)
                      {
                        printf("%s: No such file or directory.\n", pa);
                      }  
		    }
		  }
		  else if(arg_no == 3)
		  {
		    printf("Too many arguments.\n");
		  }
                }
		
		// built-in command list
		else
                if (strcmp(arg[0], "list") == 0)
                {
                  printf("Executing built-in [list]\n");
                  struct dirent *de;
                  DIR *dr;

		  if(arg_no == 1)
                  {
		    dr = opendir(".");
		    if(dr == NULL)
		    {
		      printf("Can't open current directory\n");
		    }
		    else
		    {
		      while((de = readdir(dr)) != NULL)
		      {
			printf("%s\n", de->d_name);
		      }
		      closedir(dr);
		    }
                  }
		  else
		  {
		    for(int i = 1; i <= arg_no; i++) 
		    {
                      if(arg[i] != NULL) 
		      {
			printf("\n");
                        printf("[%s]:\n", arg[i]);
                        dr = opendir(arg[i]);
                    	if(dr == NULL)
                    	{
                      	  printf("Can't open current directory\n");
                    	}
                    	else
                    	{
                      	  while((de = readdir(dr)) != NULL)
                      	  {
                            printf("%s\n", de->d_name);
                      	  }
			  closedir(dr);
                    	}  
                      }
		    }
                  }
		}

		// built-in command pid
                else
                if (strcmp(arg[0], "pid") == 0)
                {
                  printf("Executing built-in [pid]\n");
                  printf("Shell pid: %d\n", getpid());
                }
		
		// built-in command kill
		else
                if (strcmp(arg[0], "kill") == 0)
                {
                  printf("Executing built-in [kill]\n");
		  int r;  // kill(pid, sig_no);
		  char *p;
		  r = 0;
		  //printf("%d\n", argc);
		  //printf("%d\n", arg_no);

		  if(arg_no == 2)
		  {
		    r = kill(atoi(arg[1]), SIGTERM);  // kill pid == kill -TERM pid
		  }
		  else if(arg_no == 3 && arg[1][0] == '-')  // kill -SIG pid
		  {
		    p = arg[1]+1;
		    r = atoi(p);
		    printf("signal # [%d]\n", r);
		    r = kill(atoi(arg[2]), r);  // kill -SIG_NO pid
		  }
		  else
		  {
		    printf("kill: syntax error\n");
		  }
		  printf("return from kill %d with error code %d (%d)\n", r, errno, ESRCH);
		  perror(arg[0]);
		  if(errno == ESRCH)
		  {
		    printf("*** No such process ***\n");
		  }
                }
		
		// built-in command prompt
		else
                if (strcmp(arg[0], "prompt") == 0)
                {
		  printf("Executing built-in [prompt]\n");

		  strcpy(prompt, "");

		  if(arg[1] == NULL)
		  {
		    printf("input prompt prefix: ");
		    fgets(prompt, MAXLINE, stdin);
		    if(prompt[strlen(prompt) - 1] == '\n')
		    {
		      prompt[strlen(prompt) - 1] = 0;
		    }
		  }
		  else
		  {
		    strcpy(prompt,arg[1]);
		  }
		}
	        
		// built-in command printenv
		else
                if (strcmp(arg[0], "printenv") == 0)
                {
		  if(arg_no == 1)
		  {
	            printf("Executing built-in [printenv]\n");
		    int i = 0;
		    while(envp[i] != NULL) 
		    {
		      printf("%s\n", envp[i]);
		      i++;
		    }
		  }
		  else if(arg_no == 2)
		  {
		    if(getenv(arg[1]) != NULL)
		    {
		      printf("Executing built-in [printenv]\n");
		      printf("%s\n", getenv(arg[1]));
		    }
		  }
		  else
		  {
		    fprintf(stderr, " No PATH variable set.\n");
		  }
                }
		
		// built-in command setenv
		else
                if (strcmp(arg[0], "setenv") == 0)
                {
                  if(arg_no == 1)
                  {
                    printf("Executing built-in [setenv]\n");
                    int i = 0;
                    while(envp[i] != NULL)
                    {
                      printf("%s\n", envp[i]);
                      i++;
                    }
                  }
                  else if(arg_no == 2)
                  {
	            printf("Executing built-in [setenv]\n");
                    setenv(arg[1], "", 1);
                  }
		  else if(arg_no == 3)
                  {
	            printf("Executing built-in [setenv]\n");
                    setenv(arg[1], arg[2], 1);
                  }
                  else
                  {
                    fprintf(stderr, "Too many arguments\n");
                  }
                }
		
		// built-in command noclobber
		else
                if (strcmp(arg[0], "noclobber") == 0) {
                  printf("Executing built-in [noclobber]\n");
                  noclobber = 1 - noclobber; // switch value
                  printf("%d\n", noclobber);
                }
		
		// built-in command echo
		else
                if (strcmp(arg[0], "echo") == 0) {
                  printf("Executing built-in [echo]\n");
                /***
                  for (i = 0; i < arg_no; i++)
                    printf("arg[%d] = %s\n", i, arg[i]);
                ***/
                  printf("%s\n", arg[1]);
                }
		
		// built-in command watchuser
                else
                if (strcmp(arg[0], "watchuser") == 0) 
		{
		    if(arg[1]!=NULL && arg[2]!=NULL && strcmp(arg[2],"off")==0){
				if(thread==0){
				init_thread();
				thread=1;
			}
				printf("Executing built- in [watchuser] removing user %s\n",arg[2]);
				pthread_mutex_lock(&lock);
				deleteC(arg[1]);
				pthread_mutex_unlock(&lock);				
			} else if(arg[2]==NULL && arg[1]!=NULL){
				if(thread==0){
				init_thread();
				thread=1;
			}
				printf("Executing built-in [watchuser] for user %s\n",arg[1]);
				pthread_mutex_lock(&lock);
				insertC(arg[1]);
				pthread_mutex_unlock(&lock);
			} else{
				printf("Invalid Arguments\n");
			}
		}

		else {  // external command
                  if ((pid = fork()) < 0) {
                        printf("fork error");
                  } else if (pid == 0) {        /* child */
                                        // an array of aguments for execve()
                        char    *execargs[MAXARGS]; 
                        glob_t  paths;
                        int     csource, j;
                        char    **p;

                        if (arg[0][0] != '/' && strncmp(arg[0], "./", 2) != 0 && strncmp(arg[0], "../", 3) != 0) {  // get absoulte path of command
                          pa = get_path();      
                          cmd = which(arg[0], pa);
                          if (cmd) 
                            printf("Executing [%s]\n", cmd);
                          else {              // argument not found
                            printf("%s: Command not found\n", arg[1]);
                            goto nextprompt;
                          }

                          while (pa) {   // free list of path values
                             tmp = pa;
                             pa = pa->next;
                             free(tmp->element);
                             free(tmp);
                          }
                          execargs[0] = malloc(strlen(cmd)+1);
                          strcpy(execargs[0], cmd); // copy absolute path
                          free(cmd);
                        }
                        else {
                          execargs[0] = malloc(strlen(arg[0])+1);
                          strcpy(execargs[0], arg[0]);  // copy command
                        }
			
                        j = 1;
                        for (i = 1; i < arg_no; i++) { // check arguments
                          if (strchr(arg[i], '*') != NULL) { // wildcard!
                            csource = glob(arg[i], 0, NULL, &paths);
                            if (csource == 0) {
                              for (p = paths.gl_pathv; *p != NULL; ++p) {
                                execargs[j] = malloc(strlen(*p)+1);
                                strcpy(execargs[j], *p);
                                j++;
                              }
                              globfree(&paths);
                            }
                            else
                            if (csource == GLOB_NOMATCH) {
                              execargs[j] = malloc(strlen(arg[i])+1);
                              strcpy(execargs[j], arg[i]);
                              j++;
                            }
                          }
                          else {
                            execargs[j] = malloc(strlen(arg[i])+1);
                            strcpy(execargs[j], arg[i]);
                            j++;
			  }
                        }
                        execargs[j] = NULL;

                        if (background) { // get rid of argument "&"
                          j--;
                          free(execargs[j]);
                          execargs[j] = NULL;
                        }			
			

			if (redirection)
                        {
                          int fid;
                          if (!append && rstdout && !rstderr) // >
                          {
                                if(noclobber){
                                        printf("Cannot be overwritten, noclobber is on");
                                } else {
                                         fid = open(execargs[j-1], O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP);
                                         close(1);
                                         dup(fid);
                                         close(fid);
                                }
                          }
                          if (append && rstdout && !rstderr) // >>
                          {
                                 if(noclobber){
                                        printf("Cannot be overwritten, noclobber is on");
                                } else {
                                 fid = open(execargs[j-1], O_WRONLY|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR|S_IRGRP);
                                 close(STDOUT_FILENO);
                                 dup(fid);
                                 close(fid);
                                }
                          }
                          if(!append && rstdout && rstderr){ // >&
                                if(noclobber){
                                        printf("New file cannot be created, noclobber is on");
                                } else {
                                        close(1);
                                        fid = open(execargs[j-1], O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP);
                                        close(2);
                                        dup(1);
                                 }
                         }
                          if(append && rstdout && rstderr){ // >>&
                                if(noclobber){
                                        printf("New file cannot be created, noclobber is on");
                                } else {

                                        close(1);
                                        fid = open(execargs[j-1], O_WRONLY|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR|S_IRGRP);
                                        close(2);
                                        dup(1);
                                }
                          }
                          if(rstdin){ // <
                                if(noclobber){
                                        printf("Cannot be overwritten, noclobber is on");
                                } else {
                                         fid = open(execargs[j-1], O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP);
					 close(0);
                                         dup(fid);
                                         close(fid);
                                }
                          }
                        }
			else if(pipe){
                                if(rstdout && rstderr){ // |
                                        strcpy(*right, arg[2]);
                                        strcpy(*left, arg[0]);

                                        piping(&arg[2], &arg[0] , 0);
                                        goto again;
                                }
                                else if(rstdout){
                                        strcpy(*right, arg[2]);
                                        strcpy(*left, arg[0]);

                                        piping(&arg[2], &arg[0] , 1);
                                        goto again;
                                }
                        }

			// print arguments into execve()
                     
                        //for (i = 0; i < j; i++)
                          //printf("exec arg[%d] = %s\n", i, execargs[i]);
                    

                        execve(execargs[0], execargs, NULL);
                        printf("couldn't execute: %s", buf);
                        exit(127);
                  }

                  /* parent */
                  if (! background) { // foreground process
                    if ((pid = waitpid(pid, &status, 0)) < 0)
                      printf("waitpid error");
                  }
                  else {              // no waitpid if background
                    background = 0;
                  }
                  
                  //if (WIFEXITED(status))  
                    //printf("child terminates with (%d)\n", WEXITSTATUS(status));

                }

           nextprompt:
		while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
		cwd = getcwd(NULL, 0);
  		if(strcmp(prompt, "") != 0)
  		{
    		  printf("%s ", prompt);
  		}
  		printf("[%s]", cwd);
  		free(cwd);

		fprintf(stdout, " >> ");
		fflush(stdout);
	}
	
	if(eofChar == NULL)
	{
	  printf("^D\n");
	  printf("Use \"exit\" to leave tcsh\n");
	  goto again;
	}
	exit(0);
}

void piping(char **right, char **left, char c){
        int pipefd[2];
        pipe(pipefd);
        int pid;
        if((pid = fork()) < 0){
                perror("fork error(1)");
                exit(1);
        }
        else if(pid > 0){
                if(waitpid(pid, NULL, 0) < 0){
                        perror("waitpid error");
                        exit(1);
                }
                if((pid = fork()) < 0){
                        perror("fork error(2)");
                        exit(0);
                }
                else if(pid > 0){
                        if(waitpid(pid, NULL, 0) < 0){
                                perror("waitpid error");
                                exit(1);
                        }
                        return;
                } else if(pid == 0) {
                        close(pipefd[1]);
                        close(0);
                        dup2(pipefd[0], 0);
                        if(execvp(right[0], right) < 0){
                                execv(right[0], right);
                        }
                        exit(0);
                }
        } else if(pid == 0){
                close(1);
                if(c)
                        close(2);
                close(pipefd[0]);
                dup2(pipefd[1], 1);
                if(c){
                        dup2(pipefd[1], 2);
                }
                if(execvp(left[0], left) < 0){
                        execv(left[0], left);
                }
                exit(0);
        }

}
