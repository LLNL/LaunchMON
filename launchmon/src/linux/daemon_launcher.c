#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>

int main(int argc,char** argv)
{
/*
  char filename[]="/tmp/work/ramya/AAAXXXXXX";
  int fd;
  fd=mkstemp(filename);
  write(fd,"here1",strlen("here1")+1);
  
  printf("i am here\n");

*/
  
  int my_apid=atoi(argv[1]);
  //int no_daemon_opts=argc-10; /* first arg is apid, second is daemon_path followed by daemon opts and pmgr collective arguments */
   
  int no_daemon_opts=argc-5;

  

   
  char* lib_path;
  lib_path = getenv("LD_LIBRARY_PATH");

  char add_lib_path[80];
  sprintf(add_lib_path,":/var/spool/alps/%d/toolhelper%d",my_apid,my_apid);

  strcat(lib_path,(const char*)add_lib_path);
  setenv("LD_LIBRARY_PATH",(const char*)lib_path,1);


  /*dynamic allocation of daemon argument list*/
  char** myargv=(char**) malloc(argc*sizeof(char*));
  myargv[0]=argv[2];/*daemon_path*/


  /* Fill in the daemon opts ..how many ever they may be ***/
  
  int i;
  printf("trying to fill daemon opt\n");
  for(i=1;i<=no_daemon_opts;i++)
  {
     myargv[i]=argv[2+i];
        
  }
  
  /** Fill in 7 pmgr_collective args***/
  int pmgrargstart=i;
  int j;
  for(j=i;j<i+2;j++)
  {
     myargv[j]=argv[argc-2+j-pmgrargstart];
  }
  myargv[j]=NULL; 
    
  //write(fd,"here2",strlen("here2")+1);
  //char execd[100];
  //sprintf(execd, "execd %s\n", myargv[0]);
  //write(fd,execd,strlen(execd) +1);
 /*
  int k=1;
  while(myargv[k]!=NULL)
  { 
    sprintf(execd, "execd %s\n", myargv[k]);
    write(fd,execd,strlen(execd) +1);
    k++;
  }
*/
  
  int ret=execv(myargv[0],myargv);

  if(ret==-1)
  {
   //write(fd,"execv failed", strlen("execv failed")+1);
   printf("error: %d\n",errno);
  }
  

  return 0;
}


