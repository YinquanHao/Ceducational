#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
char* wolfieRequestFactory(char* input);
char* clientToServerMessageRequestFactory(char* input, char* userName);
char* iamRequestFactory(char* userName);
char* MOTDmessageFactory(char* motdMessage);
char* EMITFactory(char* time);
char* MSGTokenlize(char* chatCommand, char* fromUser);
char* decodeMSG(char* MSGString);
int checkPassword(char* password);


int main(int argc, char ** argv){
  char wolfie[] = "WOLFIE";
  char *inputStr = &wolfie[0];
  char *wolfieRequestResult = wolfieRequestFactory(inputStr);
  printf("wolfieRequestResult %s\n",wolfieRequestResult);
  printf("size of wolfieRequestResult %zu\n",strlen(wolfieRequestResult));

  char chat[] = "/chat Yinquan HelloYinquan";
  inputStr = &chat[0];
  char userName[] = "YinquanHao";
  char* user = &userName[0];
  printf("%s",clientToServerMessageRequestFactory(inputStr,user));

  printf("%zu\n",sizeof("WOLFIE "));
  printf("%zu\n",sizeof("WOLFIE \r\n\r\n"));
  printf("%zu\n",sizeof(wolfieRequestFactory(inputStr)));
  printf("%s",iamRequestFactory(user));

  char* fromUser = "YinquanHao";
  char userCommand[] = "/chat Liwen hello, i am Yinquan \r\n\r\n";
  char* chatCommand = &userCommand[0];
  char* generatedMessage = MSGTokenlize(chatCommand,fromUser);
  printf("the generatedMessage = %s",generatedMessage);
  printf("the decodedMessage = %s",decodeMSG(generatedMessage));

  char* myPassword = "Cool320!";
  int valid = checkPassword(myPassword);
  printf("is the password valid ? : %d\n",valid );

  myPassword = "cool320!";
  valid = checkPassword(myPassword);
  printf("is the password valid ? : %d\n",valid );

  myPassword = "Cool320!";
  valid = checkPassword(myPassword);
  printf("is the password valid ? : %d\n",valid );

  myPassword = "coolttt!";
  valid = checkPassword(myPassword);
  printf("is the password valid ? : %d\n",valid );

  myPassword = "Cool320";
  valid = checkPassword(myPassword);
  printf("is the password valid ? : %d\n",valid );

  char passwordField[1024] = {};
  char* password = getpass(passwordField);
    printf("password entered  : %s\n",password );

}

/*this function is used for sending "WOLFIE" */
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

/*this function is used for formatting the /chat toUser Message*/
char* clientToServerMessageRequestFactory(char* input, char* userName){
  strtok(input," ");
  char *token;
  token = strtok(NULL," ");
  char* buffer = (char*)malloc(sizeof buffer);
  strcpy(buffer,"MSG ");
  strcat(buffer,token);
  strcat(buffer," ");
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

char* HImessageFactory(char* userName){
  char *buffer = (char*)malloc(sizeof buffer);
  strcpy(buffer,"HI ");
  strcat(buffer,userName);
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
  strcat(buffer,"\r\n\r\n");
  return buffer;
}



char* decodeMSG(char* MSGString){
  strtok(MSGString," ");
  strtok(NULL," ");
  strtok(NULL," ");
  return strtok(NULL,"\r\n\r\n");
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
