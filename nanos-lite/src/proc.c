#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
uint8_t time[4] = {1,10};
static PCB pcb_boot = {};
PCB *current = NULL;

char* pal_argv[] = {
  NULL
};

char* pal_envp[] = {
  // "home=pwd",
  // "ARCH=riscv",
  // "ARCH=riscv1",
  // "ARCH=riscv2",
NULL
};

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void init_proc() {
  context_kload(&pcb[0], hello_fun, NULL);
  context_uload(&pcb[1], "/bin/nterm", pal_argv, pal_envp);
  Log("wuyu");
  // context_kload(&pcb[0], hello_fun, NULL);
  // printf("pcb[0].cp : %d\n", (int32_t)pcb[0].cp->mepc);
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  // naive_uload(NULL, "/bin/nterm-riscv64");
}

Context*  schedule(Context* prev) {
  // save the context pointer
  current->cp = prev;

  // always select pcb[0] as the new process
  // current = &pcb[0];
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  // current = &pcb[0]
  // then return the new context
  // printf("pcb[0].cp : %d\n", (int32_t)pcb[0].cp->mepc);
  return current->cp;
}
uint8_t task_time = 255,time_index;
Context*  time_schedule(Context* prev) {
  if(task_time == 255){
    time_index = 0;
    task_time = time[0];
    return schedule(prev);
  }
  else if(task_time == 0){
    time_index %= 2;
    task_time = time[time_index];
    return schedule(prev);
  }
  task_time--;
  return prev;
}
