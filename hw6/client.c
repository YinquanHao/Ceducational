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
#include <ctype.h>
#include "sfwrite.h"

#define MAXLINE         1024
extern char** environ;
pthread_mutex_t stdout_mutex;
pthread_mutex_t stderr_mutex;

typedef struct sockaddr SA;

typedef struct userNode {
    char *userName;
    int fd;
    time_t time;
    struct userNode * next;
    struct userNode * prev;
} userNode_t;


typedef struct{
  int maxfd;
  fd_set read_set;
  fd_set ready_set;
  int nready;
  int maxi;
  int clientfd[FD_SETSIZE];
}pool;

typedef struct{
  userNode_t* head;
  int clientfd;
  char* NAME;
  char* MSG;
  pool* pl;
}thread_command;


int open_clientfd(char* hostname, int port);
void help();
void *getCommandThread(void *vargp);
void *OpenChatThread(void *vargp);
int execvpe(const char *file, char *const argv[],char *const envp[]);
char* wolfieRequestFactory(char* input);
char* iamRequestFactory(char* userName);
char* iamNewRequestFactory(char* userName);
char* MSGTokenlize(char* chatCommand, char* fromUser);
char* decodeMSG(char* MSGString);
void init_pool(int listenfd, pool *p);
void add_client(int connfd, pool *p);
void pushToUserList(userNode_t * head, char* userName, int fd);
int checkUserName(userNode_t* head, char* userName);
userNode_t *SearchUserByName(userNode_t * head, char* name);
void printUserList(userNode_t * head);
void  addToHead(userNode_t* head, char* userName, int fd);
userNode_t* openChat(int clientfd,char* NAME,char* readBuff,userNode_t* head,pool pool);
userNode_t *SearchUserByFD(userNode_t * head,int fd);
int checkPassword(char* password);
char* passwordRequestFactory(char* password);
char* oldPasswordRequestFactory(char* password);;


int main(int argc, char ** argv){
  int clientfd, port;
  char *host, buf[MAXLINE];
  char readBuff[MAXLINE];
  char* NAME;
  int opt;
  static pool pool;
  int createNewAccount = 0;

/*this part of code is used for the flag arguments*/
  while((opt = getopt(argc, argv, "hvc")) != -1) {
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

         case 'c':
              fprintf(stderr, "newuser account \n" );
              createNewAccount =1;
              break;
            }
       }
       fprintf(stderr,"(argc - optind) = %d\n",argc - optind);
       fprintf(stderr,"optind = %d\n",optind);
       fprintf(stderr,"argc = %d\n",argc);
       if((optind < argc) && ((argc - optind) == 3)) {
         NAME = argv[optind++];
         fprintf(stderr,"NAME = %s\n",NAME);
         host = argv[optind++];
         fprintf(stderr,"host = %s\n",host);
         port = atoi(argv[optind++]);
         fprintf(stderr,"port = %d\n",port);
       }else{
         fprintf(stderr, "usage: %s <port>\n",argv[0]);
         exit(0);
       }



/*the declaration for thread used to getCommand from stdin*/
  pthread_t getCommand;
/*the declearation for thread to open a chat*/
  pthread_t chatThread;
/*create a head to store the userInformation*/
  userNode_t* head = (userNode_t*)malloc(sizeof head);


/*open a clientfd used to establish a connection with the server*/
  clientfd = open_clientfd(host,port);
  if(clientfd<0){
    ///fprintf(stdout,"Fail to connect to server because of invalid port/IP\n");
    sfwrite(&stdout_mutex,stdout,"Fail to connect to server because of invalid port/IP\n");
    exit(EXIT_SUCCESS);
  }
/*initial the pool*/
  init_pool(clientfd,&pool);

if(createNewAccount!=1){

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

    char authString[MAXLINE] = {};
    strcpy(authString,"AUTH ");
    strcat(authString,NAME);
    strcat(authString," ");
    strcat(authString,"\r\n\r\n");
    if(strcmp(authString,readBuff)==0){
      char passwordField[MAXLINE] = {};

      fprintf(stderr,"ready to get password\n");
      char* password = getpass(passwordField);
      char* generatedPasswordString = oldPasswordRequestFactory(password);
      printf("password entered  : %s\n",password );
      write(clientfd,generatedPasswordString,strlen(generatedPasswordString));

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


    if(strcmp("ERR",readBuff)==0){
      fprintf(stderr,"error PASSWORD user name\n");
      loginSuccessful=1;
      exit(0);
    }

    if(strcmp("SSAP",token)==0){
      loginSuccessful=1;
    //  fprintf(stdout," %s\n",token = strtok(NULL, " "));
      sfwrite(&stdout_mutex,stdout," %s\n",token = strtok(NULL, " "));
    }

  }
}


}else{



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
      char* generatedIAMString = iamNewRequestFactory(NAME);
      fprintf(stderr,"concated string :%s\n", generatedIAMString);
      write(clientfd,generatedIAMString,strlen(generatedIAMString));
    }
    char hinewString[MAXLINE] ;
    strcpy(hinewString,"HINEW ");
    strcat(hinewString,NAME);
    strcat(hinewString," ");
    strcat(hinewString,"\r\n\r\n");
    fprintf(stderr,"hinew string :%s\n", hinewString);
    /*send the newpass request to the server*/
    if(strcmp(readBuff,hinewString)==0){
      fprintf(stderr,"get in\n");
      char passwordField[MAXLINE] = {};

      fprintf(stderr,"ready to get password\n");
      char* password = getpass(passwordField);
      char* generatedPasswordString = passwordRequestFactory(password);
      printf("password entered  : %s\n",password );
      write(clientfd,generatedPasswordString,strlen(generatedPasswordString));
    }

    fprintf(stderr,"readbyte beforeeeeee err : %s\n", readBuff);
    char SAMEUSER[MAXLINE] = "ERR 00 USER NAME TAKEN \r\n\r\n";
    fprintf(stderr,"choose loop %d\n",strcmp(SAMEUSER,readBuff));
    if(strcmp(SAMEUSER,readBuff)==0){
      fprintf(stderr,"error same user name\n");
      loginSuccessful=1;
    }




    char* token = strtok(readBuff," ");
    ///fprintf(stderr,"token0 : %s  /end\n", token);
    //token = strtok(NULL,"\r\n\r\n");
    ///fprintf(stderr,"token : %s  /end\n", token);
    if(strcmp("SSAPWEN",readBuff)==0){
      loginSuccessful=1;
      //fprintf(stdout," %s\n",token = strtok(NULL, " "));
        sfwrite(&stdout_mutex,stdout," %s\n",token = strtok(NULL, " "));
    }



  //  char BADPASS[MAXLINE] = "ERR 02 BAD PASSWORD \r\n\r\n";
    //fprintf(stderr,"choose loop %d\n",strcmp(BADPASS,readBuff));
    if(strcmp("ERR",readBuff)==0){
      fprintf(stderr,"error PASSWORD user name\n");
      loginSuccessful=1;
      exit(0);
    }


  }
}
















}

//not necessary anymore we have the initialpool function
/*
FD_SET(clientfd,&read_set);
FD_SET(STDIN_FILENO,&read_set);
*/
fprintf(stderr,"pool.maxfd %d", pool.maxfd);

/*the main loop for the client program*/
  while(1){
    //fprintf(stderr,"head after add = %s \n",head->userName);
    //fprintf(stderr,"enter whil main\n");
    memset(&readBuff,0, MAXLINE);
    pool.ready_set = pool.read_set;
    if((select(pool.maxfd+1, &pool.ready_set,NULL,NULL,NULL))== -1){
      perror("select1");
      exit(4);
    }
    fprintf(stderr,"after select function\n");
    /*go through the each fd in the file descriptors*/
    fprintf(stderr,"pool.maxfd %d", pool.maxfd);
    for(int i=0; i< pool.maxfd+1;i++){
      fprintf(stderr,"pool.maxfd %d", pool.maxfd);
      if(FD_ISSET(i,&pool.ready_set)){

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
            /*
            if(strcmp("EIFLOW \r\n\r\n",readBuff)==0){
              fprintf(stderr,"enter im verb\n");
              char nameVerb[MAXLINE];
              strcpy(nameVerb,"IAM ");
              strcat(nameVerb,NAME);
              strcat(nameVerb," \r\n\r\n");
              fprintf(stderr,"concated string :%s\n", nameVerb);
              write(clientfd,nameVerb,sizeof(nameVerb));

            }else */
            if(strcmp("BYE \r\n\r\n",readBuff)==0){
              //fprintf(stdout,"BYE\n");
              sfwrite(&stdout_mutex,stdout,"BYE\n");
              char* byeString = "BYE \r\n\r\n";
              send(clientfd,byeString,strlen(byeString),0);

              exit(0);
            }else if(strcmp(token,"EMIT")==0){
              fprintf(stderr,"enrer emit    %s\n",readBuff);
              //char* token = strtok(readBuff," ");
              char *time = strtok(readBuff," ");
              time = strtok(NULL," ");
              //fprintf(stdout,"connected for %s seconds\n",time);
              sfwrite(&stdout_mutex,stdout,"connected for %s seconds\n",time);
            }else if(strcmp(token,"UTSIL")==0){
              //fprintf(stdout,"the user list:\n");
              sfwrite(&stdout_mutex,stdout,"the user list:\n");
              while(token){
                token = strtok(NULL,"\r\n");
                if(token){
                //fprintf(stdout," %s\n",token);
                sfwrite(&stdout_mutex,stdout," %s\n",token);
                }
              }

            }else if(strcmp(token,"MSG")==0){

              thread_command *commandArg = (thread_command*) malloc(sizeof *commandArg);
              commandArg->clientfd = clientfd;
              commandArg->NAME = NAME;
              //commandArg->MSG = readBuff;
              char* readBuffThread = (char*) malloc(MAXLINE);
              strcpy(readBuffThread,readBuff);

              fprintf(stderr,"22readBuff for readBuffThread: %s\n",readBuff);
              commandArg->MSG = readBuffThread;

              commandArg->head = head;
              commandArg->pl = &pool;
              pthread_create(&chatThread,NULL,OpenChatThread,commandArg);
              pthread_join(chatThread,NULL);
              //fprintf(stderr,"head after add = %s \n",head->userName);;
              /*
              userNode_t *resultNode = openChat(clientfd,NAME,readBuff,head, pool);
              fprintf(stderr,"back to earth\n");
              if(resultNode->userName != NULL){
                if(head->userName!=NULL){
                  pushToUserList(head,resultNode->userName,resultNode->fd);
                }else{
                  head = resultNode;
                }
              }
              */
            }else{
              /*undefined message*/
              fprintf(stdout,"%s",buf);
            }

          }

        }else if(i==0){

          thread_command *commandArg = (thread_command*) malloc(sizeof *commandArg);
          commandArg->clientfd = clientfd;
          commandArg->NAME = NAME;
          commandArg->pl = &pool;
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

        }else{

          read(i,&readBuff,MAXLINE);

          fprintf(stderr,"received from chat %s \n",readBuff);
          userNode_t *toUser = SearchUserByFD(head,i);
          fprintf(stderr,"the user %s \n",toUser->userName);
          char* userCommand = (char*)malloc(sizeof userCommand);
          strcpy(userCommand,"MSG ");
          strcat(userCommand,toUser->userName);
          strcat(userCommand," ");
          strcat(userCommand,NAME);
          strcat(userCommand," ");
          strcat(userCommand,readBuff);
          strcat(userCommand," \r\n\r\n");

          send(clientfd,userCommand,strlen(userCommand),0);


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

userNode_t* openChat(int clientfd,char* NAME,char* readBuff,userNode_t* head,pool pool){
  userNode_t *returnNode = (userNode_t*)malloc(sizeof returnNode);
  char *msgCopy = (char*)malloc(sizeof msgCopy);
  strcpy(msgCopy,readBuff);
  char* token = strtok(readBuff," ");
  token = strtok(NULL," ");
  char* user1 = token;
  fprintf(stderr,"user 1: %s\n",user1);
  token = strtok(NULL," ");
  char* user2 = token;
  fprintf(stderr,"user 2: %s\n",user2);
  token = strtok(NULL," ");
  char* message = decodeMSG(msgCopy);
  fprintf(stderr,"user 2: %s\n",message);
  int receive = 0;
  char* title;
  if(strcmp(NAME,user1)==0){
    title = user2;
    receive =1;
  }else{
    title = user1;
    receive = 0;
  }
  fprintf(stderr,"title %s\n",title);
  int exist=1;
  if(head->userName!=NULL){
    fprintf(stderr,"enter check users\n");
    //printUserList(head);
    exist = checkUserName(head,title);
  }
  if(exist == 1){
    if(head->userName!=NULL){
      fprintf(stderr,"enter check users\n");
      //printUserList(head);
      exist = checkUserName(head,title);
    }
    if(exist == 1){
    int sockfd[2];
    int successpair= socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd);
    fprintf(stderr,"successpair %d\n",successpair);
    int fd1 = *sockfd;
    int fd2 = *(sockfd+1);
    add_client(fd1,&pool);
    fprintf(stderr,"sockfd 1  = %d\n",fd1);
    fprintf(stderr,"sockfd 2  = %d\n",fd2);
    char fda[15];
    sprintf(fda, "%d", fd1);
    char fdb[15];
    sprintf(fdb, "%d", fd2);
    char MESSAGE[MAXLINE];
    if(receive==1){
      strcpy(MESSAGE,"> ");
    }else{
      strcpy(MESSAGE,"< ");
    }
    strcat(MESSAGE,message);
    send(fd1,MESSAGE, sizeof(MESSAGE),0);


  returnNode -> userName = title;
  returnNode -> fd =fd1;

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
   strcat(MESSAGE,message);
   send(fd1,MESSAGE, sizeof(MESSAGE),0);
   return NULL;
 }
 //printUserList(head);
 //return 0;
 }
return returnNode;
}

void *OpenChatThread(void *vargp){
  thread_command *arg = (thread_command*)vargp;
  userNode_t *head = arg->head;
  char *msgCopy = (char*)malloc(sizeof msgCopy);
  //msgCopy = &arg->MSG[0];
  strcpy(msgCopy,arg->MSG);
  fprintf(stderr,"message: 123123\n");

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


  if(head->userName!=NULL){
    fprintf(stderr,"enter check users\n");
    //printUserList(head);
    exist = checkUserName(head,title);
  }
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
  strcat(MESSAGE,message);
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
      char readBuff[MAXLINE];
      read(arg->clientfd,&readBuff,MAXLINE);
      if(strcmp(readBuff,"BYE \r\n\r\n")==0){
        exit(0);
      }
      //exit(0);
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

  /*
  fprintf(stdout,"/time                  ask for server for how long it has been connected\n");
  fprintf(stdout,"/help                  list all the commands which the server accepts and what they do\n");
  fprintf(stdout,"/logout                it should disconnect with the server\n");
  fprintf(stdout,"/listu                it should disconnect with the server\n");
*/
  sfwrite(&stdout_mutex,stdout,"/time                  ask for server for how long it has been connected\n");
  sfwrite(&stdout_mutex,stdout,"/help                  list all the commands which the server accepts and what they do\n");
  sfwrite(&stdout_mutex,stdout,"/logout                it should disconnect with the server\n");
  sfwrite(&stdout_mutex,stdout,"/listu                 it should disconnect with the server\n");

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


void addToHead(userNode_t* head, char* userName, int fd){
  head->userName  = userName;
  head->fd = fd;
  head-> next = NULL;
  head-> prev = NULL;
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

char* iamNewRequestFactory(char* userName){
  char *buffer = (char*)malloc(sizeof buffer);
  strcpy(buffer,"IAMNEW ");
  strcat(buffer,userName);
  strcat(buffer," \r\n\r\n");
  return buffer;
}


char* passwordRequestFactory(char* password){
  char *buffer = (char*)malloc(sizeof buffer);
  strcpy(buffer,"NEWPASS ");
  strcat(buffer,password);
  strcat(buffer," \r\n\r\n");
  return buffer;
}



char* oldPasswordRequestFactory(char* password){
  char *buffer = (char*)malloc(sizeof buffer);
  strcpy(buffer,"PASS ");
  strcat(buffer,password);
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



void init_pool(int listenfd, pool *p){
  int i;
  p->maxi = -1;
  for(i=0; i<FD_SETSIZE;i++){
    p->clientfd[i]=-1;
  }
  p->maxfd = listenfd;
  FD_ZERO(&p->read_set);
  FD_SET(listenfd,&p->read_set);
  FD_SET(STDIN_FILENO,&p->read_set);
}

void add_client(int connfd, pool *p){
  fprintf(stderr,"enter add clients\n");
  int i;
  p->nready--;
  for (i=0;i<FD_SETSIZE;i++){
      fprintf(stderr,"the position loop = %d\n",i);
      if(p->clientfd[i]<0){
        p->clientfd[i] = connfd;
        FD_SET(connfd,&p->read_set);
        if(connfd > p->maxfd){
          p->maxfd = connfd;
        }
        if(i>p->maxi){
          p->maxi = i;
        }
        break;
      }
    }
  if(i==FD_SETSIZE){
    fprintf(stderr,"add_client error: too many clients");
  }
}

void printUserList(userNode_t * head) {
    userNode_t * current = (userNode_t*)malloc(sizeof current);
    current  = head;
    while (current != NULL) {


        fprintf(stdout,"%s\n", current->userName);
        fprintf(stdout,"%d\n", current->fd);
        current = current->next;
    }
}

userNode_t *SearchUserByFD(userNode_t * head,int fd){
  userNode_t* current = head;
  while(current !=NULL){
    if((current->fd) == fd){
      return current;
    }
    current = current->next;
  }
  return NULL;
}

int checkPassword(char* password){
  int stringLength = strlen(password);
  int containUpper = -1;
  int containNumber = -1;
  int containSymbol = -1;
  int i=0;
  if(stringLength<5){
    return -1;
  }
  for(i=0;i<stringLength;i++){
    char currentChar = password[i];
    if(isalpha(currentChar)){
      if(isupper(currentChar)){
        containUpper = 1;
      }
    }else if(isdigit(currentChar)){
      containNumber = 1;

    }else{
      containSymbol = 1;
    }

  }
  if(containSymbol==1 && containUpper==1 && containNumber==1){
    return 1;
  }else{
    return 0;
  }
}
