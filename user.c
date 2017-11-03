#include <libc.h>
#include <stats.h>
char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

     int pid = fork();
     while(1) {         
     	 if (pid == 0) {
	     	write(1,"hijo",4);	     
	     	write(1,"\n",1); 
	     	exit();

	     }
	     else 
	     {
	     	write(1,"     padre",10);
	     	write(1,"\n",1);	     	
	     }
	     

	    
	  }
	  
}
