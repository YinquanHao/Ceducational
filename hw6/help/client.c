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
#include "sfwrite.h"

#define MAXLINE         1024
extern char** environ;
typedef struct sockaddr SA;
int open_clientfd(char* hostname, int port);
void help();
void *getCommandThread(void *vargp);
void *OpenChatThread(void *vargp);
int execvpe(const char *file, char *const argv[],char *const envp[]);
char* wolfieRequestFactory(char* input);
char* iamRequestFactory(char* userName);
char* MSGTokenlize(char* chatCommand, char* fromUser);
char* decodeMSG(char* MSGString);

pthread_mutex_t stdout_mutex;

typedef struct{
  int maxfd;
  fd_set read_set;
  fd_set ready_set;
  int nready;
  int maxi;
  int clientfd[FD_SETSIZE];
}pool;

typedef struct userNode {
    char *userName;
    int fd;
    time_t time;
    struct userNode * next;
    struct userNode * prev;
} userNode_t;

typedef struct{
  userNode_t* head;
  int clientfd;
  char* NAME;
  char* MSG;
  pool* pl;
}thread_command;


void pushToUserList(userNode_t * head, char* userName, int fd);
int checkUserName(userNode_t* head, char* userName);
userNode_t *SearchUserByName(userNode_t * head, char* name);
int main(int argc, char ** argv){
  int clientfd, port;
  char *host, buf[MAXLINE];
  char readBuff[MAXLINE];
  char* NAME;
  int opt;

  while((opt = getopt(argc, argv, "hv")) != -1) {
       switch(opt) {
           case 'h':
               /* The help menu was selected */
               fprintf(stderr, "-h                  Display help menu & return EXIT_SUCCESS.\n");
               fprintf(stderr, "-c                  Requests to server to create a new user\n");
               fprintf(stderr, "-v                  Verbose print all incoming and outgoing proticol verbs & content.\n");
               fprintf(stderr, "NAME                This is the username to display when chatting.\n");
               fprintf(stderr, "SERVER_IP           The ipaddress of the server to connect to.\n");
               fprintf(stderr, "SERVER_PORT         The port to connect to\n");
               exit(EXIT_SUCCESS);
               break;
           case 'v':
              fprintf(stderr, "/time                Display help menu & return EXIT_SUCCESS.\n");
              fprintf(stderr, "/help                Verbose print all incoming and outgoing proticol verbs & content.\n");
              fprintf(stderr, "/logout              Port number to listen on.\n");
              fprintf(stderr, "/listu               Message to display to the client when they connect.\n");

              break;
         }
       }

       if(optind < argc && (argc - optind) == 3) {
         NAME = argv[1];
         host = argv[2];
         port = atoi(argv[3]);
    }else{
      fprintf(stderr, "usage: %s <port>\n",argv[0]);
      exit(0);
    }

pthread_t getCommand;
  pthread_t chatThread;
  userNode_t* head = (userNode_t*)malloc(sizeof head);

  fd_set read_set;
  fd_set ready_set;
  if(argc!=4){
    fprintf(stderr, "usage: %s <port>\n",argv[0]);
    exit(0);
  }

  clientfd = open_clientfd(host,port);
  fprintf(stderr,"clientfd  = %d \n",clientfd);


  /*send WOLFIE \r\n\r\n to server*/
  char *wolfieString = "WOLFIE";
  char *generatedWolfieString  = wolfieRequestFactory(wolfieString);
  int bytesSent = write(clientfd,generatedWolfieString,strlen(generatedWolfieString));
  fprintf(stderr,"bytesSent = %d",bytesSent);


  int readbyte = 0;
  int loginSuccessful = 0;

  while(!loginSuccessful){

  fprintf(stderr,"%d rnter rhis\n",readbyte);
  memset(&readBuff,0, MAXLINE);
  readbyte = read(clientfd,&readBuff,MAXLINE);

  if(readbyte !=0){
    fprintf(stderr,"readbyte : %d\n", readbyte);
    fprintf(stderr,"readbyte : %s\n", readBuff);


    fprintf(stderr,"newwereadbyte : %s\n", readBuff);
    if(strcmp("EIFLOW \r\n\r\n",readBuff)==0){
      fprintf(stderr,"enter im verb\n");
      char* generatedIAMString = iamRequestFactory(NAME);
      fprintf(stderr,"concated string :%s\n", generatedIAMString);
      write(clientfd,generatedIAMString,strlen(generatedIAMString));
    }

    fprintf(stderr,"readbyte beforeeeeee err : %s\n", readBuff);
    char SAMEUSER[MAXLINE] = "ERR 00 USER NAME TAKEN \r\n\r\n";
    fprintf(stderr,"choose loop %d\n",strcmp(SAMEUSER,readBuff));
    if(strcmp(SAMEUSER,readBuff)==0){
      fprintf(stderr,"error same user name\n");
      loginSuccessful=1;
    }
    char* token = strtok(readBuff," ");
    fprintf(stderr,"token : %s  /end\n", token);
    if(strcmp("MOTD",token)==0){
      loginSuccessful=1;
      //fprintf(stdout," %s\n",token = strtok(NULL, " "));
      sfwrite(&stdout_mutex,stdout," %s\n",token = strtok(NULL, " "));
    }



  }

}

FD_SET(clientfd,&read_set);
FD_SET(STDIN_FILENO,&read_set);



  while(1){
    //fprintf(stderr,"enter while loop\n");
    memset(&readBuff,0, MAXLINE);
    ready_set = read_set;
    if((select(clientfd+1, &ready_set,NULL,NULL,NULL))== -1){
      perror("select1");
      exit(4);
    }
    //fprintf(stderr,"after select function\n");

    for(int i=0; i< clientfd+1;i++){
      if(FD_ISSET(i,&ready_set)){

        if(i==clientfd){

          int readbyte = 0;
          readbyte = read(clientfd,&readBuff,MAXLINE);
          char temp[MAXLINE];
          strcpy(temp,readBuff);
          char* token;
          token = strtok(temp," ");
          if(readbyte !=0){
            //fprintf(stderr,"readbyte : %d\n", readbyte);
            fprintf(stderr,"readbytein wgile liip : %s\n", readBuff);
            if(strcmp("EIFLOW \r\n\r\n",readBuff)==0){
              fprintf(stderr,"enter im verb\n");
              char nameVerb[MAXLINE];
              strcpy(nameVerb,"IAM ");
              strcat(nameVerb,NAME);
              strcat(nameVerb," \r\n\r\n");
              fprintf(stderr,"concated string :%s\n", nameVerb);
              write(clientfd,nameVerb,sizeof(nameVerb));
            }else if(strcmp("BYE \r\n\r\n",readBuff)==0){
              fprintf(stdout,"BYE\n");
              exit(0);
            }else if(strcmp(token,"EMIT")==0){
              fprintf(stderr,"enrer emit    %s\n",readBuff);
              //char* token = strtok(readBuff," ");
              char *time = strtok(readBuff," ");
              time = strtok(NULL," ");
              fprintf(stdout,"connected for %s seconds\n",time);
            }else if(strcmp(token,"UTSIL")==0){
              fprintf(stdout,"the user list %s\n",readBuff);

            }else if(strcmp(token,"MSG")==0){
              thread_command *commandArg = (thread_command*) malloc(sizeof *commandArg);
              commandArg->clientfd = clientfd;
              commandArg->NAME = NAME;

              char* readBuffThread = (char*) malloc(MAXLINE);
              strcpy(readBuffThread,readBuff);
              //commandArg->MSG = readBuff;
              commandArg->MSG = readBuffThread;
              commandArg->pl = &pool;


              commandArg->head = head;
              pthread_create(&chatThread,NULL,OpenChatThread,commandArg);
              pthread_join(chatThread,NULL);



            }else{
              fprintf(stdout,"%s",buf);
            }

          }

        }else if(i==0){

          thread_command *commandArg = (thread_command*) malloc(sizeof *commandArg);
          commandArg->clientfd = clientfd;
          commandArg->NAME = NAME;
          pthread_create(&getCommand,NULL,getCommandThread,commandArg);
          fprintf(stderr,"out getCommand\n");

          /*
          fprintf(stderr,"out getCommand 13123\n");
          readbyte = read(STDIN_FILENO,&readBuff,MAXLINE);
          if(strcmp(readBuff,"/time\n")==0){
            fprintf(stderr,"out getCommand\n");
            char* timeString = "TIME \r\n\r\n";
            int bytesent = send(clientfd,timeString,strlen(timeString),0);
            fprintf(stderr,"sent the time request to server, %d\n",bytesent);

          }
          if(strcmp(readBuff,"/logout\n")==0){
            int bytesent = send(clientfd,"BYE \r\n\r\n",8,0);
            fprintf(stderr,"logoru, %d\n",bytesent);
            exit(0);
          }
          if(strcmp(readBuff,"/listu\n")==0){
            char* listuString = "LISTU \r\n\r\n";
            int bytesent = send(clientfd,listuString,strlen(listuString),0);
            fprintf(stderr,"sent the list user request to server, %d\n",bytesent);
          }if(strcmp(readBuff,"/help\n")==0){
            help();
          }
          else{
            printf("%s",readBuff);
          }


          */

        }





      }

    }
  }



    //int readbyte = 0;
    /*the wofile verb has been sent but there is no repley*/
    /*
    fprintf(stderr,"%d rnter rhis\n",readbyte);
    readbyte = read(clientfd,&readBuff,MAXLINE);

    if(readbyte !=0){
      fprintf(stderr,"readbyte : %d\n", readbyte);
      fprintf(stderr,"readbyte : %s\n", readBuff);
      if(strcmp("EIFLOW/r/n/r/n",readBuff)==0){
        fprintf(stderr,"enter im verb\n");
        char nameVerb[MAXLINE];
        strcpy(nameVerb,"IAM ");
        strcat(nameVerb,NAME);
        strcat(nameVerb," /r/n/r/n");
        fprintf(stderr,"concated string :%s\n", nameVerb);


        //write(client,"",MAXLINE);
        write(clientfd,nameVerb,sizeof(nameVerb));
      }
    }


  while (fgets(buf,MAXLINE,stdin)!=NULL){

  }

}
*/

  exit(0);
}

int open_clientfd(char* hostname, int port){
  int clientfd;
  struct hostent *hp;
  struct sockaddr_in serveraddr;
  if((clientfd=socket(AF_INET,SOCK_STREAM,0))<0){
    return -1;
  }
  if((hp = gethostbyname(hostname))==NULL){
    return -2;
  }
  bzero((char*)&serveraddr,sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  bcopy((char*)hp->h_addr_list[0],(char*)&serveraddr.sin_addr.s_addr, hp->h_length);
  serveraddr.sin_port= htons(port);

  if(connect(clientfd,(SA*)&serveraddr,sizeof(serveraddr))<0){
    return -3;
  }

  return clientfd;

}

void *OpenChatThread(void *vargp){
  thread_command *arg = (thread_command*)vargp;
  userNode_t *head = arg->head;
  char *msgCopy = (char*)malloc(sizeof msgCopy);
  //msgCopy = &arg->MSG[0];
  strcpy(msgCopy,arg->MSG);
  fprintf(stderr,"message: %s\n",arg->MSG);
  fprintf(stderr,"message: %s\n",msgCopy);

  char* token = strtok((arg->MSG)," ");
  token = strtok(NULL," ");
  char* user1 = token;
  fprintf(stderr,"user 1: %s\n",user1);
  token = strtok(NULL," ");
  char* user2 = token;
  fprintf(stderr,"user 2: %s\n",user2);
  token = strtok(NULL," ");
  //char* message = "hello";
  char* message =  decodeMSG(msgCopy);

  //fprintf(stderr,"message: %s\n",message);


  fprintf(stderr,"user 2: %s\n",message);
  int receive = 0;
  char*  title ;
  if(strcmp(arg->NAME,user1)==0){
    title = user2;
    receive =1;
  }else{
    title = user1;
    receive = 0;
  }
  fprintf(stderr,"title %s\n",title);

  int exist=1;
  fprintf(stderr,"checkAgain1: %s\n",message);

  if(head->userName!=NULL){
    fprintf(stderr,"enter check users\n");
    //printUserList(head);
    fprintf(stderr,"checkAgain2: %s\n",message);
    exist = checkUserName(head,title);
    fprintf(stderr,"checkAgain3: %s\n",message);
  }

  fprintf(stderr,"checkAgain: %s\n",message);
  if(exist == 1){



  int sockfd[2];
  int successpair= socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd);
  fprintf(stderr,"successpair %d\n",successpair);
  int fd1 = *sockfd;
  int fd2 = *(sockfd+1);

  add_client(fd1,arg->pl);


  fprintf(stderr,"sockfd 1  = %d\n",fd1);
  fprintf(stderr,"sockfd 2  = %d\n",fd2);
  char fda[15];
  sprintf(fda, "%d", fd1);
  char fdb[15];
  sprintf(fdb, "%d", fd2);
  char MESSAGE[MAXLINE];
  //char* MESSAGE = (char*)malloc(MAXLINE);
  if(receive==1){
    strcpy(MESSAGE,"> ");
  }else{
    strcpy(MESSAGE,"< ");
  }
  strcat(MESSAGE,message);
  send(fd1,MESSAGE, sizeof(MESSAGE),0);
  if(head->userName!=NULL){
  pushToUserList(head,title,fd1);
}else{
  fprintf(stderr,"enter push to head, push %s to head \n",title);
  char* newUser = (char* )malloc(strlen(title));
  strcpy(newUser,title);
  head->userName = newUser;
  head->fd = fd1;
  head->next = NULL;
  head->prev = NULL;
}

 int pid;
 int a;
  if((pid = fork())==0){

    char *args[] = {"xterm", "-geometry", "45x35+10", "-T", title, "-e", "./chat", fdb , NULL};
    if((a=execvpe(args[0],args,environ))<0){
      fprintf(stderr, "Command not found. \n");
      char one[] = "1";
      setenv("?",one,1);
    }



  }

}else{
  fprintf(stderr,"not openong a new window");
  userNode_t* node = SearchUserByName(head,title);
  int fd1 = node->fd;
  char MESSAGE[MAXLINE];


  if(receive==1){
    strcpy(MESSAGE,"> ");
  }else{
    strcpy(MESSAGE,"< ");
  }

  fprintf(stderr,"message org: %s\n",message);
  strcat(MESSAGE,message);
  fprintf(stderr,"message: %s\n",MESSAGE);
  send(fd1,MESSAGE, sizeof(MESSAGE),0);


}


//printUserList(head);

return NULL;
}




void *getCommandThread(void *vargp){
  thread_command *arg = (thread_command*)vargp;
  //fprintf(stderr,"enter command\n");
  char buf[MAXLINE];
  while (fgets(buf,MAXLINE,stdin)!=NULL){
    char bufCpy[MAXLINE];
    strcpy(bufCpy,buf);
      fprintf(stderr,"%s\n",buf);
    if(strcmp(buf,"/help\n")==0){
      help();
    }
    if(strcmp(buf,"/time\n")==0){
      char* timeString = "TIME \r\n\r\n";
      int bytesent = send(arg->clientfd,timeString,strlen(timeString),0);
      fprintf(stderr,"sent the time request to server, %d\n",bytesent);

    }
    if(strcmp(buf,"/logout\n")==0){
      char* byeString = "BYE \r\n\r\n";
      int bytesent = send(arg->clientfd,byeString,strlen(byeString),0);
      fprintf(stderr,"logoru, %d\n",bytesent);
      fprintf(stderr,"bytes sent logout =  %d\n",bytesent);
    }
    if(strcmp(buf,"/listu\n")==0){
      char* listuString = "LISTU \r\n\r\n";
      int bytesent = send(arg->clientfd,listuString,strlen(listuString),0);
      fprintf(stderr,"sent the list user request to server, %d\n",bytesent);
    }
    char* token = strtok(bufCpy," ");
    fprintf(stderr,"the token bufCpy %s123123123\n",token);
    if(strcmp(token,"/chat")==0){

      char* generatedMSGString =  MSGTokenlize(buf, arg->NAME);
      send(arg->clientfd,generatedMSGString,strlen(generatedMSGString),0);

    }

  }
fprintf(stderr,"out command\n");
return NULL;
}



void help(){
  fprintf(stdout,"/time                  ask for server for how long it has been connected\n");
  fprintf(stdout,"/help                  list all the commands which the server accepts and what they do\n");
  fprintf(stdout,"/logout                it should disconnect with the server\n");
  fprintf(stdout,"/listu                it should disconnect with the server\n");

}


char* outgoingMesFac(char* name, char* message){
  char* result=NULL;
  strcpy(result,name);
  strcat(result," < ");
  strcat(result, message);
  return result;
}

void pushToUserList(userNode_t * head, char* userName, int fd) {
    userNode_t * current = head;
    while (current->next != NULL) {
        current = current->next;
    }
    /* now we can add a new variable */
    current->next = (userNode_t*)malloc(sizeof(userNode_t));
    current->next->userName = userName;
    current->next->fd = fd;
    current->next->time = time(NULL);
    current->next->next = NULL;
    current->next->prev = current;
}

int checkUserName(userNode_t* head, char* userName){
  fprintf(stderr, "entererererrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr\n");
  userNode_t * current = head;
  if(strcmp(current->userName,userName)==0){
    return 0;
  }
  while (current->next != NULL) {
    fprintf(stderr, "looooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooop\n");
      current = current->next;
    fprintf(stderr, "l1111111111111111111111111111111111111111111111111111111111111111111111111p\n");

      if(strcmp(current->userName,userName)==0){
        return 0;
      }
  }
  return 1;
}

userNode_t *SearchUserByName(userNode_t * head, char* name){
  userNode_t* current = head;
  while(current !=NULL){
    if(strcmp(current->userName,name)==0){
      return current;
    }
    current = current->next;
  }
  return NULL;
}


char* wolfieRequestFactory(char* input){
  char *buffer = (char*)malloc(sizeof buffer);
  strcpy(buffer,input);
  strcat(buffer," \r\n\r\n");
  return buffer;
}

char* iamRequestFactory(char* userName){
  char *buffer = (char*)malloc(sizeof buffer);
  strcpy(buffer,"IAM ");
  strcat(buffer,userName);
  strcat(buffer," \r\n\r\n");
  return buffer;
}


char* MSGTokenlize(char* chatCommand, char* fromUser){
  strtok(chatCommand," ");
  char* Messagetoken;
  char* userToken;
  userToken = strtok(NULL," ");
  Messagetoken = strtok(NULL,"\r\n\r\n");
  char *buffer = (char*)malloc(sizeof buffer);
  strcpy(buffer,"MSG ");
  strcat(buffer,userToken);
  strcat(buffer," ");
  strcat(buffer,fromUser);
  strcat(buffer," ");
  strcat(buffer,Messagetoken);
  strcat(buffer," \r\n\r\n");
  return buffer;
}

char* decodeMSG(char* MSGString){
  strtok(MSGString," ");
  strtok(NULL," ");
  strtok(NULL," ");
  return strtok(NULL,"\r\n\r\n");
}
