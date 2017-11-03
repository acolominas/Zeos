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

extern int freePID;
extern int zeos_ticks;

int rand(void)
{
    return zeos_ticks;
}

int obtainPID()
{  
  
  int pid_ok = 0;
  int pid;
  while (pid_ok != 1) {
    pid = rand() % 10;
    if (pid == 0) pid += 2;
    else if (pid == 1) pid+=1;

    int i = 2;
    int pid_encontrado = 0;
    while (pid_encontrado == 0 && i < NR_TASKS) {
      if (pid == task[i].task.PID) pid_encontrado = 1;
      ++i; 
    }

    if (i == 10 && pid_encontrado == 0) pid_ok = 1;
  }
  return pid;
}

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

int ret_from_fork() {
  return 0;
}

int sys_fork()
{
  int PID=-1;

  if(list_empty(&freequeue)) return -ENOSPACE;

  struct list_head *head = list_first(&freequeue);

  list_del(head);

  struct task_struct *child_task = list_head_to_task_struct(head);  

  union task_union *child_union = (union task_union*)child_task;

  copy_data(current(),child_union,sizeof(union task_union));
  
  allocate_DIR(child_task);

  page_table_entry *parent_PT = get_PT(current());
  page_table_entry *child_PT = get_PT(child_task);
  

  // Allocate page for DATA+STACK
  int new_frame, pag, i;
  for (pag=0; pag<NUM_PAG_DATA; pag++) 
  {
    new_frame = alloc_frame();
    if (new_frame != -1) {
      set_ss_pag(child_PT, PAG_LOG_INIT_DATA+pag, new_frame);
    }
    else { // no free pages
      for (i=0; i<pag; i++) {
        free_frame(get_frame(child_PT,PAG_LOG_INIT_DATA+i));
        del_ss_pag(child_PT, PAG_LOG_INIT_DATA+i);
      }
      list_add_tail(head, &freequeue);
      return -ENOPAGES;;
    }
  }

  // copy data+stack pages del padre al hijo 
  int INIT_PAGE_DATA = NUM_PAG_KERNEL+NUM_PAG_CODE;
  int END_PAGE_DATA = NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA;


  for (pag=INIT_PAGE_DATA; pag<END_PAGE_DATA; pag++) {
      set_ss_pag(parent_PT, pag+NUM_PAG_DATA, get_frame(child_PT, pag));
      copy_data((void*)((pag)*PAGE_SIZE), (void*)((pag+NUM_PAG_DATA)*PAGE_SIZE), PAGE_SIZE);
      del_ss_pag(parent_PT, pag+NUM_PAG_DATA);
    } 
 
  //assignamos las paginas del codigo del padre al nuevo proceso (no copia)
  for (pag=0; pag<NUM_PAG_CODE; pag++) {
    set_ss_pag(child_PT,PAG_LOG_INIT_CODE+pag,get_frame(parent_PT,PAG_LOG_INIT_CODE+pag));
  }
  // assignamos las paginas del kernel (comun para todos los procs) al nuevo proceso (no copia)
  for (pag=0; pag<NUM_PAG_KERNEL; pag++) {
    set_ss_pag(child_PT,pag,get_frame(parent_PT,pag));
  }
 
   
  //FLUSH DEL TLB
  set_cr3(current()->dir_pages_baseAddr);
   
  //CUSTOMIZAR CONTEXT PROCESO HIJO
  PID = obtainPID();
 
  child_task->PID = PID;  

  child_union->stack[KERNEL_STACK_SIZE-18] = (unsigned int)&ret_from_fork;
  child_union->stack[KERNEL_STACK_SIZE-19] = 0;
  child_task->kernel_esp = (unsigned long)&child_union->stack[KERNEL_STACK_SIZE-19];
  child_task->state = ST_READY;

  list_add_tail(head, &readyqueue);


  //STATISTICS
  init_statistics(child_task);
  
  return PID;
}

void sys_exit()
{  
  struct task_struct *task = current();

  //FREE STRUCTURES
  task->PID = -1;
  list_add_tail(&(task->list), &freequeue);

  //FREE DATA PAGES & PAGE TABLE
  page_table_entry *task_PT = get_PT(task);
  
  int i;
  
  for (i=0; i<NUM_PAG_DATA; i++) {      
      free_frame(get_frame(task_PT,PAG_LOG_INIT_DATA+i));
      //del_ss_pag(task_PT, PAG_LOG_INIT_DATA+i);
  }
  //RUN NEXT PROCESS
  sched_next_rr();
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

int sys_getstats(int pid,struct stats *st)
{
  struct task_struct * task = getStruct(pid);

  if (task != NULL) {
    copy_to_user(&(task->statistics),st,12);
    return 0;
  } 
  else return -ENOPID;
  
}