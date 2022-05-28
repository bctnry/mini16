#ifndef __MINI16_TASK_TASK
#define __MINI16_TASK_TASK

#define NR_PROCS 8    // maximum 8 processes
#define NR_REGS 11

typedef enum {
    PROC_READY = 0;
    PROC_RUNNING = 1;
    PROC_WAIT = 2;
} ProcessStatus;

typedef struct Process {
    unsigned short id;
    unsigned short reg[NR_REGS];
    ProcessStatus status;
    struct Process *next;
} Process;

Process[NR_PROCS] proc_list;
Process* proc_ptr;

#endif
