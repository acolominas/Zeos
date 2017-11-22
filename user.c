#include <libc.h>
#include <p_stats.h>

char buff[24];

int pid;


int fibonacci(int n)
{
  if (n == 0) return 0;
  else if (n == 1) return 1;
  else return (fibonacci(n-1) + fibonacci(n-2));
}

void write_stats(int pid,struct stats proc)
{
  int tam = 5;
  char buff_pid[5];
  char buff[tam];
  write(1,"\n",1);
  itoa(pid,buff_pid);
  write(1,buff_pid,5);
  itoa(proc.user_ticks,buff);
  write(1,buff,tam);
  itoa(proc.system_ticks,buff);
  write(1,buff,tam);
  itoa(proc.blocked_ticks,buff);
  write(1,buff,tam);
  itoa(proc.ready_ticks,buff);
  write(1,buff,tam);
  write(1,"\n",1);

}


int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  //0 (round robin) or 1 (FCFS)
  int res = 0;
  int total = 0;
  int tam = 10000000;
  res = set_sched_policy(1);
  if (res < 0)   write(1,"error",5);
  else {
    char buffer[8];
    int f;
    struct stats p_stats;
    int i;
    for(i = 0; i < 3; ++i) {
      if (fork() == 0){
        for(int k = 0; k < tam; ++k)
          for(int l = 0; l < tam; ++l)
            ++total;
        //f = fibonacci(100);
        //itoa(total,buffer);
        //write(1,buffer,5);
        int pid = getpid();
        get_stats(pid,&p_stats);
        write_stats(pid,p_stats);
        exit();
      }
    }

    /*for(int i = 0; i < 3; ++i) {
      get_stats(pid[i],&p_stats[i]);
    }*/


/*
       int tam = 5;
       char buff_pid[5];
       char buff[tam];
       write(1,"\n",1);
       for(int i = 0; i<3;++i) {
         itoa(pid[i],buff_pid);
         write(1,buff_pid,5);
         struct stats proc = p_stats[i];
         itoa(proc.user_ticks,buff);
         write(1,buff,tam);
         itoa(proc.system_ticks,buff);
         write(1,buff,tam);
         itoa(proc.blocked_ticks,buff);
         write(1,buff,tam);
         itoa(proc.ready_ticks,buff);
         write(1,buff,tam);
         write(1,"\n",1);
      }
      */
    }
    while(1);
}
