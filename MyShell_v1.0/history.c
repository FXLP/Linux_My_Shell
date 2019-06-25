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

int main()
{
    char* buff;
    char* path = "/tmp/historyfile";
    while(1){
        puts("init input:");
        buff = readline("");
        if(strcmp(buff,"write")==0){
            puts("read in your cmd and write into historyfile");
            int fp = open(path,O_RDWR|O_CREAT|O_TRUNC,0600);
            while(1){
                puts("input your cmd");
                buff = readline("");
                if(strlen(buff)==0) break;
                printf("your input is %s with len = %d\n",buff,strlen(buff));
                char in[strlen(buff)+1];
                int i;
                for(i=0;i<strlen(buff);i++){
                    in[i] = buff[i];
                }
                in[strlen(buff)]='\n';
                write(fp,in,strlen(buff)+1);

            }
        }
        else if(strcmp(buff,"read")==0){
            printf("reading file in path=%s\n",path);
            FILE* fp = fopen(path,"r");
            char out[100];
            memset(out,'\0',sizeof(out));
            if(fp == 0) puts("fk u!");
            int cnt = 0;
            fscanf(fp,"%s",out);
            while(!feof(fp)){
                printf("%d\t%s\n",cnt++,out);
                fscanf(fp,"%s",out);
            }
            puts("nmdwsm");
        }
        else if(strcmp(buff,"exit")==0){
            break;
        }
        else puts("error!");
    }
    return 0;
}