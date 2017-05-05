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
#include <signal.h>
#include <sys/eventfd.h>
#include <ctype.h>
#include "sfwrite.h"


#define LISTENQ         1024
#define MAXLINE         1024
pthread_mutex_t count_mutex;
pthread_mutex_t stderr_mutex;


typedef struct sockaddr SA;
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

typedef struct accNode {
    char *userName;
    char *password;
    struct accNode * next;
    struct accNode * prev;
} accNode_t;

typedef struct {
    int fd;
    userNode_t* head;
    pool* pl;
} thread_data;


typedef struct{
  int fd;
  userNode_t* head;
  pool* pl;
  char* message_input;
  accNode_t* accHead;
}thread_fd;

typedef struct{
  userNode_t* head;
  int listenfd;
  accNode_t* accHead;
}thread_command;

int open_listenfd(int port);
void check_clients(pool *p);
void init_pool(int listenfd, pool *p);
void add_client(int connfd, pool *p);
void *getCommandThread(void *vargp);
void *loginThread(void *vargp);
void pushToUserList(userNode_t * head, char* userName, int fd);
void printUserList(userNode_t * head);
int checkUserName(userNode_t* head, char* userName);
int checkAccUserName(accNode_t* accHead, char* userName);
void help();
void closeAllSocket(userNode_t* head,int listenfd);
void *getDataThread(void *vargp);
userNode_t *SearchUserByFD(userNode_t * head,int fd);
userNode_t *SearchUserByName(userNode_t * head, char* name);
void signalhandler(int signo);
char* wolfieRequestFactory(char* input);
char* iamRequestFactory(char* userName);
char* HImessageFactory(char* userName);
char* MOTDmessageFactory(char* motdMessage);
char* EMITFactory(char* time);
userNode_t* removeFromUserList(userNode_t*,userNode_t* );
char* HINEWmessageFactory(char* userName);
int checkPassword(char* );
void pushAccountList(accNode_t * head, char* userName, char* password);
void printAccount(accNode_t * head);
int matchAccUserPassword(accNode_t* accHead, char* userName, char* password);
userNode_t* head = NULL;

int main(int argc, char **argv){
  signal(SIGINT,signalhandler);
  int opt;
  int listenfd,connfd;
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  static pool pool;
  char* port_input;
  char* message_input;
  //userNode_t* head = NULL;
  head = (userNode_t*)malloc(sizeof(userNode_t));
  head->fd = -1;
  accNode_t* accHead = (accNode_t*)malloc(sizeof(accNode_t));
  accHead->userName = NULL;
  pthread_t login;
  pthread_t getCommand;
  pthread_t getData;


  while((opt = getopt(argc, argv, "hv")) != -1) {
       switch(opt) {
           case 'h':
               /* The help menu was selected */
               sfwrite(&count_mutex,stdout,"-h                  Display help menu & return EXIT_SUCCESS.\n");
               sfwrite(&count_mutex,stdout,"-v                  Verbose print all incoming and outgoing proticol verbs & content.\n");
               sfwrite(&count_mutex,stdout,"PORT_NUMBER         Port number to listen on.\n");
               sfwrite(&count_mutex,stdout,"MOTD                Message to display to the client when they connect.\n");
               /*
               fprintf(stderr, "-h                  Display help menu & return EXIT_SUCCESS.\n");
               fprintf(stderr, "-v                  Verbose print all incoming and outgoing proticol verbs & content.\n");
               fprintf(stderr, "PORT_NUMBER         Port number to listen on.\n");
               fprintf(stderr, "MOTD                Message to display to the client when they connect.\n");
               */
               exit(EXIT_SUCCESS);
               break;
           case 'v':
              help();
              break;
            }
       }
  if(optind < argc && (argc - optind) == 2) {
    port_input = argv[optind++];
    message_input = argv[optind++];
  }else{
    //fprintf(stderr, "usage: %s <port>\n",argv[0]);
    sfwrite(&stderr_mutex,stderr,"usage: %s <port>\n",argv[0]);
    exit(0);
  }
  int port = atoi(port_input);
  /*open a listening fd and bind it */
  listenfd = open_listenfd(port);
  /* initial the pool structure*/
  init_pool(listenfd,&pool);
  sfwrite(&count_mutex,stdout," currently listening on port %d\n",port);
  /*add stdin in to the read_set,
  Now inthr read_set we have two file descriptor one is litening fd used to accept new connection request the other is stdin*/
  //FD_SET(STDIN_FILENO,&pool.read_set);

  //fprintf(stderr,"listenfd prepared\n");
  while(1){
  //fprintf(stderr,"enter the while loop\n");

      pool.ready_set = pool.read_set;
      //fprintf(stderr,"select function below\n");
      //fprintf(stderr,"print maxfd = %d\n",pool.maxfd);
    if((pool.nready = select(pool.maxfd+1, &pool.ready_set,NULL,NULL,NULL))== -1){
      perror("select1");
      exit(4);
    }



    //fprintf(stderr,"select function prepared %d\n",pool.nready);
    /*go through all the file descriptors in the read_set*/
    for(int i=0; i< pool.maxfd+1;i++){
      //fprintf(stderr,"start working through the fds i = %d\n",i);
      /*enter the if loop when we got a file descriptor has data ready*/
      if(FD_ISSET(i,&pool.ready_set)){
        //fprintf(stderr,"found ready i=%d\n",i);
        /*in this case we got the listenfd has something ready to read, which means there is a new connection need to handle*/
        if(i==listenfd){

          /*in this portion we handled the connect request and connect  them*/
          //fprintf(stderr,"get in listenfd\n");
          clientlen = sizeof(struct sockaddr_storage);
          connfd = accept(listenfd,(SA*)&clientaddr,&clientlen);
          //fprintf(stderr,"print the connfd %d\n", connfd);
          struct sockaddr_in *sin = (struct sockaddr_in *)&clientaddr;
          unsigned char *ip = (unsigned char* )&sin->sin_addr.s_addr;
          sfwrite(&stderr_mutex,stderr,"server connect to %d \n", ip[0]);
          //fprintf(stderr,"server connect to %d \n", ip[0]);

          /*we create a new thread to handle the login process*/
          thread_fd *loginarg =(thread_fd*) malloc(sizeof *loginarg);
          /*we pass the arguments to the thread we make a structure which contains the connfd and the head of the userList*/
          //printUserList(head);
          loginarg->fd = connfd;
          loginarg->head = head;
          loginarg->accHead = accHead;
          loginarg->pl = &pool;
          loginarg->message_input=message_input;

          pthread_create(&login,NULL,loginThread ,loginarg);
          //fprintf(stderr,"rerwerwerwerwerwev2vqe4bv\n");


        }
        else if(i==0){
          thread_command *commandArg = (thread_command*) malloc(sizeof *commandArg);
          commandArg->head = head;
          commandArg->listenfd = listenfd;
          commandArg->accHead = accHead;
          //fprintf(stderr,"enter the getCommand\n");
          pthread_create(&getCommand,NULL,getCommandThread,commandArg);
            //fprintf(stderr,"out getCommand\n");
            /*handle data from a client*/
        }else{
          //fprintf(stderr,"enter the read file reques\n");
          thread_data *dataArg = (thread_data*) malloc(sizeof *dataArg);
          dataArg->fd = i;
          dataArg->head = head;
          dataArg->pl = &pool;
          pthread_create(&getData,NULL,getDataThread,dataArg);
          //pthread_join(getData,NULL);
          //fprintf(stderr,"meai lood head%s",head->userName);

/*

          fprintf(stderr,"enter the else lopp??????\n");
          int nbytes;
          char readBuffer[MAXLINE];
          if((nbytes = recv(i,readBuffer,sizeof readBuffer,0))<=0){
            if(nbytes ==0){
              fprintf(stderr,"%d 's connection closed\n",i);
            }else{
              perror("recv");
            }
            close(i);
            ///remember to remove it


          }else{
            fprintf(stderr,"%d says %s",i,readBuffer);
          }

*/



        }







      }
    }

}
    /*
    pool.ready_set = pool.read_set;
    //fprintf(stderr,"maxfd %d\n", pool.maxfd);
    pool.nready = select(pool.maxfd+1, &pool.read_set,NULL,NULL,NULL);

    if(FD_ISSET(STDIN_FILENO, &pool.read_set)){
      command();
    }

    if(FD_ISSET(listenfd,&pool.ready_set)){
      clientlen = sizeof(struct sockaddr_storage);
      connfd = accept(listenfd,(SA*)&clientaddr,&clientlen);
      struct sockaddr_in *sin = (struct sockaddr_in *)&clientaddr;
      unsigned char *ip = (unsigned char *)&sin->sin_addr.s_addr;

      fprintf(stderr,"server connect to %d \n", ip[0]);


      thread_fd *loginarg =(thread_fd*) malloc(sizeof *loginarg);
      loginarg->fd = connfd;
      loginarg->head = head;
      pthread_create(&login,NULL,loginThread ,loginarg);
      add_client(connfd, &pool);
    }
    check_clients(&pool);
  }

*/
}


int open_listenfd(int port)
{
  int listenfd, optval=1;
  struct sockaddr_in serveraddr;
  if((listenfd = socket(AF_INET, SOCK_STREAM, 0))<0){
    //fprintf(stderr,"enterthis -1");
    return -1;
  }
  if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,
                (const void *)&optval , sizeof(int))<0){
                  //fprintf(stderr,"enterthis -1");
                  return -1;
                }

  bzero((char *)&serveraddr ,sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)port);

  //fprintf(stderr, "%d",INADDR_ANY);

  if(bind(listenfd,(SA*)&serveraddr,sizeof(serveraddr))<0){
    //fprintf(stderr,"enterthis -1");
    return -1;
  }
  if(listen(listenfd, LISTENQ)<0){
    //fprintf(stderr,"enterthis -1");
    return -1;
  }
  return listenfd;
}

void init_pool(int listenfd, pool *p){
  int i;
  p->maxi = -1;
  //fprintf(stderr,"fdsize %d\n",FD_SETSIZE);
  for(i=0; i<FD_SETSIZE;i++){
    p->clientfd[i]=-1;
  }

  p->maxfd = listenfd;
  FD_ZERO(&p->read_set);
  FD_SET(listenfd,&p->read_set);
  FD_SET(STDIN_FILENO,&p->read_set);
}

void add_client(int connfd, pool *p){
  //fprintf(stderr,"enter add clients\n");
  int i;
  p->nready--;
  for (i=0;i<FD_SETSIZE;i++){
      //fprintf(stderr,"the position loop = %d\n",i);
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
    sfwrite(&stderr_mutex,stderr,"add_client error: too many clients");
    //fprintf(stderr,"add_client error: too many clients");
  }
}


void check_clients(pool *p)
{
  //  printf("Server received");
    int i, connfd;
    ///char buf[MAXLINE];
    //rio_t rio;

    for (i = 0; (i <= p->maxi) && (p->nready > 0); i++) {
	connfd = p->clientfd[i];
	//rio = p->clientrio[i];

	/* If the descriptor is ready, echo a text line from it */
	if ((connfd > 0) && (FD_ISSET(connfd, &p->ready_set))) {

	    p->nready--;
      printf("Server received");
      close(connfd);

}else{
  close(connfd);
}
    }
}


void *getDataThread(void *vargp){

  //fprintf(stderr,"enter getdatathread\n");
  thread_data *arg = (thread_data*)vargp;
  int fd = arg->fd;
  int nbytes;
  char readBuffer[MAXLINE];
  memset(readBuffer,0,sizeof(readBuffer));
  //fprintf(stderr,"reachtgis point?\n");

  if((nbytes = recv(fd,readBuffer,sizeof readBuffer,0))<=0){
    if(nbytes ==0){
    //  fprintf(stderr,"%d 's connection closed\n",fd);
    }else{
      perror("recv");
    }
    close(fd);
    ///remember to remove it


  }else{
    //copy the readBuffer
    char readBufferCpy[MAXLINE];
    strcpy(readBufferCpy,readBuffer);
    char* token = strtok(readBufferCpy," ");

    if(strcmp("TIME \r\n\r\n",readBuffer)==0){
      userNode_t *currentUser = SearchUserByFD(arg->head,fd);
      time_t loginTime = currentUser->time;
      time_t currentTime = time(NULL);
      //fprintf(stderr,"total login time %li",currentTime-loginTime);
      char sendBuffer[MAXLINE];
      sprintf(sendBuffer,"%li",currentTime-loginTime);
      char* time = &sendBuffer[0];

      char* generatedTimeString   =  EMITFactory(time);
      send(fd,generatedTimeString,strlen(generatedTimeString),0);
    }else if(strcmp("LISTU \r\n\r\n",readBuffer)==0){

      char userList[MAXLINE];
      userNode_t * current = (arg->head);
      strcpy(userList,"UTSIL ");
      while(current->prev!=NULL){
        current = current->prev;
      }
      while(current != NULL){
        strcat(userList,current->userName);
        strcat(userList," \r\n");
        current = current->next;
        //fprintf(stderr,"Looooop\n");
      }
        strcat(userList," \r\n\r\n");
      char *generatedUTSILString  = &userList[0];
      send(fd,generatedUTSILString,strlen(generatedUTSILString),0);

      //fprintf(stderr, " the ois the userlist %s\n",userList);
    }else if(strcmp("BYE \r\n\r\n",readBuffer)==0){

      send(arg->fd,"BYE \r\n\r\n",8,0);
      close(arg->fd);

      FD_CLR(arg->fd,&(arg->pl->read_set));

      arg->pl->clientfd[arg->fd] = -1;

      userNode_t *returnedNode = SearchUserByFD(arg->head,arg->fd);
      fprintf(stderr,"get the remove target : %s\n",returnedNode->userName);
      if(arg->head->next==NULL){
        arg->head->fd = -1;
        arg->head->userName = NULL;
        fprintf(stderr,"genter remive head \n");
      }else{
        arg->head = removeFromUserList(returnedNode,arg->head);
        fprintf(stderr,"new header%s",arg->head->userName);
      }





    //  fprintf(stderr,"33333333333333333333333333333333333333333333333333333333333333333333333333\n");
    }else if(strcmp("MSG",token)==0){
    //  fprintf(stderr,"server enter the MSG\n");
      token = strtok(NULL," ");
      //fprintf(stderr,"to User : %s\n",token);
      userNode_t *targetUser = SearchUserByName(arg->head,token);
      if(targetUser!=NULL){
        //fprintf(stderr,"find : %s\n",targetUser->userName);
        send(targetUser->fd,readBuffer,strlen(readBuffer),0);
        send(arg->fd,readBuffer,strlen(readBuffer),0);


      }else{
        sfwrite(&count_mutex,stdout,"User DNE\n");
        char* error01String = "ERR 01 USER NOT AVAILABLE \r\n\r\n";
        send(arg->fd,error01String,strlen(error01String),0);
      }

    }
  }
return NULL;
//fprintf(stderr,"out the getdata thread\n");
}



void *getCommandThread(void *vargp){
  thread_command *arg = (thread_command*)vargp;
  pthread_detach(pthread_self());
  //fprintf(stderr,"enter command\n");
  char buf[MAXLINE];
  while (fgets(buf,MAXLINE,stdin)!=NULL){
      //fprintf(stderr,"%s\n",buf);
    if(strcmp(buf,"/help\n")==0){
      help();
    }
    if(strcmp(buf,"/users\n")==0){
    //  fprintf(stderr,"enter printer users\n");
      //fprintf(stderr,"head user %s\n",arg->head->userName);
      userNode_t* head = (userNode_t*)malloc(sizeof(userNode_t));
      head = arg->head;
      printUserList(head);
    }
    if(strcmp(buf,"/shutdown\n")==0){
      closeAllSocket(arg->head,arg->listenfd);
    }
    if(strcmp(buf,"/accts\n")==0){
      accNode_t* head = (accNode_t*)malloc(sizeof(accNode_t));
      head = arg->accHead;
      printAccount(head);
    }

  }
//fprintf(stderr,"out command\n");
return NULL;
}

void *loginThread(void *vargp){
  char readBuff[MAXLINE];
  memset(&readBuff,0,sizeof(readBuff));
  char* token;
  thread_fd *fd = (thread_fd*)vargp;
  int connfd = fd->fd;
  free(vargp);
  //fprintf(stderr, "connfd inside the login thread %d", connfd);
  char* NAME= (char* )malloc(MAXLINE);


  int readbyte = read(connfd,&readBuff,MAXLINE);
  //fprintf(stderr, "readbuff %s\n", readBuff);


  if(readbyte!=0){
    if(strcmp("WOLFIE \r\n\r\n",readBuff)==0){
      char* eiflowString = "EIFLOW";
      char* generatedEiflow =  wolfieRequestFactory(eiflowString);
      write(connfd,generatedEiflow,strlen(generatedEiflow));



      int receiveIM = 0;
      while(!receiveIM){
        memset(&readBuff,0,sizeof(readBuff));
        int readbyte = read(connfd,&readBuff,MAXLINE);
        //fprintf(stderr, "readbuff %s\n", readBuff);
        if(readbyte!=0){
          token = strtok(readBuff," ");
          if(strcmp(token,"IAM")==0){
            token = strtok(NULL, " ");
            strcpy(NAME,token);
            //fprintf(stderr,"token %s \n",token);


            /*check if the userAccout exist*/
            if(fd->accHead->userName == NULL){
              /*no account*/
              char* error01String = "ERR 01 USER NOT AVAILABLE \r\n\r\n";
              send(connfd,error01String,strlen(error01String),0);
              //fprintf(stderr,"auth = %d",bytessent);

              char *byeString = "BYE \r\n\r\n";
              send(connfd,byeString,strlen(byeString),0);
              receiveIM =1;
              break;
            }else{
              if(checkAccUserName(fd->accHead,token)!=0){
                /*no account*/
                char* error01String = "ERR 01 USER NOT AVAILABLE \r\n\r\n";
                send(connfd,error01String,strlen(error01String),0);
                //fprintf(stderr,"auth = %d",bytessent);

                char *byeString = "BYE \r\n\r\n";
                send(connfd,byeString,strlen(byeString),0);
                receiveIM =1;
                break;
              }else{
                /*found account*/

                if(fd->head->fd == -1){
                  char authString[MAXLINE]={};
                  strcpy(authString,"AUTH ");
                  strcat(authString,token);
                  strcat(authString," ");
                  strcat(authString,"\r\n\r\n");
                  send(connfd,authString,strlen(authString),0);
                  //fprintf(stderr,"auth = %d",bytessent);
                  //break;
                }else{
                  if(checkUserName(fd->head,token)!=0){
                    char authString[MAXLINE]={};
                    strcpy(authString,"AUTH ");
                    strcat(authString,token);
                    strcat(authString," ");
                    strcat(authString,"\r\n\r\n");
                    send(connfd,authString,strlen(authString),0);
                    //fprintf(stderr,"auth = %d",bytessent);
                    //break;
                  }else{

                    char *error00String = "ERR 00 USER NAME TAKEN \r\n\r\n";
                    send(connfd,error00String,strlen(error00String),0);
                    //fprintf(stderr,"same user nanr bytessent = %d",bytessent);


                    char *byeString = "BYE \r\n\r\n";
                    send(connfd,byeString,strlen(byeString),0);

                    receiveIM =1;
                    break;
                  }
                }


              }

            }







/*




              if(fd->head->fd == -1){

                char* generatedHIString = HImessageFactory(token);
                int bytessent = send(connfd,generatedHIString,strlen(generatedHIString),0);


                fprintf(stderr,"hi verb bytessent = %d",bytessent);
                fprintf(stderr,"add to head\n");

                fd->head->fd = connfd;
                fd->head->userName = token;
                fd->head->next = NULL;
                fd->head->prev = NULL;
                fd->head->time = time(NULL);
                char* c_time_string;
                c_time_string = ctime(&(fd->head->time));
                printf("Current time is %s", c_time_string);
                add_client(connfd,fd->pl);


                char* generatedMOTDString = MOTDmessageFactory(fd->message_input);
                bytessent = send(connfd,generatedMOTDString,strlen(generatedMOTDString),0);
                fprintf(stderr,"motd bytessent = %d",bytessent);


              }else{
                if(checkUserName(fd->head,token)!=0){

                  char* generatedHIString = HImessageFactory(token);
                  int bytessent = send(connfd,generatedHIString,strlen(generatedHIString),0);
                  fprintf(stderr,"push to\n");
                  pushToUserList(fd->head,token,connfd);
                  add_client(connfd,fd->pl);


                  char* generatedMOTDString = MOTDmessageFactory(fd->message_input);
                  bytessent = send(connfd,generatedMOTDString,strlen(generatedMOTDString),0);
                  fprintf(stderr,"motd bytessent = %d",bytessent);


                }else{
                  fprintf(stderr,"same userName\n");
                  char *error00String = "ERR 00 USER NAME TAKEN \r\n\r\n";
                  int bytessent = send(connfd,error00String,strlen(error00String),0);
                  fprintf(stderr,"same user nanr bytessent = %d",bytessent);


                  char *byeString = "BYE \r\n\r\n";
                  bytessent = send(connfd,byeString,strlen(byeString),0);

                }
              }

*/




              printUserList(fd->head);


              //receiveIM=1;
            }else if(strcmp(token,"IAMNEW")==0){



              token = strtok(NULL, " ");
              strcpy(NAME,token);

              //fprintf(stderr,"token %s \n",token);
              /*if this is the first login user add to head of list*/
                if(fd->accHead->userName == NULL){
                  /*sent the hi message*/
                  char* generatedHINEWString = HINEWmessageFactory(token);
                  send(connfd,generatedHINEWString,strlen(generatedHINEWString),0);
                  //fprintf(stderr,"hi new verb bytessent = %d",bytessent);

                  ///int readbyte = read(connfd,&readBuff,MAXLINE);




                }else{
                  if(checkAccUserName(fd->accHead,token)!=0){
                    /*sent the hi message*/
                    char* generatedHINEWString = HINEWmessageFactory(token);
                    send(connfd,generatedHINEWString,strlen(generatedHINEWString),0);
                    //fprintf(stderr,"same user nanr bytessent = %d",bytessent);
                  }else{
                    //fprintf(stderr,"same userName\n");
                    char *error00String = "ERR 00 USER NAME TAKEN \r\n\r\n";
                    send(connfd,error00String,strlen(error00String),0);
                    //fprintf(stderr,"same user nanr bytessent = %d",bytessent);
                    char *byeString = "BYE \r\n\r\n";
                    send(connfd,byeString,strlen(byeString),0);
                    receiveIM=1;
                  }
                }

            }else if(strcmp(token,"NEWPASS")==0){
              token = strtok(NULL, " ");
              //fprintf(stderr,"token %s \n",token);
              int isValid = checkPassword(token);
              if(isValid ==1){
                char* SSAPWEN = "SSAPWEN \r\n\r\n";
                send(connfd,SSAPWEN,strlen(SSAPWEN),0);
                /*get hi send*/
                /*this token should be the name of the user who trying to connect to the server*/
                /*add to account list*/
                if(fd->accHead->userName == NULL){
                  fd->accHead->userName = NAME;
                  fd->accHead->password = token;
                  //fprintf(stderr,"the password created %s\n",token);
                }else{
                  pushAccountList(fd->accHead, NAME, token);
                }



                char* generatedHIString  =  HImessageFactory(NAME);
                //fprintf(stderr,"HI XXX  : %s\n",generatedHIString);
                send(connfd, generatedHIString,strlen(generatedHIString),0);

                /*add to login user list*/
                if(fd->head->fd == -1){
                fd->head->fd = connfd;
                fd->head->userName = NAME;
                fd->head->next = NULL;
                fd->head->prev = NULL;
                fd->head->time = time(NULL);
                char* c_time_string;
                c_time_string = ctime(&(fd->head->time));
                printf("Current time is %s", c_time_string);
                add_client(connfd,fd->pl);
              }else{
                //fprintf(stderr,"push to\n");
                pushToUserList(fd->head,NAME,connfd);
                add_client(connfd,fd->pl);

              }




                char* generatedMOTDString = MOTDmessageFactory(fd->message_input);
                send(connfd,generatedMOTDString,strlen(generatedMOTDString),0);
                //fprintf(stderr,"motd bytessent = %d",byteSent);
                /*add user*/









              }else{
                char *error02String = "ERR 02 BAD PASSWORD \r\n\r\n";
                 send(connfd,error02String,strlen(error02String),0);
                //fprintf(stderr,"same user nanr bytessent = %d",bytessent);
                char *byeString = "BYE \r\n\r\n";
                send(connfd,byeString,strlen(byeString),0);
                receiveIM=1;
              }
              receiveIM=1;
            }else if(strcmp(token,"PASS")==0){




              token = strtok(NULL, " ");
              //fprintf(stderr,"token %s \n",token);
              int isValid = matchAccUserPassword(fd->accHead, NAME, token);
              if(isValid ==1){
                char* SSAPWEN = "SSAP \r\n\r\n";
                send(connfd,SSAPWEN,strlen(SSAPWEN),0);
                /*get hi send*/
                /*this token should be the name of the user who trying to connect to the server*/
                /*add to account list*/



                char* generatedHIString  =  HImessageFactory(NAME);
                //fprintf(stderr,"HI XXX  : %s\n",generatedHIString);
                send(connfd, generatedHIString,strlen(generatedHIString),0);

                /*add to login user list*/
                if(fd->head->fd == -1){
                fd->head->fd = connfd;
                fd->head->userName = NAME;
                fd->head->next = NULL;
                fd->head->prev = NULL;
                fd->head->time = time(NULL);
                char* c_time_string;
                c_time_string = ctime(&(fd->head->time));
                printf("Current time is %s", c_time_string);
                add_client(connfd,fd->pl);
              }else{
                //fprintf(stderr,"push to\n");
                pushToUserList(fd->head,NAME,connfd);
                add_client(connfd,fd->pl);

              }




                char* generatedMOTDString = MOTDmessageFactory(fd->message_input);
                send(connfd,generatedMOTDString,strlen(generatedMOTDString),0);
              //  fprintf(stderr,"motd bytessent = %d",byteSent);
                /*add user*/









              }else{
                char *error02String = "ERR 02 BAD PASSWORD \r\n\r\n";
                send(connfd,error02String,strlen(error02String),0);
                //fprintf(stderr,"same user nanr bytessent = %d",bytessent);
                char *byeString = "BYE \r\n\r\n";
                send(connfd,byeString,strlen(byeString),0);
                receiveIM=1;
              }
              receiveIM=1;








            }
          }

      }
    }
  }


  //write(connfd,"WOFILE",6);write(connfd,"WOFILE",6);
  //fsync(connfd);
  //close(connfd);
  //pipe(&connfd);


  return NULL;
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
void pushAccountList(accNode_t * head, char* userName, char* password) {
    accNode_t * current = head;
    while (current->next != NULL) {
        current = current->next;
    }
    /* now we can add a new variable */
    current->next = (accNode_t*)malloc(sizeof(accNode_t));
    current->next->userName = userName;
    current->next->password = password;
    current->next->next = NULL;
    current->next->prev = current;
}



userNode_t* removeFromUserList(userNode_t* toRemove,userNode_t* head){
  userNode_t * current = toRemove;
if(current->prev!=NULL && current->next!=NULL){
current->prev->next = current->next;
current->next->prev = current->prev;
return head;
}else if(current->prev!=NULL && current->next==NULL){
current->prev->next = NULL;
return head;
}else if(current->prev==NULL && current->next!=NULL){
head = current->next;
return head;
}
return head;
}



int checkUserName(userNode_t* head, char* userName){
  userNode_t * current = head;
  if(strcmp(current->userName,userName)==0){
    return 0;
  }
  while (current->next != NULL) {
      current = current->next;
      if(strcmp(current->userName,userName)==0){
        return 0;
      }
  }
  return 1;
}

int checkAccUserName(accNode_t* accHead, char* userName){
  accNode_t * current = accHead;
  if(strcmp(current->userName,userName)==0){
    return 0;
  }
  while (current->next != NULL) {
      current = current->next;
      if(strcmp(current->userName,userName)==0){
        return 0;
      }
  }
  return 1;

}
/*
char* makeUserList(userNode_t * head){
  char userList[MAXLINE];
  userNode_t * current = head;
  while(current != NULL){
    strcpy(userList,"UTSIL ");
    strcat(userList,current->userName);
    strcat(userList,"\r\n");
    current = current->next;

  }
  return userList;
}
*/
void printUserList(userNode_t * head) {
    userNode_t * current = head;
    while (current != NULL) {

        sfwrite(&count_mutex,stdout,"%s\n", current->userName);
        sfwrite(&count_mutex,stdout,"%d\n", current->fd);
        //fprintf(stdout,"%s\n", current->userName);
        //fprintf(stdout,"%d\n", current->fd);
        current = current->next;
    }
}


void printAccount(accNode_t * head) {
    accNode_t * current = head;
    while (current != NULL) {

        sfwrite(&count_mutex,stdout,"%s\n", current->userName);
        sfwrite(&count_mutex,stdout,"%s\n", current->password);
        //fprintf(stdout,"%s\n", current->userName);
        //fprintf(stdout,"%s\n", current->password);
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

void help(){

  sfwrite(&count_mutex,stdout,"/users                 dump a list of currently logged in users to stdout\n");
  sfwrite(&count_mutex,stdout,"/help                  list all the commands which the server accepts and what they do\n");
  sfwrite(&count_mutex,stdout,"/shutdown              shutdown and cleanly disconnect all connected users\n");
/*
  fprintf(stdout,"/users                 dump a list of currently logged in users to stdout\n");
  fprintf(stdout,"/help                  list all the commands which the server accepts and what they do\n");
  fprintf(stdout,"/shutdown              shutdown and cleanly disconnect all connected users\n");
  */
}

void closeAllSocket(userNode_t* head,int listenfd){
  userNode_t * current = head;
  while (current != NULL) {
      int userfd = current->fd;
      fprintf(stderr,"close all userfd = %d\n", userfd);
      int bytesent = send(userfd,"BYE \r\n\r\n",8,0);
      fprintf(stderr,"%d\n",bytesent);
      shutdown(userfd,2);
      current= current->next;
  }
  shutdown(listenfd,2);
  exit(0);
}
void signalhandler(int signo){
  if(signo == SIGINT){
    sfwrite(&count_mutex,stdout,"received Signal\n");
    //closeAllSocket(head,listenfd);
  }
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

char* HImessageFactory(char* userName){
  char *buffer = (char*)malloc(sizeof buffer);
  strcpy(buffer,"HI ");
  strcat(buffer,userName);
  strcat(buffer," \r\n\r\n");
  return buffer;
}

char* HINEWmessageFactory(char* userName){
  char *buffer = (char*)malloc(sizeof buffer);
  strcpy(buffer,"HINEW ");
  strcat(buffer,userName);
  strcat(buffer," \r\n\r\n");
  return buffer;
}

char* MOTDmessageFactory(char* motdMessage){
  char *buffer = (char*)malloc(sizeof buffer);
  strcpy(buffer,"MOTD ");
  strcat(buffer,motdMessage);
  strcat(buffer," \r\n\r\n");
  return buffer;
}

char* EMITFactory(char* time){
  char *buffer = (char*)malloc(sizeof buffer);
  strcpy(buffer,"EMIT ");
  strcat(buffer,time);
  strcat(buffer," \r\n\r\n");
  return buffer;
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


int matchAccUserPassword(accNode_t* accHead, char* userName, char* password){
  accNode_t * current = accHead;
  if(strcmp(current->userName,userName)==0){
    if(strcmp(current->password,password)==0){
      return 1;
    }
  }
  while (current->next != NULL) {
      current = current->next;
      if(strcmp(current->userName,userName)==0){
        if(strcmp(current->password,password)==0){
          return 1;
        }else{
        return 0;
        }
      }
  }
  return 0;

}
