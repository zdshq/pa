#include <am.h>
#include <nemu.h>
#include <klib.h>
#include <assert.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}
pte_t *pde;
void map(AddrSpace *as, void *va, void *pa, int prot) {
  // assert(va == pa);
  pde = as->ptr + 1024*4;
  flex_addr *va1 = va;
  pte_t *pte = va1->ppn1*4 + as->ptr;
  pte->prevent = 1;
  pte->read = prot & 1;
  pte->write = (prot>>1) & 1;
  for(; (uintptr_t)pde < (uintptr_t)as->ptr + 1024*4*1024; pde++){
    if(pde->prevent == 0){
      printf("pde->prevent : %p\n", va);
      pte->phy = (uintptr_t)pde >> 12;
      pde->read = prot & 1;
      pde->write = (prot>>1) & 1;
      pde->prevent = 1;
      pde->phy = (uintptr_t)pa >> 12;
      break;
    }
  }
}

Context* ucontext(AddrSpace* as, Area kstack, void* entry) {

  printf("kstack.end:%p,kstack.start:%p,size:%d\n", kstack.end, kstack.start, kstack.end - kstack.start);

  Context* p = (Context*)(kstack.end - sizeof(Context));

  memset(p, 0, sizeof(Context));

  printf("Context size:%d\n", (kstack.end - (void*)p));
  assert((kstack.end - (void*)p) == sizeof(Context));

  printf("entry:%p\n", entry);


  p->mepc = (uintptr_t)entry;   // mret 后，进入 entry

  p->mstatus = 0xa00001800; // for difftest

  return p;
}

