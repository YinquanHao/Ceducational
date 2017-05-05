#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <time.h>
#define MAXLINE         1024
void *getCommandThread(void *vargp);

typedef struct{
  //userNode_t* head;
  int clientfd;
  char* NAME;
  char* MSG;
}thread_command;
int main(int argc, char ** argv){
    int toClientfd = atoi(argv[1]);
    fprintf(stderr,"the to clientfd %d\n",toClientfd);
    //pthread_t getCommand;
    fd_set ready_set;
    fd_set read_set;
    FD_SET(toClientfd,&read_set);
    FD_SET(STDIN_FILENO,&read_set);
    char readBuff[MAXLINE];
    while(1){
      ready_set = read_set;
      fprintf(stderr,"select block\n");
      if((select(toClientfd+1, &ready_set,NULL,NULL,NULL))== -1){
        perror("select1");
        exit(4);
      }
      for(int i=0; i< toClientfd+1;i++){
        if(FD_ISSET(i,&ready_set)){

          if(i==toClientfd){
            memset(&readBuff,0, MAXLINE);
            int readbyte = 0;
            readbyte = read(toClientfd,&readBuff,MAXLINE);
              if(readbyte !=0){
                fprintf(stdout,"%s\n",readBuff);

              }

          }else if(i==STDIN_FILENO){
            /*

            thread_command *commandArg = (thread_command*) malloc(sizeof *commandArg);
            //commandArg->head = head;
            commandArg->clientfd = toClientfd;
            //commandArg->NAME = NAME;
            //fprintf(stderr,"enter the getCommand\n");
            pthread_create(&getCommand,NULL,getCommandThread,commandArg);
            fprintf(stderr,"out getCommand\n");
            */
            fprintf(stdout,"enter standard in\n");
            memset(&readBuff,0, MAXLINE);
            int readbyte = 0;
            readbyte = read(STDIN_FILENO,&readBuff,MAXLINE);
            fprintf(stdout,"readbytes = %d\n",readbyte);
              if(readbyte !=0){
                fprintf(stdout,"enter readbuyes !=0 \n");
                char* readString =(char*)malloc(sizeof readString);
                strcpy(readString,readBuff);
                int sentBytes = send(toClientfd,readString,strlen(readString),0);
                fprintf(stdout,"send = %d \n",sentBytes);

              }
            }





          }

    }


}

}
void *getCommandThread(void *vargp){
  //thread_command *arg = (thread_command*)vargp;
  //fprintf(stderr,"enter command\n");
  char buf[MAXLINE];
  while (fgets(buf,MAXLINE,stdin)!=NULL){
    char bufCpy[MAXLINE];
    strcpy(bufCpy,buf);
    fprintf(stderr,"%s\nend message",buf);
    //send(clientfd,listuString,strlen(listuString),0);



  }
fprintf(stderr,"out command\n");
return NULL;
}
