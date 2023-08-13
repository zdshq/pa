#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
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

  context_uload(&pcb[0], "/bin/timer-test", pal_argv, pal_envp);
  context_kload(&pcb[1], hello_fun, NULL);
  Log("wuyu");
  // context_uload(&pcb[1], "/bin/nterm", pal_argv, pal_envp);
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
  current = current == &pcb[0] ? &pcb[1] : &pcb[0];

  // then return the new context
  // printf("pcb[0].cp : %d\n", (int32_t)pcb[0].cp->mepc);
  return current->cp;

}
