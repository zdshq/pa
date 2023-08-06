#include <am.h>
#include <loongarch/loongarch32r.h>
#include <klib.h>
#include <assert.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
    uintptr_t ecode = 0;
    switch (ecode) {
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
  asm volatile("csrwr %0, 0xc" : : "r"(__am_asm_trap));  // 0xc = eentry

  // register event handler
  user_handler = handler;

  return true;
}

Context* kcontext(Area kstack, void (*entry)(void*), void* arg) {   // 初始化

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
  asm volatile("li.w $a7, -1; syscall 0");
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
