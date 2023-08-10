#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context* c) {
  //printf("__am_irq_handle code:%d\n", c->mcause);
  if (user_handler) {
    Event ev = { 0 };
    switch (c->mcause) {
      // case  11: ev.event = EVENT_SYSCALL;c->mepc += 4; break; // yield
    case 11:
      if (c->GPR1 == 1) {
        ev.event = EVENT_YIELD;
      }
      else {
        ev.event = EVENT_SYSCALL;
      }
      c->mepc += 4;
      break;

    default: ev.event = EVENT_ERROR; break;
    }

    // for (size_t i = 0; i < 32; i++) {
    //   printf("%d: %x\n", i, c->gpr[i]);
    // }
    c = user_handler(ev, c);
    // printf("hhh: %d", c->mepc);
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

Context* kcontext(Area kstack, void (*entry)(void*), void* arg) {

  printf("kstack.end:%p,kstack.start:%p,size:%d\n", kstack.end, kstack.start, kstack.end - kstack.start);
  Context* p = (Context*)(kstack.end - sizeof(Context));
  memset(p, 0, sizeof(Context));

  printf("Context size:%d\n", (kstack.end - (void*)p));
  assert((kstack.end - (void*)p) == sizeof(Context));

  printf("entry:%p\n", entry);
  p->mepc = (uintptr_t)entry;   // mret 后，进入 entry
  p->gpr[10] = (uintptr_t)arg; // a0 传惨,暂定为一个字符串


  p->mstatus = 0xa00001800; // for difftest

  return p;
}

void yield() {
  asm volatile("li a7, 1; ecall");
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
