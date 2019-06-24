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

struct childcmd
{
    int childCnt;
    char childList[MAXARGLENGTH][MAXCMDLENGTH];
    int background;
}child[MAXCMDLENGTH];


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

int split_cmd(int argCnt, char argList[MAXARGLENGTH][MAXCMDLENGTH])
{
    int i;
    int cnt = 0;
    int lastCnt = 0;
    //struct childcmd child[MAXCMDLENGTH];
    char* arg[argCnt+1];
    for(i=0;i<=argCnt;i++){
        if(i==argCnt) arg[i]=NULL;
        else{
            arg[i] = (char*) argList[i];
        }
    }
    
    //for(i=0;i<argCnt;i++) printf("argList[%d]=%s\n",i,arg[i]);
    for(i=0;i<=argCnt;i++){
        if(i == argCnt){
            int j;
            if((i-lastCnt)==0) break;
            child[cnt].childCnt = i - lastCnt;
            child[cnt].background = 0;
            for(j=0;j< i-lastCnt;j++){
                memset(child[cnt].childList[j],'\0',sizeof(child[cnt].childList[j]));
                strcpy(child[cnt].childList[j],arg[lastCnt+j]);
            }
            cnt++;
            break;
        }
        if(strncmp(arg[i],"&",1)==0){
            int j,num;
            num = i - lastCnt;
            if(num==0){
                lastCnt = i+1;
                continue;
            }
            child[cnt].childCnt = num;
            child[cnt].background = 1;
            for(j=0;j<num;j++){
                memset(child[cnt].childList[j],'\0',sizeof(child[cnt].childList[j]));
                strcpy(child[cnt].childList[j],arg[lastCnt+j]);
            }
            lastCnt = i+1;
            cnt++;
            continue;
        }
    }
    return cnt;
    //debug
    // for(i=0;i<cnt;i++){
    //     printf("This is child[%d] with\n",i);
    //     printf("childCnt = %d\n",child[i].childCnt);
    //     for(lastCnt=0;lastCnt<child[i].childCnt;lastCnt++){
    //         printf("childList[%d]=%s\n",lastCnt,child[i].childList[lastCnt]);
    //     }
    //     printf("background=%d\n",child[i].background);
    //     puts("");
    // }

}

void cmd_control(int childCnt,struct childcmd* child)
{
    int i;
    int error=0;
    for(i=0;i<childCnt;i++){
        int argcnt;
        int piped=0;
        int inredir=0;
        int outredir=0;
        int status;
        int fd;
        if(error){
            printf("Wrong Command!\n");
            break;
        }
        int j=0;
        argcnt = child[i].childCnt;
        char* arg[argcnt+1];
        char* argnext[argcnt+1];
        char* file;
        for(j=0;j<argcnt;j++){
            arg[j] = (char*) child[i].childList[j]; 
        }
        arg[argcnt] = NULL;
        for(j=0;arg[j]!=NULL;j++){
            if(error) break;
            if(strcmp(arg[j],">")==0){
                outredir++;
                if(arg[j+1]==NULL) error=1;
                if(outredir > 1) error=1;
            }
            if(strcmp(arg[j],"<")==0){
                inredir = 1;
                if(j==0) error=1;
                if(inredir > 1) error=1;
            }
            if(strcmp(arg[j],"|")==0){
                piped++;
                if(piped > 1) error=1;
                if(j==0) error=1;
                if(arg[j+1]==NULL) error=1;
            }
        }
    }
}

int main(int argc,char** argv)
{
    int buflen = MAXCMDLENGTH * sizeof(char);
    int InputNULLFlag,i;
    int argCnt = 0;
    int childCnt = 0;
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

        childCnt = split_cmd(argCnt,argList);
        //do_cmd(childCnt,child);
        cmd_control(childCnt,child);
    }
    return 0;
}