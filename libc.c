/*
 * libc.c 
 */

#include <libc.h>

#include <types.h>

#include <errno.h>

int errno;

void itoa(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

int strlen(char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}

int write(int fd,char *buffer, int size)
{
  int res;
  __asm__ __volatile__ ("int $0x80;": "=a" (res): "a" (4), "b" (fd),"c" (buffer), "d" (size));
  if(res < 0){
      errno = -res;
      return -1;
  }
  return res;
}

int gettime()
{  
  int res = -1;
  __asm__ __volatile__ ("int $0x80;":"=a" (res): "a" (10));
  if(res < 0){
      errno = -res;
      return -1;
  }
  return res;
}

int getpid(){
  int res = -1;
  __asm__ __volatile__ ("int $0x80;":"=a" (res): "a" (20));
  if(res < 0){
      errno = -res;
      return -1;
  }
  return res;

}

int fork()
{
  int res = -1;
  __asm__ __volatile__ ("int $0x80;":"=a" (res): "a" (2));
  if(res < 0){
      errno = -res;
      return -1;
  }
  return res;
}

void exit(void)
{
   __asm__ __volatile__ ("int $0x80;":: "a" (1));
 
}

int get_stats(int pid, struct stats *st)
{
  int res = -1;
  __asm__ __volatile__ ("int $0x80;":"=a" (res): "a" (35),"b" (pid),"c"(st));
  if(res < 0){
      errno = -res;
      return -1;
  }
  return res;

}
void perror(char *s)
{
  
  write(1,s,strlen(s));

  switch(errno){
    case ENOSYS:
         write(1,"Function not implemented",24);
         break;
    case EPNULL:
        write(1,"Pointer is null",15);
        break;
    case EMSGSIZE:
        write(1,"Message too long",16);
        break;
    case EACCES:
        write(1,"Permission denied",17);
        break;
    case EBADF:
        write(1,"Bad file number",15);
        break;
    case ENOSPACE:
        write(1,"No space for a new process",26);
        break;
    case ENOPAGES:
        write(1,"There are no enough free pages",29);
        break;
    case ENOPID:
        write(1,"No such process",15);
        break;
  } 
}


