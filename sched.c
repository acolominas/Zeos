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

int quantum_total = 0;
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

	tss.esp0 = (unsigned long)&new->stack[KERNEL_STACK_SIZE];

	set_cr3(new->task.dir_pages_baseAddr);

	int ret_value;

	__asm__ __volatile__(
  	"movl %%ebp, %0;"
	: "=g" (ret_value)
	);

	current()->kernel_esp = ret_value;

	__asm__ __volatile__(
	"movl %0, %%esp;"
	:
	: "g" (new->task.kernel_esp)
	);

	__asm__ __volatile__(
  	"popl %ebp\n\t"
  	"ret"	
	);
}

void task_switch(union task_union*new)
{
  
    __asm__ __volatile__("pushl %esi;""pushl %edi;""pushl %ebx");

   	inner_task_switch(new);

   	__asm__ __volatile__("popl %ebx;""popl %edi;""popl %esi");

}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");
    //int pid = sys_getpid();
    //char buffer[4];
    //itoa(pid,buffer);
	while(1)
	{
		sys_write(1,"idle",4);
	}
}

void init_idle (void)
{
	struct list_head *head = list_first(&freequeue);

    list_del(head);

	struct task_struct *idle = list_head_to_task_struct(head);	

	idle->PID = 0;

	allocate_DIR(idle);

	idle_task_union = (union task_union*)idle;

	idle_task_union->stack[KERNEL_STACK_SIZE-1] = (unsigned long)&cpu_idle;

	idle_task_union->stack[KERNEL_STACK_SIZE-2] = 0;

	idle_task_union->task.kernel_esp = (unsigned long)&idle_task_union->stack[KERNEL_STACK_SIZE-2];

	idle_task_union->task.quantum = QUANTUM;
	idle_task_union->task.state = ST_READY;

	idle_task = idle; 
}

void init_task1(void)
{
	struct list_head *head = list_first(&freequeue);

    list_del(head);

	struct task_struct *init = list_head_to_task_struct(head);	

	init->PID = 1;

	allocate_DIR(init);

	set_user_pages(init);

	union task_union *init_task_union = (union task_union*)init;

	init_task_union->task.quantum = QUANTUM;
	init_task_union->task.state = ST_RUN;

	quantum_total = init_task_union->task.quantum;

	tss.esp0 = (unsigned long)&init_task_union->stack[KERNEL_STACK_SIZE];

	set_cr3(init->dir_pages_baseAddr);
}

void init_freequeue()
{
	INIT_LIST_HEAD(&freequeue);
	   
	for (int i = 0; i < NR_TASKS; ++i) {
		list_add_tail(&(task[i].task.list), &freequeue);		
	}
}

void init_readyqueue()
{
	INIT_LIST_HEAD(&readyqueue);
}

int get_quantum(struct task_struct *t) {
	return t->quantum;
}
void set_quantum(struct task_struct *t, int quantum) {
	t->quantum = quantum;
}

void update_sched_data_rr ()
{
	--quantum_total;
}

void sched_next_rr ()
{
	if (!list_empty(&readyqueue)) {
		struct list_head *head = list_first(&readyqueue);
		struct task_struct *next = list_head_to_task_struct(head);
		next->state = ST_RUN;
		quantum_total = get_quantum(next);
		task_switch((union task_union*)next);
		list_del(head);
	}
	else {
		task_switch(idle_task_union);
		idle_task_union->task.state = ST_RUN;
		quantum_total = get_quantum(&idle_task_union->task);
	}
}

int needs_sched_rr (void) 
{ 
//returns: 1 if it is necessary to change the current process and 0
//otherwise
	if ((quantum_total == 0) && (!list_empty(&readyqueue))) return 1;
	if (quantum_total == 0) quantum_total = current()->quantum;
	return 0;
}

void update_process_state_rr (struct task_struct *t, struct list_head *dst_queue)
{
	if (t->state != ST_RUN) list_del(&(t->list));		
	if (dst_queue != NULL)
	{
    	list_add_tail(&(t->list),dst_queue);
    	t->state = ST_READY;
	}
	else t->state = ST_RUN;
		
}

void schedule()
{
	update_sched_data_rr ();

	if (needs_sched_rr()) { 	       

		update_process_state_rr(current(),&readyqueue);			
			
		sched_next_rr();					
						
	}	
	
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

