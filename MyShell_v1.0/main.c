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
//test
#define NORMAL 0 //build_in and execvp
#define OUTREDIRECT 1 //with '>'
#define INREDIRECT 2 //with '<'
#define PIPED 3 //with '|'

#define MAXDIRLENGTH 1024
#define MAXCMDLENGTH 256
#define MAXARGLENGTH 128

void init_print()
{
    int maxlen = MAXDIRLENGTH * sizeof(char);
    char* Nowdir = (char*) malloc(maxlen);
    getcwd(Nowdir,maxlen);
    printf("MyShell @ %s $",Nowdir);
    free(Nowdir);
}

int get_input(char* buff)
{
    char* tmp;
    tmp = readline(" ");
    if(strlen(tmp)!=0){
        add_history(tmp);
        strcpy(buff,tmp);
        return 1;
    }
    else{
        return 0;
    }
}

bool endcheck(char* buff)
{
    if(!strcmp(buff,"exit")||!strcmp(buff,"logout")) return true;
    return false;
}

void prasing_space(char* buff, int* argCnt, char argList[MAXARGLENGTH][MAXCMDLENGTH])
{
    int header = 0,tailer = 0;
    int number = 0;
    int len = strlen(buff);
    while(true){
        if(header >= len) break;
        if(buff[header]==' ') header++;
        else{
            tailer = header;
            number = 0;
            while((buff[tailer]!=' ')&&(buff[tailer]!='\n')&&(tailer < len)){
                number++;
                tailer++;
            }
            int iter;
            for(iter=0;iter<=number;iter++){
                if(iter == number) argList[*argCnt][iter]='\0';
                argList[*argCnt][iter] = buff[header+iter];
            }
            *argCnt = *argCnt + 1;
            header = tailer;
        }
    }
    return ;
}

int main(int argc,char** argv)
{
    int buflen = MAXCMDLENGTH * sizeof(char);
    int InputNULLFlag,i;
    int argCnt = 0;
    char argList[MAXARGLENGTH][MAXCMDLENGTH];
    char* buff = (char*)malloc(buflen);
    if(buff == NULL){
        perror("Buffer Malloc Failed ERROR!");
        exit(-1);
    }
    memset(buff,0,buflen);//init_buff

    while(true)
    {
        memset(buff,0,buflen);
        for(argCnt = 0;argCnt<MAXARGLENGTH;argCnt++){
            argList[argCnt][0] = '\0';
        }
        argCnt = 0;
        init_print();

        InputNULLFlag = get_input(buff);
        if(!InputNULLFlag) continue;//while input is NULL continue
        
        if(endcheck(buff)) break;//exit and logout remember include <stdbool.h>
        
        prasing_space(buff,&argCnt,argList);
        //now argCnt shows how many args in the command
        //now argList[i] with %s shows what args is
        //puts(buff);//debug
    }
    return 0;
}