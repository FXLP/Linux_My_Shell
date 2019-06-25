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
	int packflag = 0;
    int header = 0,tailer = 0;
    int number = 0;
    int len = strlen(buff);
    while(true){
        if(header >= len) break;
        if(buff[header]==' ') header++;
        else{
			tailer = header;
            number = 0;
            while(!packflag&&(buff[tailer]!=' ')&&(buff[tailer]!='\n')&&(tailer < len)){
				if(buff[tailer]=='\''){
					packflag=1;
					number++;
					tailer++;
				}
                number++;
                tailer++;
            }
			if(packflag){
				while(buff[tailer]!='\''){
					number++;tailer++;
				}
				packflag = 0;
				number++;
				tailer++;
			}
            int iter;
            for(iter=0;iter<=number;iter++){
                if(iter == number){
                    argList[*argCnt][iter]='\0';
                    break;
                }
                argList[*argCnt][iter] = buff[header+iter];
            }
            *argCnt = *argCnt + 1;
            header = tailer;
        }
    }
	// for(number=0;number<*argCnt;number++){
	// 	printf("argList[%d]=",number);
	// 	puts(argList[number]);
	// }
	// puts(" ");
    return ;
}

void killfun(int sig){
	int status;
	wait(&status);
	return;
}

void do_cmd(int argcount, char arglist[100][256])
{
	int	flag = 0;
	int	how = 0;        /* 用于指示命令中是否含有>、<、|   */
	int	background = 0; /* 标识命令中是否有后台运行标识符& */
	int	status;
	int	i;
	int	fd;
	char*	arg[argcount+1];
	char*	argnext[argcount+1];
	char*	file;
	pid_t	pid;

	/*将命令取出*/
	for (i=0; i < argcount; i++) {
		arg[i] = (char *) arglist[i];
	}
	arg[argcount] = NULL;

	/*查看命令行是否有后台运行符*/
	for (i=0; i < argcount; i++) {
		if (strncmp(arg[i], "&",1) == 0) {
			if (i == argcount-1) {
				background = 1;
				arg[argcount-1] = NULL;
				break;
			}
			else {
				printf("wrong command错误指令\n");
				return ;
			}
		}
	}

	for (i=0; arg[i]!=NULL; i++) {
		if (strcmp(arg[i], ">") == 0 ) {
			flag++;
			how = OUTREDIRECT;
			if (arg[i+1] == NULL)
				flag++;
		}
		if ( strcmp(arg[i],"<") == 0 ) {
			flag++;
			how = INREDIRECT;
			if(i == 0)
				flag++;
		}
		if ( strcmp(arg[i],"|")==0 ) {
			flag++;
			how = PIPED;
			if(arg[i+1] == NULL)
				flag++;
			if(i == 0 )
				flag++;
		}
	}
	/* flag大于1，说明命令中含有多个> ,<,|符号，本程序是不支持这样的命令的
	   或者命令格式不对，如"ls －l /tmp >" */
	if (flag > 1) {
		printf("wrong command\n");
		return;
	}

	if (how == OUTREDIRECT) {  /*命令只含有一个输出重定向符号> */
		for (i=0; arg[i] != NULL; i++) {
			if (strcmp(arg[i],">")==0) {
				file   = arg[i+1];
				arg[i] = NULL;
			}
		}
	}

	if (how == INREDIRECT) {    /*命令只含有一个输入重定向符号< */
		for (i=0; arg[i] != NULL; i++) {
			if (strcmp (arg[i],"<") == 0) {
				file   = arg[i+1];
				arg[i] = NULL;
			}
		}
	}

	if (how == PIPED) {  /* 命令只含有一个管道符号| */
/* 把管道符号后门的部分存入argnext中，管道后面的部分是一个可执行的shell命令 */
		for (i=0; arg[i] != NULL; i++) {
			if (strcmp(arg[i],"|")==0) {
				arg[i] = NULL;
				int j;
				for (j=i+1; arg[j] != NULL; j++) {
					argnext[j-i-1] = arg[j];
				}
				argnext[j-i-1] = arg[j];
				break;
			}
		}
	}

	if ( (pid = fork()) < 0 ) {
		printf("fork error\n");
		return;
	}

	switch(how) {
		case 0:
			/* pid为0说明是子进程，在子进程中执行输入的命令 */
			/* 输入的命令中不含>、<和| */
			if (pid == 0) {
				//printf("debug:\n%s",arg[0]);
				if(strcmp(arg[0],"alias")==0 || strcmp(arg[0],"unalias")==0){
					exit(0);
				}
				if ( !(find_command(arg[0])) ) {
					printf("%s : command not found\n", arg[0]);
					exit(0);
				}
				execvp(arg[0], arg);
				exit(0);
			}
			else//built_in cmd
			{
				//check arg[0] == alias or unalias
				if(strcmp(arg[0],"alias")==0){
				//	puts("alias");
					char cmd[MAXCMDLENGTH];
					char full[MAXCMDLENGTH];
					char aliasbuff[MAXDIRLENGTH];
					memset(aliasbuff,'\0',sizeof(aliasbuff));
					memset(cmd,'\0',sizeof(cmd));
					memset(full,'\0',sizeof(full));
					int cnt;
					for(cnt=0;arg[cnt]!=NULL;cnt++){
						// printf("arg[%d]=",cnt);
						// puts(arg[cnt]);
					}
					if(cnt == 1){//alias
						char* aliaspath = "/tmp/aliasfile";
						FILE* fp = fopen(aliaspath,"r");
						int i;
						// puts("init print");
						// for(i=0;i<strlen(aliasbuff);i++){
						// 	printf("%d ",aliasbuff[i]);
						// }
						// puts("init print end");
						fgets(aliasbuff,MAXDIRLENGTH,fp);
						while(!feof(fp)){
							//printf("%s with len=%d\n",aliasbuff,sizeof(aliasbuff));
							for(i=0;i<sizeof(aliasbuff);i++){
								if(aliasbuff[i]=='\t'){
									aliasbuff[i]='=';
								}
								if(aliasbuff[i]=='\n'){
									aliasbuff[i]='\0';
									break;
								}
							}
							printf("%s\n",aliasbuff);
							fgets(aliasbuff,MAXDIRLENGTH,fp);
						}
						// puts("alias end");
					}
					else if(cnt == 2){//alias cmd
						int i=0,iter;
						int flag = 0;
						int len = strlen(arg[1]);
						for(iter=0;iter<len;iter++){
							if(!flag){
								cmd[i++] = arg[1][iter];
							}
							if(arg[1][iter]=='=')
						}
					}else{
						printf("Wrong Command usage:alias\nPerhaps you should delete useless space front or back of '='\n");
					}
				}
				if(strcmp(arg[0],"unalias")==0){
					puts("unalias");	
				}
				//check arg[0] == history
				if(strcmp(arg[0],"history")==0){
					char* hispath = "/tmp/historyfile";
					FILE* fp = fopen(hispath,"r");
					char out[100];
					memset(out,'\0',sizeof(out));
					int cnt = 0;
					fscanf(fp,"%s",out);
					while(!feof(fp)){
						printf("%d\t%s\n",cnt++,out);
						fscanf(fp,"%s",out);
					}
				}
				
				if(strcmp(arg[0],"kill")==0){
					//printf("debug");
					kill(arg[1],SIGKILL);
					signal(17,killfun);
				}
				if(strcmp(arg[0],"cd")==0){
					chdir(arg[1]);
				}
			}
			break;
		case 1:
			/* 输入的命令中含有输出重定向符> */
			if (pid == 0) {
				if ( !(find_command(arg[0])) ) {
					printf("%s : command not found\n",arg[0]);
					exit(0);
				}
				fd = open(file,O_RDWR|O_CREAT|O_TRUNC,0644);
				dup2(fd,1);
				execvp(arg[0],arg);
				exit(0);
			}
			break;
		case 2:
			/* 输入的命令中含有输入重定向符< */
			if (pid == 0) {
				if ( !(find_command (arg[0])) ) {
					printf("%s : command not found\n",arg[0]);
					exit(0);
				}
				fd = open(file,O_RDONLY);
				dup2(fd,0);
				execvp(arg[0],arg);
				exit(0);
			}
			break;
		case 3:
			/* 输入的命令中含有管道符| */
			if(pid == 0) {
				int  pid2;
				int  status2;
				int  fd2;

				if ( (pid2 = fork()) < 0 ) {
					printf("fork2 error\n");
					return;
				}
				else if (pid2==0) {
					if ( !(find_command(arg[0])) ) {
						printf("%s : command not found\n",arg[0]);
						exit(0);
					}
					fd2 = open("/tmp/youdonotknowfile",
							O_WRONLY|O_CREAT|O_TRUNC,0644);
					dup2(fd2, 1);
					execvp(arg[0], arg);
					exit(0);
				}

				if (waitpid(pid2, &status2, 0) == -1)
					printf("wait for child process error\n");

				if ( !(find_command(argnext[0])) ) {
					printf("%s : command not found\n",argnext[0]);
					exit(0);
				}
				fd2 = open("/tmp/youdonotknowfile",O_RDONLY);
				dup2(fd2,0);
				execvp (argnext[0],argnext);

				if ( remove("/tmp/youdonotknowfile") )
					printf("remove error\n");
				exit(0);
			}
			break;
		default:
			break;
	}

	/* 若命令中有&，表示后台执行，父进程直接返回不等待子进程结束 */
	if ( background == 1 ) {
		printf("[process id %d]\n",pid);
		return ;
	}

	/* 父进程等待子进程结束 */
	if (waitpid (pid, &status,0) == -1)
		printf("wait for child process error\n");
}

/* 查找命令中的可执行程序 */
int find_command (char *command)
{
	DIR*             dp;
	struct dirent*   dirp;
	char*			 path[] = { "./", "/bin", "/usr/bin", NULL};
    int cmdlen = strlen(command);
	/* 使当前目录下的程序可以被运行，如命令"./fork"可以被正确解释和执行 */
    if( strncmp(command,"./",2) == 0 )
		command = command + 2;
	/* 分别在当前目录、/bin和/usr/bin目录查找要可执行程序 */
	int i = 0;
	while (path[i] != NULL) {
		if ( (dp = opendir(path[i]) ) == NULL)
			printf ("can not open /bin \n");
		while ( (dirp = readdir(dp)) != NULL) {
            //printf("Now reading cmd = %s\n",dirp->d_name);
			if (strncmp(dirp->d_name,command,cmdlen) == 0) {
                //printf("NOW path %s\n",path[i]);
				closedir(dp);
				return 1;
			}
		}
		closedir (dp);
		i++;
	}
	return 0;
}

void add_to_my_history(char* buff,int fp){
	int len = strlen(buff);
	int i;
	char in[len+1];
	for(i=0;i<len;i++){
		in[i] = buff[i];
	}
	in[len] = '\n';
	write(fp,in,len+1);
}

void alias(char* buff,int fp){
	// puts(buff);
	// printf("%d\n",fp);
	return;
}

int main(int argc,char** argv)
{
    int buflen = MAXCMDLENGTH * sizeof(char);
    int InputNULLFlag,i;
    int argCnt = 0;
    int childCnt = 0;
    char argList[MAXARGLENGTH][MAXCMDLENGTH];
    char* buff = (char*)malloc(buflen);
	char* history_path = "/tmp/historyfile";
	char* alias_path = "/tmp/aliasfile";
    if(buff == NULL){
        perror("Buffer Malloc Failed ERROR!");
        exit(-1);
    }
    memset(buff,'\0',buflen);//init_buff
	int historyfp = open(history_path,O_RDWR|O_CREAT|O_TRUNC,0644);
	int aliasfp = open(alias_path,O_RDWR|O_CREAT,0644);
    while(true)
    {
        memset(buff,'\0',buflen);
        for(argCnt = 0;argCnt<MAXARGLENGTH;argCnt++){
            argList[argCnt][0] = '\0';
        }
        argCnt = 0;
        init_print();

        InputNULLFlag = get_input(buff);
        if(!InputNULLFlag) continue;//while input is NULL continue

		//add buff into history file
		add_to_my_history(buff,historyfp);
		//check buff => alias file if has replace
		alias(buff,aliasfp);

        if(endcheck(buff)) break;//exit and logout remember include <stdbool.h>
        
        prasing_space(buff,&argCnt,argList);
        
        do_cmd(argCnt,argList);
    }
    if(buff != NULL){
        free(buff);
        buff=NULL;
    }
    return 0;
}
