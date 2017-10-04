/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1
#define BUFFER_SIZE 128

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

int sys_fork()
{
  int PID=-1;

  // creates the child process
  
  return PID;
}

void sys_exit()
{  
}

int sys_write(int fd,char *buffer, int size)
{
  int res = 0;
  char buff[BUFFER_SIZE];

  res = (check_fd(fd,ESCRIPTURA));
  if (res < 0) return res;  
  if (buffer == NULL) return -EPNULL;
  if (size < 0) return -EMSGSIZE;
  
  while(size > BUFFER_SIZE){
    copy_from_user(buffer,buff,BUFFER_SIZE);
    res +=sys_write_console(buff,BUFFER_SIZE);
    buffer += BUFFER_SIZE;
    size -= BUFFER_SIZE;
  }

  copy_from_user(buffer, buff, size);
  res += sys_write_console(buff,size);
  
  return res;
}

int sys_gettime()
{
  extern int zeos_ticks;
  return zeos_ticks;
}

