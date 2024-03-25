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
#include <readline/history.h>
#include<errno.h>
#define PATHMAX 4096
#define MAX 200
#define BLUE "\033[34m"//宏定义实现有色字体
#define GREEN "\033[32m"
#define CLOSE "\033[0m"

int cd =0;
int i_redir=0;
int o_redir=0;
int _pipe=0;
int a_o_redir=0;
int pass=0;//命令解析的参数

void printname(void);
int analyze_cmd(int,char**);
void showhistory();
void mycd(char *argv[]);
void oredir(char *argv[]);
void aoredir(char *argv[]);
void iredir(char *argv[]);
void mymulpipe(char *argv[], int );
void do_cmd(int,char**);
void clear_para();

int main(){
  signal(SIGINT,SIG_IGN);//屏蔽ctrl+c
  signal(SIGTSTP,SIG_IGN); //屏蔽ctrl+z
  while(1){
    char*argv[MAX]={NULL};
    printname();
    char*command=readline(" ");//readline函数输出给出的字符串并读取一行输入，并为读取的输入动态分配内存，返回值为指向读取输入的指针
    if (command == NULL) continue;//屏蔽ctrl+d 
    int argc=1;  
    argv[0] = strtok(command, " ");
    for(int i=1;argv[i] = strtok(NULL, " ");i++) argc++;//将命令行输入分割为多个命令
    analyze_cmd(argc,argv);//解析命令
    do_cmd(argc,argv);//实现命令
    free(command); //释放空间
    clear_para();//重置参数
    }
}

void printname(){
    char pathname[PATHMAX];
    getcwd(pathname,PATHMAX);//获取当前目录
    printf(BLUE"Whosefrienda-shell"CLOSE);//打印shell名称
    printf(GREEN" :%s"CLOSE,pathname);//打印路径
    printf("$ ");
    fflush(stdout);//清除缓冲区
}

int analyze_cmd(int argc,char*argv[]){
    if (argv[0] == NULL) return 0;
    if (strcmp(argv[0], "cd") == 0) cd = 1;
    for (int i = 0; i < argc; i++){
      if (strcmp(argv[i], ">") == 0) o_redir = 1;
      if (strcmp(argv[i], "|") == 0) _pipe = 1;
      if (strcmp(argv[i], ">>") == 0) a_o_redir = 1;
      if (strcmp(argv[i], "<") == 0) i_redir = 1;
      if (strcmp(argv[i], "&") == 0){
        pass = 1;
        argv[i]=NULL;
      }
    }
}

void do_cmd(int argc,char*argv[]){
  if(pass==1) argc--;
  if (cd == 1) mycd(argv);
  else if (strcmp(argv[0], "history") == 0) showhistory();//展示历史命令
  else if (strcmp(argv[0], "exit") == 0)
  {
    printf("exit\n");
    printf("有停止的任务\n");
    exit(0);
  }
  else if ( i_redir== 1) iredir(argv);// < 
  else if ( o_redir== 1) oredir(argv);// >
  else if ( a_o_redir== 1) aoredir(argv);// >>
  else if ( _pipe == 1) mymulpipe(argv, argc);// |  管道放在最后判定，因为重定向中也有管道的判定
  else //需要fork子进程进行执行的命令
  {
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
      execvp(argv[0], argv);
      perror("command");
      exit(1);
    }
    else if (pid > 0) //父进程
    {
      if(pass==1)
      {
        printf("%d\n",pid);
        return;
      }
      waitpid(pid, NULL, 0);
    }
  }
}

void showhistory()
{
  int i = 0;
  HIST_ENTRY **his;
  his = history_list();
  while (his[i] != NULL)
    printf("%-3d   %s\n", i, his[i++]->line);
}

char lastpath[MAX];//为实现cd-而声明
void mycd(char *argv[]){
if (argv[1] == NULL)//未输入要跳转的目录的情况
  {
    getcwd(lastpath, sizeof(lastpath));
    chdir("/home");
  }
  else if (strcmp(argv[1], "-") == 0)//实现cd -
  {
    char newlastpath[MAX];
    getcwd(newlastpath, sizeof(lastpath));
    chdir(lastpath);
    printf("%s\n", lastpath);
    strcpy(lastpath, newlastpath);
  }
  else if (strcmp(argv[1], "~") == 0)//跳转主目录（这里的代码是跳转到我自己的主目录wanggang）
  {
    getcwd(lastpath, sizeof(lastpath));
    chdir("/home/wanggang");
  }
  else
  {
    getcwd(lastpath, sizeof(lastpath));
    chdir(argv[1]);
  }
}

void oredir(char *argv[]){
char *preargv[MAX] = {NULL};
  int i = 0;
  while (strcmp(argv[i], ">"))
  {
    preargv[i] = argv[i];
    i++;
  }
  int preargc=i;//重定向前面参数的个数
  i++;
  //出现 echo "adcbe" > test.c  这种情况
  int fdout = dup(1);//让标准输出获取一个新的文件描述符
  int fd = open(argv[i], O_WRONLY | O_CREAT | O_TRUNC,0666); //只写模式|表示如果指定文件不存在，则创建这个文件|表示截断，如果文件存在，并且以只写、读写方式打开，则将其长度截断为0。
  dup2(fd, 1);
  pid_t pid = fork();
  if (pid == 0) //子进程
  {
    if (_pipe=1) //管道'|'
    {
      mymulpipe(preargv, preargc);
    }
    else
     execvp(preargv[0], preargv);
  }
  else if (pid > 0)//父进程
  {
     if(pass==1)
      {
        printf("%d\n",pid);
        return;
      }
    waitpid(pid, NULL, 0);
  }
  dup2(fdout, 1);//
}

void aoredir(char *argv[]){

  char *preargv[MAX] = {NULL};
  int i = 0;
  
  while (strcmp(argv[i], ">>"))
  {
    preargv[i] = argv[i];
    i++;
  }
  int preargc=i;//重定向前面参数的个数
  i++;
  int fdout = dup(1);//让标准输出获取一个新的文件描述符
  int fd = open(argv[i], O_WRONLY | O_CREAT | O_APPEND,0666); //只写模式|表示如果指定文件不存在，则创建这个文件|表示追加，如果原来文件里面有内容，则这次写入会写在文件的最末尾。
  pid_t pid = fork();
   dup2(fd, 1);
  if (pid == 0) //子进程
  {
    if (_pipe == 1) //管道'|'
      {
        mymulpipe(preargv, preargc);
      }
      else
       execvp(preargv[0], preargv);
  }
  else if (pid > 0)
  {
     if(pass==1)
      {
        printf("%d\n",pid);
        return;
      }
    waitpid(pid, NULL, 0);
  }
  dup2(fdout, 1);
}

void iredir(char *argv[]){
char *preargv[MAX] = {NULL};
  int i = 0;
  while (strcmp(argv[i], "<"))
  {
    preargv[i] = argv[i];
    i++;
  }
  i++;
  int preargc=i;//重定向前面参数的个数
  int fdin = dup(0);//让标准输入获取一个新的文件描述符
  int fd = open(argv[i], O_RDONLY,0666); //只读模式框架
   dup2(fd, 0);
  pid_t pid = fork();
  if (pid == 0) //子进程
  {
     if (_pipe == 1) //管道'|'
      {
        mymulpipe(preargv, preargc);
      }
      else if(o_redir==1)
        oredir(preargv);
      else if(a_o_redir==1)
        aoredir(preargv);
      else
        xecvp(preargv[0], preargv);
  }
  else if (pid > 0)
  {
    waitpid(pid, NULL, 0);
  }
  dup2(fdin, 0);
}

void mymulpipe(char *argv[], int argc ){
pid_t pid;
  int index[10];//存放每个管道的下标
  int number=0;//统计管道个数
  for(int i=0;i<argc;i++)
    if(!strcmp(argv[i],"|")) index[number++]=i;
  int cmd_count=number+1;//命令个数
  char* cmd[cmd_count][10];
  for(int i=0;i<cmd_count;i++)//将命令以管道分割存放组数组里
  {
    if(i==0)//第一个命令
    {
      int n=0;
      for(int j=0;j<index[i];j++)
      {
        cmd[i][n++]=argv[j];
      }
      cmd[i][n]=NULL;
    }
    else if(i==number)//最后一个命令
    {
      int n=0;
      for(int j=index[i-1]+1;j<argc;j++)
      {
        cmd[i][n++]=argv[j];
      }
      cmd[i][n]=NULL;
    }
    else 
    {
      int n=0;
      for(int j=index[i-1]+1;j<index[i];j++)
      {
        cmd[i][n++]=argv[j];
      }
      cmd[i][n]=NULL;
    }
  }//经过上述操作，我们已经将指令以管道为分隔符分好,下面我们就可以创建管道了
  int fd[number][2];  //存放管道的描述符
  for(int i=0;i<number;i++)//循环创建多个管道
  {
    pipe(fd[i]);
  }
  int i=0;
  for(i=0;i<cmd_count;i++)//父进程循环创建多个并列子进程
  {
    pid=fork();
    if(pid==0)//子进程直接退出循环，不参与进程的创建
    break;
  }
  if(pid==0)//子进程
  {
    if(number)
    {
      if(i==0)//第一个子进程
      {
        dup2(fd[0][1],1);//绑定写端`  
        close(fd[0][0]);//关闭读端
        //其他进程读写端全部关闭
        for(int j=1;j<number;j++)
        {
          close(fd[j][1]);
          close(fd[j][0]);
        }
      }
      else if(i==number)//最后一个进程
      {
        dup2(fd[i-1][0],0);//打开读端
        close(fd[i-1][1]);//关闭写端
         //其他进程读写端全部关闭
        for(int j=0;j<number-1;j++)
        {
          close(fd[j][1]);
          close(fd[j][0]);
        }
      }
      else //中间进程
      {
        dup2(fd[i-1][0],0);//前一个管道的读端打开
        close(fd[i-1][1]);//前一个写端关闭
        dup2(fd[i][1],1);//后一个管道的写端打开
        close(fd[i][0]);//后一个读端关闭
        //其他的全部关闭
        for(int j=0;j<number;j++)
        {
           if(j!=i&&j!=(i-1))
           {
             close(fd[j][0]);
             close(fd[j][1]);
           }
        }
      }
    }
 
    execvp(cmd[i][0],cmd[i]);//执行命令
    perror("execvp");
    exit(1);
  }
  //父进程什么都不干，把管道的所有口都关掉
    for(i=0;i<number;i++)
    {
        close(fd[i][0]);
        close(fd[i][1]);//父进程端口全部关掉

    }
     if(pass==1)
      {
        pass=0;
        printf("%d\n",pid);
        return;
      }
  for(int j=0;j<cmd_count;j++)//父进程等待子进程
  wait(NULL);
}
void clear_para(){
cd =0;
i_redir=0;
o_redir=0;
_pipe=0;
a_o_redir=0;
}
