  /*

  Jane Yoo
  Project Assignment #3
  Due Thu, March 8

  */

  #include <stdio.h>
  #include <string.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <sys/wait.h>
  #include <stdbool.h>
  #include <sys/types.h>
  #include <sys/stat.h>

  char ** parseArray; 
  int numParseElements=0;
  char** pathArray;
  char * path;
  int x =0; // variable that stores the return value of the exec function
  int pathArrayCounter =0;

  void freeParseMemory() {
      for(int i=0; i<numParseElements; i++) {
        free(parseArray[i]);
      }
      numParseElements=0; 
  }

  void reportError() {
    char error_message[30] = "An error has occured";
    write(STDERR_FILENO, error_message, strlen(error_message));
  }

  void freePathMemory() {
      for(int i=0; i<pathArrayCounter; i++) {
        free(pathArray[i]);
      }
      pathArrayCounter=0; 
  }

  //exit built-in function
  void whoosh_exit() {
    freeParseMemory();
    free(parseArray);
    freePathMemory();
    free(pathArray);
    free(path);
    exit(0);
  }

  //built in whoosh methods
  void whoosh_pwd() {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("Dir: %s\n", cwd);
  }

  //built in whoosh method
  void whoosh_cd(char *args) {
    if(args==NULL) {
      chdir(getenv("HOME"));
    }
    else {
      if(chdir(args)!=0) {
        reportError();
      }
    }   
  }

  /**
  *This method resets the path array and the path counter.It assigns the contents of *the parse memory to the path array and updates the path counter.
  **/
  void whoosh_setpath(char** array) {
    if(array ==NULL) {
     reportError();
     }
    freePathMemory();

    for(int i=0; i<numParseElements && array[i+1]!=NULL; i++) {
      pathArray[i]=strdup(array[i+1]);
      pathArrayCounter++;
    }
  }

  //built in whoosh method
  void whoosh_printpath() {
    if(pathArrayCounter!=0) {
      for(int i=0; i<pathArrayCounter; i++) {
        printf("%s ", pathArray[i]);
      }
      printf("\n");
    }
    else {
      printf("/bin\n");
    }
  }

  /**
  *This method is used to check if the path exists in the current directory
  **/
  bool fileExists(char* filePath) {
    if(filePath==NULL) {
     reportError();
    }
    struct stat pathStat;
    stat(filePath, &pathStat);
    if((stat(filePath, &pathStat) >= 0)) {
      return true;
    }
    return false;
  }

  /**
  *This method tokenizes the command line and sets each token value to a specific *parse array index. It updates the arrays counter. And returns the array.
  **/
  char** whoosh_read(char* commandString){
    if(commandString==NULL){
      reportError();
    }
    char* tokptr;
    int i=0;
   
    parseArray = malloc(100*sizeof(char*));

    tokptr = strtok(commandString, " ");

    while(tokptr){
      parseArray[i]= strdup(tokptr);
      tokptr= strtok(NULL, " ");
      i++;
      numParseElements++;
    }

    parseArray[i]= NULL;
    numParseElements++;
   
    return parseArray;
  }

  bool isBuiltInCommand(char** array){
   if(array==NULL) {
       reportError();
    }
        if((strcmp(array[0],"cd")==0) | (strcmp(array[0],"pwd")==0) | (strcmp(array[0],"exit")==0) | (strcmp(array[0],"setpath")==0)| (strcmp(array[0],"printpath")==0)) {
          return true;
        }
    return false;
  }

  /**
  *This method exceutes the external commands by forking a process. It also runs *executable programs when the user sets the path. 
  **/
  void whoosh_execute(char** array){
    if(array==NULL){
        reportError();
    }
    pid_t pid;
    pid= fork();
    
    if(pid<0) {
      reportError();
    }
    else if(pid == 0) {
       path = malloc(100*sizeof(char*));
    
       strcpy(path, pathArray[0]);
       path = strcat(path, "/");
       path = strcat(path, array[0]);
       
       if(fileExists(path)){
         x = execv(path, array);
       }

       else{
       // else runs when the user sets the path
       //for(int i = 1; i<pathArrayCounter; i++){
        int i=1;
        x = -1;
             // the loop iterates until the path has been found and we are able to execute the program
        while (x<0 && i < pathArrayCounter) {  
            strcpy(path,pathArray[i]); //allocates memory - free it
            path = strcat(path, "/");
            path = strcat(path, array[0]);
            if(fileExists(path)) {
              x = execv(path, array);
              if(x < 0) {
                reportError();
                break;
              }
            }
            i++;
        }
         
        if(x < 0) {
            reportError();
        }
   
        //free all memory of the child process 
        freeParseMemory();
        free(parseArray);
        freePathMemory();
        free(pathArray);
        free(path);
        exit(0);
      }
     }
   
    else {
        wait(NULL);
        return;
    }
  }

  /**
  *This method executes built-in commands and also gives an error if the parameters *are not the correct format.
  **/
  void whoosh_exeOwnCommads(char** array) {
     
      if(array==NULL) {
        reportError();
      }
      if(numParseElements>=4 && (strcmp(array[0], "setpath")!=0)) {
        reportError();
      }
      if(strcmp(array[0], "cd" )==0) {
          whoosh_cd(array[1]);
      
      }
      else if((strcmp(array[0],"exit")==0) ) {   
          if(array[1]==NULL) {
              whoosh_exit();
          }
          else {
              reportError();
          }
      }
      else if(strcmp(array[0], "pwd")==0) {
          whoosh_pwd();
      }
      else if(strcmp(array[0], "setpath")==0) {
          whoosh_setpath(array);
      }
      else {
        whoosh_printpath();
      }
  }

  void printPrompt() {
    printf("%s ", "whoosh>");
  }

  int main(int argc, char** argv) {

    if(argc > 1) {
      reportError();
      exit(0);
    }

    char** argarray;
    char commandLine[128];
    char* c;//stores the pointer returned from strchr

    pathArray = malloc(50*sizeof(char));
    pathArray[0] = "/bin";
    //char commandLine[128]; // through error when beyond 128

    while(1) {
      printPrompt();    
      //scanf("%[^\n]%*c", commandLine);

      if(fgets(commandLine, 128, stdin)!=NULL) {
         c = strchr(commandLine, '\n');
          if(c) {
            *c = '\0';
          }

          if((strlen(commandLine)==0) | (strcmp(&commandLine[0], " ")==0)) {
              continue;
          }
      }
    
      if(sizeof(commandLine)>128) {
        reportError();
        whoosh_exit();
      }

      argarray = whoosh_read(commandLine);
     
      if(isBuiltInCommand(argarray)) {
         whoosh_exeOwnCommads(argarray);
      }
      else {
        whoosh_execute(argarray);
      }
     
     //rest the parse array pointer and its contents
     freeParseMemory();
     free(parseArray);
    }
   
  //free all memory
    free(parseArray);
    freeParseMemory();
    freePathMemory();
    free(pathArray);
    free(path);
    return 0;

  }