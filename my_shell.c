#include<stdio.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.>
#include<dirent.h>
#include<readline/readline.h>
#include<errno.h>
#define PATHMAX 4096
#define BLUE "\033[34m"
#define CLOSE "\033[0m"
void analyze_cmd(char*);
void do_cmd(int,char**)
int main(){
    singal(SIGHUP,SIG_IGN);
    singal(SIGTIN,SIG_IGN);
    singal(SIGTOU,SIG_IGN);
    singal(SIGINT,SIG_IGN);
    singal(SIGSTP,SIG_IGN);
    while(1){
        char pathname[PATHMAX];
        getcwd(pathname,PATHMAX)
        char*command=readline(BLUE"Whosefrienda-shell"CLOSE);//readline函数输出给出的字符串并读取一行输入，并为读取的输入动态分配内存，返回值为指向读取输入的指针
        if(!command)perror("readline malloc failure");
        analyze_cmd(command);
        do_cmd(argc,argv);
        free(command);
    }
}