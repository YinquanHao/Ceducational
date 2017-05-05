#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

typedef struct node {
    char *cmd;
    struct node * next;
    struct node * prev;
} node_t;

typedef struct Stack {
    char data[1024];
    int size;
} Stack;

// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024
#define MAX_ARGUMENT 256
void removeMutipleSpace(char* , char []);
int buildin(char** );
void exit(int status);
int parseline(char *, char **);
int evaluate(char *);
char *getenv(const char *);
char *getcwd(char *, size_t );
pid_t fork(void);
pid_t waitpid(pid_t pid, int *status, int options);
int execvpe(const char *file, char *const argv[],char *const envp[]);
int doesFileExist(const char *filename);
int containSlash(char* filename);
char* removeNewline(char* cmd);
void push(node_t * , char* str);
void print_list(node_t * );
long int strtol(const char *nptr, char **endptr, int base);
void removeNewLineSpace(char* cmdBuffer, char modify[]);
void moveCursorBackN(int N);
int checklastthree(char* cursor);
void moveCursorForwardN(int N);
void eraseAfterCursor();
node_t* getprev(node_t* current);
void writeToCmd(char* cursor, char target[]);
node_t * returnLastNode(node_t* node_t);
void print_listBackward(node_t* node_t);
node_t* getNext(node_t* current);
void writeHistory(FILE* fp,node_t* head);
int checkIsEmpty(char*);
void Stack_Init(Stack *S);
char Stack_Top(Stack *S);
void Stack_Push(Stack *S, char d);
void Stack_Pop(Stack *S);
int* spliteCmdByRedirec(char* argv[]);
int findInBracket(char* argv[]);
void mutiplePipEvaluate(int* splitedArray, char *argv[MAX_ARGUMENT]);
char* removeNewlineSp(char* cmd);
//int stat(const char *restrict path, struct stat *restrict buf);

//prevDirec is used to store the previous directory
char* prevDirec;
extern char** environ;









int main (int argc, char ** argv, char **envp) {
  int debugMode = 0;/*0 for non-debug mode and 1 for debug mode*/
  int finished = 0;
  node_t* head =NULL;
  //head->prev = NULL;
  //head->next = NULL;
  node_t* tail = head;
  int prevUp = 0;
  //fprintf(stderr,"qutoe decimal: %d",'"');




  /*
  Stack_Push(arrowStack,'a');
  Stack_Push(arrowStack,'b');
  fprintf(stderr,"top of the stack %c",Stack_Top(arrowStack));
  Stack_Pop(arrowStack);
  fprintf(stderr,"top of the stack %c",Stack_Top(arrowStack));
  */


  //node_t* current = head;
  char zero[] = "0";
  //int arrowValue = -1;
  setenv("?", zero,1 );
  int exitStatus=0;
  char *relevant_history_directory = (char*)malloc(100);
  int escape = 0;

  strcat(relevant_history_directory,getenv("HOME"));

  strcat(relevant_history_directory,"/historylog");
  //fprintf(stderr,"%s",relevant_history_directory);



  FILE* fp;
  char* buf = (char*)malloc(1024);

  if ((fp = fopen(relevant_history_directory, "ab+")) == NULL)
  {
    perror("fopen source-file");
  }

  while (fgets(buf, sizeof(buf)+1, fp) != NULL)
  {
    //buf[strlen(buf+1)] =; // eat the newline fgets() stores
    char* modifyCmd = (char*) malloc(MAX_INPUT);
    removeNewLineSpace(buf,modifyCmd);
    if(head == NULL){
      head = (node_t*)malloc(sizeof(node_t));
      //fprintf(stderr,"head ==NULL\n");
      //fprintf(stderr,"enter push: %s",buf);
      head->cmd = modifyCmd;
    }else{
      //fprintf(stderr,"enter push: %s",buf);
      push(head, modifyCmd);

      //tail = returnLastNode(head);
     }
    //printf("%s\n", buf);


  }
  fclose(fp);
  //remove(relevant_history_directory);
  fprintf(stderr, "print headdddddd");
  print_list(head);
  fprintf(stderr, "print headdddddd    ending");

  if(head!=NULL){
    fprintf(stderr, "head !-=nulll");
  print_list(head);
  tail = returnLastNode(head);
  if(tail!=NULL){
  tail = tail->prev;
}
}

 //fprintf(stderr,"tail: %s\n",tail->cmd);




/*
    char * line = NULL;
    size_t len = 0;
    ssize_t readsize;
    FILE *fp = fopen(relevant_history_directory,"ab+");
    //fp = fopen("/etc/motd", "r");
    if (fp == NULL){
        fprintf(stderr, "fp is NULL" );
        exit(EXIT_FAILURE);
      }
      //fprintf(stderr, "fp is after" );
    while ((readsize = getline(&line, &len, fp)) != -1) {
        fprintf(stderr, "fp is NULL" );
        fprintf(stderr,"Retrieved line of length %zu :\n", readsize);
        fprintf(stderr,"%s", line);
        if(head == NULL){
          head = (node_t*)malloc(sizeof(node_t));
          fprintf(stderr, "fp is NULL" );
          //fprintf(stderr,"head ender: %s",cmd);
          head->cmd = line;
        }else{
          //fprintf(stderr,"enter push: %s",cmd);
          push(head, line);

          //tail = returnLastNode(head);
         }

    }

    fclose(fp);

*/
  /*
  char relevant_history_directory[200];
  strcpy(relevant_history_directory,getenv("HOME"));

  char history_directory[] = "/historylog";
  strcat(relevant_history_directory,history_directory);

  fprintf(stderr,"%s",relevant_history_directory);
*/

  /*
  if(!fp){
    printf("efftot");
  }
  */
  //char *prompt = "320sh> ";



  char *buffprompt = (char*)malloc(MAX_INPUT);
  char *cwd = getcwd(buffprompt,MAX_INPUT);

  char *prompt = strcat(cwd,"] 320sh> ");

  if(argc>1 ){
    //fprintf(stderr, "argv: %s\n", argv[1]);
    if(!strcmp(argv[1],"-d")){
    debugMode =1;
  }
  }
  //fprintf(stderr, "argc: %d\n", argc);

  //prompt = strcat("[",prompt);

  //char *test = "enter this->";

  char cmd[MAX_INPUT];
  char OpenBrac[3]=" [";
  prompt = strcat(OpenBrac,prompt);


  while (!finished) {
    char *cursor;
    char last_char;
    int rv;
    int count;
    int countBacked;

    Stack* arrowStack= NULL;
    arrowStack = (Stack *)malloc(sizeof(Stack));


    // Print the prompt
    rv = write(1, prompt, strlen(prompt));

    if (!rv) {
      finished = 1;
      break;
    }

    int escapeCount =0;
    // read and parse the input
    for(rv = 1, count = 0,
	  cursor = cmd, last_char = 1, countBacked =0;
	  rv
	  && (++count < (MAX_INPUT-1))
	  && (last_char != '\n');
	cursor++) {

      //write(1,test,strlen(prompt));
      rv = read(0, cursor, 1);
      last_char = *cursor;
      //fprintf(stderr, "\nlasr_cqweqwehar: %d \n",last_char);
      //fprintf(stderr, "\n cmdcmd: %s \n",cmd);
      //printf("cursor address: %p ",cursor);
      /*detect the escape character*/
      if(last_char==27){
        escape=1;
        escapeCount=0;
      }


      if(escape != 1){

      if(last_char == 3) {
        write(1, "^c", 2);
      }else if(last_char == 127){
        if(count>0){
          if(arrowStack->size ==0){
        moveCursorBackN(1);
        eraseAfterCursor();
        cursor = cursor-2;
        count = count -1;
      }else{
        //ssmoveCursorBackN(1);
        eraseAfterCursor();
        cursor = cursor-1;
        count = count -1;
        //Stack_Pop(arrowStack);
        while(arrowStack->size>0){
          char current = Stack_Top(arrowStack);
          moveCursorBackN(1);
          *cursor = current;
          write(1,&current,1);
          Stack_Pop(arrowStack);
          count++;
          cursor++;
        }
        moveCursorBackN(1);
        cursor --;
      }

      }
        count--;
      }else {
         //fprintf(stderr, "enter wite tot file");
	       write(1, &last_char, 1);
      }

    }/*<-----if escape = false*/else{
      escapeCount++;
      if(escapeCount==3){
        /*left*/
        if(last_char==68){

          *cursor = '\0';
          cursor =cursor -1;

          *cursor = '\0';
          cursor = cursor-1;

          *cursor = '\0';
          cursor = cursor -1;

          if(count>0){
          moveCursorBackN(1);
          count--;



          char current = *cursor;
          Stack_Push(arrowStack,current);
          //fprintf(stderr,"top of the stack %c",Stack_Top(arrowStack));
          //fprintf(stderr,"size of the stack %d",arrowStack->size);





          cursor = cursor -1;



          //count--;
          countBacked++;
          }
        }
          /*right*/
        if(last_char==67){
          //char ptrchar= *(cursor);
          fprintf(stderr, "cursor char %s\n", cursor );
          *cursor = '\0';
          cursor =cursor -1;
          //ptrchar= *(cursor);
          //fprintf(stderr, "cursor char %c\n", ptrchar );
          fprintf(stderr, "cursor char %s\n", cursor );
          *cursor = '\0';
          cursor = cursor-1;
          //ptrchar= *(cursor);
          //fprintf(stderr, "cursor char %c\n", ptrchar );
          fprintf(stderr, "cursor char %s\n", cursor );
          *cursor = '\0';
          cursor = cursor -1;

          if(countBacked>0){
            moveCursorForwardN(1);
            countBacked--;
            count++;

            fprintf(stderr, "cursor char this %s\n", cursor );

            cursor++;

            /*this curosr position is correct*/
          fprintf(stderr, "cursor char %s\n", cursor );


          }

        }
        if(last_char==65){
          //fprintf(stderr, " head%s\n", head->cmd);
          /*
          if(head!=NULL){
            fprintf(stderr, "head !-=nulll");
          print_list(head);
          tail = returnLastNode(head);
        }
        */
          *cursor = '\0';
          cursor =cursor -1;
          //ptrchar= *(cursor);
          //fprintf(stderr, "cursor char %c\n", ptrchar );
          *cursor = '\0';
          cursor = cursor-1;
          //ptrchar= *(cursor);
          //fprintf(stderr, "cursor char %c\n", ptrchar );
          *cursor = '\0';
          cursor = cursor -1;


          moveCursorBackN(count);
          countBacked = countBacked + count;
          eraseAfterCursor();
          count = 0;
          //fprintf(stderr, "check pt\n" );
          size_t length;

          if(tail!=NULL){

          node_t* prev = getprev(tail);
          if(prevUp==0){
            prev = getNext(prev);
          }

          //fprintf(stderr, "check pt1\n" );
          char* prevcmd = prev -> cmd;
          tail = getprev(tail);
          //write(1, prevcmd, strlen(prevcmd));
          length = strlen(prevcmd);
          //fprintf(stderr,"length %zu", length);
          prevcmd = removeNewline(prevcmd);
          //fprintf(stderr,"prevcmd123%s\n",prevcmd);
          write(1, prevcmd, length);
          writeToCmd(prevcmd, cmd);
          count = count + length;
        }else{
          fprintf(stderr,"NULL");
          length =1;
        }

          cursor = cursor+length;
        //  fprintf(stderr,"nive213%s\n",cmd);
          //moveCursorBackN(count-1);
          //cursor = cursor -4 - count;
        }
        if(last_char==66){
          *cursor = '\0';
          cursor =cursor -1;
          //ptrchar= *(cursor);
          //fprintf(stderr, "cursor char %c\n", ptrchar );
          *cursor = '\0';
          cursor = cursor-1;
          //ptrchar= *(cursor);
          //fprintf(stderr, "cursor char %c\n", ptrchar );
          *cursor = '\0';
          cursor = cursor -1;
          if(tail!=NULL){
          moveCursorBackN(count);
          countBacked = countBacked + count;
          eraseAfterCursor();
          count = 0;
          node_t* prev = getNext(tail);
          char* prevcmd = prev -> cmd;

          tail = getNext(tail);
          size_t length = strlen(prevcmd);
          write(1, prevcmd, length);
          writeToCmd(prevcmd, cmd);
          //fprintf(stderr,"nive%s\n",cmd);
          cursor = cursor+length;
}
        }
        escape=0;

      }
      count--;
    }



    }
    //fprintf(stderr,"nive111111111111111111111111%s\n",cmd);
    *cursor = '\0';

    if (!rv) {
      finished = 1;
      break;
    }


    // Execute the command, handling built-in commands separately
    // Just echo the command line for now
    // write(1, cmd, strnlen(cmd, MAX_INPUT));
     if(debugMode==1){
       fprintf(stderr,"RUNNING: %s",cmd);
     }
     char* modifyCmd = (char*)malloc(MAX_INPUT);

     removeNewLineSpace(cmd,modifyCmd);
     //removeNewline(modifyCmd);
     int isEmpty = checkIsEmpty(modifyCmd);
     //fprintf(stderr,"check is emplty value%d\n",isEmpty);
     if(isEmpty==1){
     if(head == NULL){
       head = (node_t*)malloc(sizeof(node_t));
       fprintf(stderr,"head ==NULL");
       head->cmd = modifyCmd;
     }else{
       //fprintf(stderr,"enter push: %s",cmd);
       push(head, modifyCmd);
       //fprintf(stderr,"modify::::%s:::::::",modifyCmd);

       tail = returnLastNode(head);
      }
      /*
     print_list(head);
     fprintf(stderr,"reach this point");
     fprintf(stderr,"print bacl");
     print_listBackward(tail);
     */

   }else{
     //fprintf(stderr,"empty cmd");
      }
     //fprintf(stderr,"printend");



     exitStatus = evaluate(cmd);




     if(exitStatus ==-1){
       FILE *fp = fopen(relevant_history_directory,"ab+");
       if(!fp){
         fprintf(stderr,"error");
       }

       writeHistory(fp,head);
       //while()
       //fputs(cmd,fp);
       fclose(fp);
       exit(3);
     }





     /*
     FILE *fp = fopen(relevant_history_directory,"ab+");

     if(!fp){
       fprintf(stderr,"error");
     }

     fputs(cmd,fp);
     fclose(fp);
     */










     if(debugMode==1){
       fprintf(stderr,"ENDED: %s (ret = %d)\n",removeNewline(cmd),exitStatus);
     }
     /*
     write(1,"abc\n" , strlen("abc\n"));
     write(1,prevDirec , strlen(prevDirec));
     write(1,"abc\n" , strlen("abc\n"));
     */
//  write(1, "12312312312312312\n", strlen("12312312312312312\n"));
  /*
  char** env;
  for (env = envp; *env != 0; env++){
    char* thisEnv = *env;
    printf("%s\n", thisEnv);
  }
  */

  cwd = getcwd(buffprompt,MAX_INPUT);
  //prompt =NULL;
  prompt = strcat(cwd,"] 320sh> ");
  char a[] = "[";
  prompt = strcat(a,prompt);

}


  return 0;
}

int evaluate(char *cmd){

  //fprintf(stderr,"evaluate intake cmd::::%s:::::::::: \n",cmd);
  char *argv[MAX_ARGUMENT];
  char cmdBuffer[MAX_INPUT];
  int background;
  pid_t pid;
  strcpy(cmdBuffer,cmd);
  //printf("123123213");
  cmdBuffer[strlen(cmdBuffer)-1]=' ';


  background = parseline(cmdBuffer,argv);
  //int small = getSmallestQuote(argv);


//  int* splitedArray  = spliteCmdByRedirec(argv);

  //size_t numberOfOperater = sizeof(splitedArray)/(sizeof(splitedArray[0]));

  //mutiplePipEvaluate(splitedArray,argv);

  //size_t numberOfSplitedArgv =numberOfOperater+1;
/*
  char ***subArgvs = (char***) malloc(numberOfSplitedArgv);
  char* currentPtr = argv[]
  for (int i=0;i<numberOfSplitedArgv;i++){

  }
  */

  //fprintf(stderr,"splitedArray Size %lu",sizeof(splitedArray)/(sizeof(splitedArray[0])));
  /*
  size_t k=0;
  while(k<(sizeof(splitedArray)/(sizeof(splitedArray[0])))){

    fprintf(stderr,"solitedArray: %d", *(splitedArray+k));
    k++;
  }
*/



/*for redirec*/

  int inBrac = findInBracket(argv);

  char *newargv[256];
  char *newargv1[256];

  if(inBrac>0){

    int i=0;
    for(i=0;i<inBrac;i++){
      newargv[i] = argv[i];
    }
    i=0;
    while(argv[inBrac+i]!=NULL){
      newargv1[i] =argv[inBrac+i];
      i++;
    }
    i=0;
    while(newargv[i]!=NULL){
      //fprintf(stderr,"new argv%s\n",newargv[i]);
      i++;
    }
    i=0;
    while(newargv[i]!=NULL){
      //fprintf(stderr,"new argv1%s\n",newargv1[i]);
      i++;
    }
  }

/*for redirec*/


  //fprintf(stderr,"inbeac: %d\n",inBrac);

  int i=0;
  for(i=0;i<(MAX_ARGUMENT-1);i++){
    if(argv[i]!=NULL){
  //  printf("%d:%s:\n",i, argv[i]);
  }else{
    break;
  }
  }

  if(argv[0]==NULL){
    return 0;
  }

  int result =0;



  if(inBrac>0){
    int i=0;
    while(i<256){
      argv[i] = newargv[i];
      i++;
    }
    i=0;
    while(argv[i]!=NULL){
      fprintf(stderr,"123new argv%s\n",argv[i]);
      i++;
    }
  }




  if((result = buildin(argv))==0){
    /*none buildin functions*/
    if((pid = fork())==0){/*child process run user job*/
      int a=0;

      int isAbsPath = containSlash(argv[0]);
      int execute = 0;
      //printf("%d: Does containSlash. \n",isAbsPath);
      if(isAbsPath ==1){
        if(doesFileExist(argv[0])==0){
          printf("%s: file not found123. \n", argv[0]);
          execute = 0;
          exit(0);
        }else{
          execute =1;
        }
      }else{
        execute =1;
      }
      if(execute ==1){

/*for redirec*/
        FILE *fpwrite;

        if(inBrac>0){
        //FILE *fpwrite;
        if((fpwrite = fopen(newargv1[1], "ab+")) == NULL)
        {
          perror("fopen source-file");
        }
        dup2(fpwrite->_fileno,stdout->_fileno);
      }

      /*for redirec*/

      if((a=execvpe(argv[0],argv, environ))<0){
        fprintf(stderr,"%s: Command not found. \n", argv[0]);
        char one[] = "1";
        setenv("?",one,1);
        return 1;
        exit(0);
      }


/*for redirec*/
      if(inBrac>0){
      fclose(fpwrite);
      }
    }

/*for redirec*/


    }




    if(background==0){
      int status;

      if(waitpid(pid,&status,0)<0){
        printf("waitfg:waitpid error");
      }
      else{
        //printf("%d %s",pid, cmd);

      }
      char *statusBuffer = (char*) malloc(200);
      sprintf(statusBuffer,"%d",status);
      setenv("?", statusBuffer,1 );
      free(statusBuffer);
      //fprintf(stderr,"status command %d",status);
    }
    /*this else block is for the  build in set the $? to zero*/
  }else if(result == -1){
    return -1;
  }else{
    char zero[]="0";
    setenv("?",zero,1);
  }

char* endptr;
int value =(int) strtol(getenv("?"),&endptr,10);

return value;

}


int parseline(char *cmdBuffer, char **argv){
  char *argvptr;
  int argc = 0;
  char *modify = (char*)malloc(MAX_INPUT);

  /*ignore the leading space*/
  while(*cmdBuffer && (*cmdBuffer==' ')){
    cmdBuffer++;
  }
  /*make all mutiple space to exactly one, and store the new buffer in char* modify*/
  removeMutipleSpace(cmdBuffer, modify);
//  printf("modify:%s\n",modify);

  argvptr = strtok(modify," ");

  while(argvptr!=NULL){
    argv[argc]=argvptr;
    argc++;
    argvptr = strtok(NULL," ");
  }
  argv[argc]=NULL;

  if(argc==0){
    return 1;
  }

  return 0;

}

int buildin(char** argv){
  /*handle the buid in  exit*/
  //fprintf(stderr,"print argv inside buildin argv[0]:::::%s", argv[0]);








  if(!strcmp(argv[0],"exit")){
  //  FILE *fp = fopen(relevant_history_directory,"ab+");
  //  writeHistory(fp,head);



    //exit(3);
    return -1;
  }

  /*handle the buid in  pwd get current directory*/
  if(!strcmp(argv[0],"pwd")){
    char* cwd;
    char buff[MAX_INPUT];
    cwd = getcwd(buff, MAX_INPUT);
     fprintf(stderr, "%s\n", cwd);
     //write(1, "\n", strlen("\n"));
     return 1;
  }

  if(!strcmp(argv[0],"cd")){
    char *buff = (char*)malloc(MAX_INPUT);
    if(argv[1]==NULL){
      prevDirec = getcwd(buff,MAX_INPUT);
      chdir(getenv("HOME"));
    }else if(!strcmp(argv[1],"..")){
      prevDirec = getcwd(buff,MAX_INPUT);
      chdir("..");
    }else if(!strcmp(argv[1],".")){
      prevDirec = getcwd(buff,MAX_INPUT);
      chdir(".");
    }else if(!strcmp(argv[1],"-")){
      char* temp = getcwd(buff, MAX_INPUT); ;
      chdir(prevDirec);
      prevDirec = temp;
    }else{
      //chdir(argv[1]);
      /*
      int i;
      for(i = 0; argv[i] != 0; i++) {
        printf("%d: %s\n", i, argv[i]);
      }
      fprintf(stderr, "buff: %s\n", buff);
      fprintf(stderr, "buff: %p, argv[0]: %p, argv[1]: %p\n", buff, argv[0], argv[1]);

      // free(prev);
      prevDirec = getcwd(buff,MAX_INPUT);
      fprintf(stderr, "buff: %s, prevDirct: %s, argv[1]: %s\n", buff, prevDirec, argv[1]);
      write(1, prevDirec, strlen(prevDirec));
      write(1, "\n", strlen("\n"));
      */
      prevDirec = getcwd(buff,MAX_INPUT);
      //fprintf(stderr,"print argv inside buildin:::::%s:::::::::\n", argv[1]);
      chdir(argv[1]);
    }
    //chdir(argv[1]);
    return 1;
  }
  if(!strcmp(argv[0],"help")){
    //char *cdhelp = "cd [-L|[-P [-e]] [-@]] [dir]            \n";

    fprintf(stderr, "cd [-L|[-P [-e]] [-@]] [dir]            \n");
    fprintf(stderr, "pwd [-LP]           \n");
    fprintf(stderr, "echo [-neE] [arg ...]            \n");
    fprintf(stderr, "set [-abefhkmnptuvxBCHP] [-o option->           \n");
    fprintf(stderr, "help [-dms] [pattern ...]             \n");
    /*
    fprintf(stderr, "cd [-L|[-P [-e]] [-@]] [dir]            \n");
    fprintf(stderr, "cd [-L|[-P [-e]] [-@]] [dir]            \n");
    fprintf(stderr, "cd [-L|[-P [-e]] [-@]] [dir]            \n");
    fprintf(stderr, "cd [-L|[-P [-e]] [-@]] [dir]            \n");
    fprintf(stderr, "cd [-L|[-P [-e]] [-@]] [dir]            \n");
    fprintf(stderr, "cd [-L|[-P [-e]] [-@]] [dir]            \n");
    fprintf(stderr, "cd [-L|[-P [-e]] [-@]] [dir]            \n");
    fprintf(stderr, "cd [-L|[-P [-e]] [-@]] [dir]            \n");
    fprintf(stderr, "cd [-L|[-P [-e]] [-@]] [dir]            \n");
    */


    return 1;
  }

  /*need to solve the setenv scope problem*/

  if(!strcmp(argv[0],"set")){

    //char** envp = environ;
    //extern char** environ;

    //write(1, argv[1], strlen(argv[1]));/*path*/
    //write(1, argv[2], strlen(argv[2]));/*=*/
    //write(1, argv[3], strlen(argv[3]));/*/bin:*/

    int a = setenv(argv[1],argv[3],1);
    if(a!=0){
      fprintf(stderr, "set not successful");
    }
    /*
    char** env;
    for (env = environ; *env != 0; env++){
      char* thisEnv = *env;
      printf("%s\n", thisEnv);
    }
*/
  return 1;
  }

  if(!strcmp(argv[0],"echo")){
    char* firstChar = argv[1];

    char first = *firstChar;
    firstChar++;
    if(first == '$'){
      //write(1, "$", 1);
      //write(1, firstChar, strlen(firstChar));
      if(getenv(firstChar)!=NULL){
      write(1, getenv(firstChar), strlen(getenv(firstChar)));
      write(1, "\n", 1);
    }
    }else{
      write(1, "Not Valid", strlen("Not Valid"));
    }




    return 1;

  }

  

  if(!strcmp(argv[0],"&")){



    return 1;
  }


  /*insert more build in funcyions heere*/


  //char zero[] = "0";
  //setenv("?", zero, 1 );

  return 0;
}






/*make all mutiple space to exactly one, and store the new buffer in char* modify*/
void removeMutipleSpace(char* cmdBuffer, char modify[]){
  int counter = 0;
  while(*cmdBuffer && (*cmdBuffer==' ')){
    cmdBuffer++;
  }
//  printf("%s \n", cmdBuffer);
  while(*cmdBuffer!='\0'){
    while(*cmdBuffer == ' '&& *(cmdBuffer+1)==' '){
      cmdBuffer++;
    }
    modify[counter]= *cmdBuffer;
    counter++;
    cmdBuffer++;
  }
  modify[counter]='\0';
}

int doesFileExist(const char *filename){
  struct stat status;
  int result = stat(filename, &status);
  return (result == 0);

}

int containSlash(char* filename){
  char* ptr = filename;
  size_t i=0;
  int result =0;
  for(i=0;i<strlen(filename);i++){
    char current  = *ptr;
    if(current =='/'){
      result =1;
      break;
    }
    ptr++;
  }
  return result;
}

char* removeNewline(char* cmd){
  char* buffer = cmd;
  size_t length = strlen(buffer);
  *(buffer + length -1)=' ';
  return buffer;

}
char* removeNewlineSp(char* cmd){
  char* buffer = cmd;
  size_t length = strlen(buffer);
  for(size_t i=0; i<length;i++){
    if(*(buffer+i)=='\n'){
      *(buffer+i) = ' ';
    }
  }
  return buffer;

}

int checkIsEmpty (char* cmdBuffer){
  size_t i = 0;
  int result = 0;
  for(i=0;i<strlen(cmdBuffer);i++){
    char temp = *(cmdBuffer+i);
    //fprintf(stderr, "temp:%d",temp);
    //fprintf(stderr, "temp:%d",'A');
    if(temp !=10 && temp !=11 && temp != 32&& temp!='\r'){
      result =1;
      break;
    }
  }
  return result;

}

void push(node_t * head, char* test) {
    node_t * current = head;
    while (current->next != NULL) {
        current = current->next;
    }

    /* now we can add a new variable */
    current->next = (node_t*)malloc(sizeof(node_t));
    current->next->cmd = test;
    current->next->next = NULL;

    current->next->prev = current;

}

void writeHistory(FILE* fp, node_t* head){
  node_t * current = head;

  while (current != NULL) {
      fputs(current->cmd,fp);
      current = current->next;
      //fputs(current,fp);ss
  }
}

void print_list(node_t * head) {
    node_t * current = head;

    while (current != NULL) {
        fprintf(stderr,"%s\n", current->cmd);
        current = current->next;
    }
}

void print_listBackward(node_t * tail) {
    node_t * current = tail;

    while (current != NULL) {
        fprintf(stderr,"%s\n", current->cmd);
        current = current->prev;
    }
}


node_t* getprev(node_t* current){
  //fprintf(stderr, "check pt3\n" );
  //fprintf(stderr, "%p\n", current);
  node_t *result = current->prev;
  //fprintf(stderr, "check pt4\n" );
  if(result == NULL){
    return current;
  }else{
    return result;
  }
}

node_t* getNext(node_t* current){
  node_t * result = current -> next;
  if(result == NULL){
    return current;

  }else{
    return result;
  }
}

void removeNewLineSpace(char* cmdBuffer, char modify[]){
  int counter = 0;
  while(*cmdBuffer && (*cmdBuffer==' ')){
    cmdBuffer++;
  }
//  printf("%s \n", cmdBuffer);
  while(*cmdBuffer!='\0'){
    while(*cmdBuffer == ' '&& *(cmdBuffer+1)==' '){
      cmdBuffer++;
    }
    modify[counter]= *cmdBuffer;
    counter++;
    cmdBuffer++;
  }
  modify[counter]=' ';
}

void moveCursorBackN(int N){
    char cmd[]  = "tput cub1 ";
    int i=0;
    for(i =0;i<N;i++){
    evaluate(cmd);
  }
}

void moveCursorForwardN(int N){
    char cmd[]  = "tput cuf1 ";
    int i=0;
    for(i =0;i<N;i++){
    evaluate(cmd);
  }
}

void eraseAfterCursor(){
  char cmd[] = "tput el ";
  evaluate(cmd);
}

/*return 0 is it's not a arrow key, if it is a arrow key return 65,66,67, 68 which represents for the up dowan left and right*/
int checklastthree(char* cursor){
  char last_char = *cursor;
  char second_char  = *(cursor-1);
  char first_char = *(cursor-2);
  if(first_char == 27 && second_char == 91){
    return (int)last_char;
  }else{
    return 0;
  }
}

void writeToCmd(char* cursor, char target[]){

  size_t i=0;
  memset(target, '\0', 1024);
  for(i=0;i<strlen(cursor)-1;i++){
    char currentChar = *(cursor+i);

    target[i] = currentChar;
  }
  //fprintf(stderr, "1231231target%s\n",target );
}

node_t * returnLastNode(node_t* node_t){
  while(node_t->next!=NULL){
    node_t = node_t -> next;
  }
  return node_t;
}



char Stack_Top(Stack *S)
{
    if (S->size == 0) {
        fprintf(stderr, "Error: stack empty\n");
        return -1;
    }

    return S->data[S->size-1];
}

void Stack_Push(Stack *S, char d)
{
    if (S->size < 1024)
        S->data[S->size++] = d;
    else
        fprintf(stderr, "Error: stack full\n");
}

void Stack_Pop(Stack *S)
{
    if (S->size == 0)
        fprintf(stderr, "Error: stack empty\n");
    else
        S->size--;
}

int findInBracket(char* argv[]){
  int i=0;
  //const char *in = "<";
  for(i=0;i<(MAX_ARGUMENT-1);i++){
    if(argv[i]!=NULL){
    if(!strcmp(argv[i],">")){
      return i;
      //break;
    }
  }else{
    return -1;
    break;
  }
}
return -2;
}

int* spliteCmdByRedirec(char* argv[]){
  int * splitPoint = (int*) malloc(MAX_ARGUMENT);
  int i=0;
  int j = 0;
  //const char *in = "<";
  for(i=0;i<(MAX_ARGUMENT-1);i++){
    if(argv[i]!=NULL){
    if(!strcmp(argv[i],">")){
      *(splitPoint+j)=i ;
      j++;
    }
  }else{
    break;
  }
}
return splitPoint;

}

/*
void mutiplePipEvaluate(int* splitedArray, char *argv[MAX_ARGUMENT]){
  pid_t status;
  int i, err;
  int new_fd[2], old_fd[2];
  pid_t pid, childpid;
  if ( (childpid = fork()) == -1)
    {
        fprintf(stderr, "fail to create child process");
        exit(1);
    }


  size_t numberOfCmd = (sizeof(splitedArray)/sizeof(splitedArray[0]))+1;

  //int numberOfCmdRemining = numberOfCmd;
  char* current[numberOfCmd][MAX_ARGUMENT] ;
  for(size_t i=0;i<(sizeof(current)/sizeof(current[0]));i++){
    for(size_t j=0;j<(sizeof(current[0])/sizeof(current[0][0]));j++){
      current[i][j]=NULL;
    }
  }
  int i=0;
  int j=0;
  int k=0;
  int z=0;
  while(argv[i]!=NULL){
    char* ptr = argv[i];
    current[j][k] = ptr;
    k++;
    i++;
    if(i==*(splitedArray+z)){
      j++;
      k=0;
      z++;
    }

  }


  for(size_t i=0;i<(sizeof(current)/sizeof(current[0]));i++){
    for(size_t j=0;j<(sizeof(current[0])/sizeof(current[0][0]));j++){
      if(current[i][j]!=NULL){
      fprintf(stderr,"importing :%s\n", current[i][j]);
        }
    }
    fprintf(stderr,"--------------------------------\n");
  }


  fprintf(stderr,"curremt [0] %p\n",*current[0]);
  fprintf(stderr,"curremt [1] %p\n",current[1]);
  fprintf(stderr,"curremt [2] %p\n",current[2]);


















}
*/
/*
int* getSmallestQuote(char *argv[]){
  //fprintf(stderr,"enter small quote");
  int i=0;
  const char *quote = "\"";

  for(i=0;i<(MAX_ARGUMENT-1);i++){
    //fprintf(stderr,"typeof %s",argv[i]);
    if(argv[i]!=NULL){
    if(strstr(argv[i],quote)!=NULL){
      int j =0;
      for(j=0;j<strlen(argv[i]);i++){
        int count=0;
        char current = *(argv[i]+j);
        fprintf(stderr,"current char  = %c",current);
        if(current == '\"'){
          count++;
        }
      }
      if(count%2!=0)
    }
  }else{
    break;;
  }
  }
  return -1;
}
*/
