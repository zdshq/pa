#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    for(int i = 0 ; i < 32; i += 2){
      printf("%d:%d \t %d:%d \n",i,c->gpr[i],i+1,c->gpr[i+1]);
    }
    Event ev = {0};
    switch (c->mcause) {
      case 0: ev.event = 0 ; break;
      case EVENT_YIELD: ev.event = EVENT_YIELD ; break;
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }

  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));
  // asm volatile("csrw mstatus, %0" : : "r"(0xa00001800));
  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  return NULL;
}

void yield() {
  asm volatile("li a7, 1; ecall");
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
