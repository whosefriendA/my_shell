#include<stdio.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<dirent.h>
#include<readline/readline.h>
#include<errno.h>
#define PATHMAX 4096
#define MAX 200
#define BLUE "\033[34m"
#define GREEN "\033[32m"
#define CLOSE "\033[0m"
    int cd =0;
    int i_redir=0;
    int o_redir=0;
    int _pipe=0;
    int a_i_redir=0;
    int a_o_redir=0;
    int pass=0;

void printname(void);
int analyze_cmd(int,char**);
void do_cmd(int,char**);

int main(int argc,char*argv){
    signal(SIGHUP,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
    signal(SIGTTOU,SIG_IGN);
    signal(SIGINT,SIG_IGN);
    signal(SIGTSTP,SIG_IGN); 
    while(1){
      char*argv[MAX]={NULL};
      printname();
      char*command=readline("");//readline函数输出给出的字符串并读取一行输入，并为读取的输入动态分配内存，返回值为指向读取输入的指针
      if (command == NULL) continue;//屏蔽ctrl+d 
      int argc=1;  
      argv[0] = strtok(command, " ");
      for(int i=1;argv[i] = strtok(NULL, " ");i++) argc++;//将命令行输入分割为多个命令
      analyze_cmd(argc,argv);//解析命令
      do_cmd(argc,argv);//实现命令
      free(command);  
    }
}
void printname(){
    char pathname[PATHMAX];
    getcwd(pathname,PATHMAX);
    printf(BLUE"Whosefrienda-shell"CLOSE);
    printf(GREEN" :%s"CLOSE,pathname);
    printf(" $ ");
    fflush(stdout);
}
int analyze_cmd(int argc,char*argv[]){
    if (argv[0] == NULL) return 0;
    if (strcmp(argv[0], "cd") == 0) cd = 1;
    for (int i = 0; i < argc; i++){
      if (strcmp(argv[i], ">") == 0) o_redir = 1;
      if (strcmp(argv[i], "|") == 0) _pipe = 1;
      if (strcmp(argv[i], ">>") == 0) a_o_redir = 1;
      if (strcmp(argv[i], "<") == 0) i_redir = 1;
      if (strcmp(argv[i], "<<") == 0) a_i_redir = 1;
      if (strcmp(argv[i], "&") == 0){
        pass = 1;
        argv[i]=NULL;
      }
    }
}
void do_cmd(int argc,char*argv[]){
    if(pass==1)
  {
    argc--;
  }
  if (cd == 1)
  {
    mycd(argv);
  }
  else if (strcmp(argv[0], "history") == 0)
  {
    ShowHistory();
  }
  else if (strcmp(argv[0], "exit") == 0)
  {
    printf("exit\n");
    printf("有停止的任务\n");
    exit(0);
  }
  else if ( o_redir== 1) //输出重定向'>'
  {
    mydup(argv);
  }
  else if (pipe == 1) //管道'|'
  {
    callCommandWithPipe(argv, argc);
  }
  else if ( a_o_redir== 1)// >>
  {
    mydup2(argv);
  }
  else if ( i_redir== 1)// <
  {
    mydup3(argv);
  }
  else //需要子进程进行执行的第三方函数
  {
    if (strcmp(argv[0], "ll") == 0)
    {
      strcpy(argv[0], "ls");
      argv[argc++] = "-l";
    }
    if (strcmp(argv[0], "ls") == 0)
      argv[argc++] = "--color=auto";
    pid_t pid = fork();
    if (pid < 0)
    {
      perror("fork");
      exit(1);
    }
    else if (pid == 0) //子进程
    {
      execvp(argv[0], argv); //执行命令
      perror("commod");
      exit(1);
    }
    else if (pid > 0) //父进程
    {
      if(pass==1)
      {
        pass=0;
        printf("%d\n",pid);
        return;
      }
      waitpid(pid, NULL, 0);
    }
  }
}