#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  int res = set_sched_policy(1);
  int i = fork();

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
}
