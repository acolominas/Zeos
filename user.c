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

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  int res = set_sched_policy(1);
  int pid[3];
  char buffer[5];
  int f;
  struct stats p_stats[3];
  int i;
  for(i = 0; i < 3; ++i) {
    int id = fork();
    if (id != 0) pid[i] = id;
    else {
      write(1,"hijo",4);
      f = fibonacci(10000);
      itoa(f,buffer);
      write(1,buffer,5);
      get_stats(pid[i],&p_stats[i]);
      //bucle(10000000000);
      exit();
    }
  }

  /*for(int i = 0; i < 3; ++i) {
    get_stats(pid[i],&p_stats[i]);
  }*/



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

    while(1);
     //struct stats proc2 = p_stats[1];
     //struct stats proc3 = p_stats[2];

 /*
  while(1) {
     if (i == 0) {
       write(1,"Soy HIJO",8);
       write(1,"\n",1);
       int a = read(0," ",50);
     }
     else {
       write(1,"               Soy PADRE",24);
       write(1,"\n",1);
     }

  }
  */
}
