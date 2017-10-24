/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>
#include <libc.h>

/**
 * Container for the Task array and 2 additional pages (the first and the last one)
 * to protect against out of bound accesses.
 */
union task_union protected_tasks[NR_TASKS+2]
  __attribute__((__section__(".data.task")));

union task_union *task = &protected_tasks[1]; /* == union task_union task[NR_TASKS] */

#if 0
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif

extern struct list_head blocked;
struct task_struct *idle_task;

/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

void inner_task_switch(union task_union*new)
{

	tss.esp0 = (unsigned long)&new->stack;

	set_cr3(new->task.dir_pages_baseAddr);

	int ret_value;

	__asm__ __volatile__(
  	"movl %%ebp, %0;"
	: "=g" (ret_value)
	);

	(current())->kernel_esp = ret_value;

	__asm__ __volatile__(
  	"movl %0, %%esp;"
	:
	: "g" (new->task.kernel_esp)
	);

	__asm__ __volatile__(
  	"popl %ebp\n\t"
  	"iret"	
	);
}

void task_switch(union task_union*new)
{
	__asm__ __volatile__(
  	"pushl %esi\n\t"
  	"pushl %edi\n\t"
  	"pushl %ebx"
  );

   inner_task_switch(new);

  __asm__ __volatile__(
  	"popl %ebx\n\t"
  	"popl %edi\n\t"
  	"popl %esi"
  );

}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");

	while(1)
	{
	;
	}
}

void init_idle (void)
{
	struct list_head *head = list_first(&freequeue);

    list_del(head);

	struct task_struct *idle = list_head_to_task_struct(head);

	idle_task_union = (union task_union*)idle;

	idle->PID = 0;

	allocate_DIR(idle);

	//CONTEXT
	idle_task_union->stack[1023] = (unsigned long)&cpu_idle;

	idle_task_union->stack[1022] = 0;

	idle_task_union->task.kernel_esp = (unsigned long)&idle_task_union->stack[1022];
	////////

	idle_task = idle; 
}

void init_task1(void)
{
	struct list_head *head = list_first(&freequeue);

    list_del(head);

	struct task_struct *init = list_head_to_task_struct(head);

	union task_union *init_task_union = (union task_union*)init;

	init->PID = 1;

	allocate_DIR(init);

	set_user_pages(init);

	tss.esp0 = (unsigned long)&init_task_union->stack;

	set_cr3(init->dir_pages_baseAddr);


}

void init_freequeue()
{
	INIT_LIST_HEAD(&freequeue);
    
    struct list_head *head = list_first(&freequeue);

	for (int i=1;i<NR_TASKS;++i) {		

        struct list_head *new = &task[i].task.list;

		list_add(new,head);

		head = new;
	}
}

void init_readyqueue()
{
	INIT_LIST_HEAD(&readyqueue);
}

void init_sched(){
	
	init_freequeue();

	init_readyqueue();

}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

struct task_struct *list_head_to_task_struct(struct list_head *l) {
	return (struct task_struct*)((unsigned int)l&0xfffff000);

}

