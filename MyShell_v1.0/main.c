#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdbool.h>
//history and mouse
#include<readline/readline.h>
#include<readline/history.h>

#define NORMAL 0 //build_in and execvp
#define OUTREDIRECT 1 //with '>'
#define INREDIRECT 2 //with '<'
#define PIPED 3 //with '|'

#define MAXDIRLENGTH 1024
#define MAXCMDLENGTH 128

void init_print()
{
    int maxlen = MAXDIRLENGTH * sizeof(char);
    char* Nowdir = (char*) malloc(maxlen);
    getcwd(NowdirP,maxlen);
    printf("MyShell @ %s $",Nowdir);
    free(Nowdir);
}

int get_input(char* buff)
{
    char* tmp;
    tmp = readline(" ");
    if(strlen(tmp)!=0){
        add_history(tmp);
        int i;int len = strlen(tmp);
        char max[len+1];
        for(i=0;i<len;i++) max[i] = tmp[i];
        max[len] = '\n';
        strcpy(buff,max);
        return 1;
    }
    else{
        return 0;
    }
}

bool endcheck(char* buff)
{
    char* now = NULL;
    int len = strlen(buff);
    len--;
    now = (char*) malloc(len * sizeof(char));
    int i;
    for(i=0;i<=len;i++){
        now[i] = buff[i];
        if(i==len) now[i] = '\0';
    }
    if(!strcmp(now,"exit")||!strcmp(now,"logout")) return true;
    return false;
}

int main(int argc,char** argv)
{
    int buflen = MAXCMDLENGTH * sizeof(char);
    int InputNULLFlag,i;
    int argCnt = 0;
    char argList[MAXCMDLENGTH][MAXCMDLENGTH];
    char** arg = NULL;
    char* buff = (char*)malloc(buflen);
    if(buff == NULL){
        perror("Buffer Malloc Failed ERROR!");
        exit(-1);
    }
    memset(buff,0,buflen);//init_buff

    while(true)
    {
        memset(buff,0,buflen);
        init_print();

        InputNULLFlag = get_input(buff);
        if(!InputNULLFlag) continue;//while input is NULL continue
        
        if(endcheck(buff)) break;//exit and logout remember include <stdbool.h>

    }
    return 0;
}