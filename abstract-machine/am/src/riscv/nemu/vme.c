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
int m = 0;
void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  m++;
  if(m == 2){
      printf("hahahaerwer\t\n");
  }
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  if(m == 2){
      printf("hahahaerwer\t\n");
  }
  memcpy(updir, kas.ptr, PGSIZE);
  if(m == 2){
      printf("hahahaerwer\t\n");
  }
  // int i = 0;
  void *va = as->area.start;
  for (; va < as->area.start + (as->area.end - as->area.start) / 8; va += PGSIZE) {
    // if(m == 2){
    //   i++;
    //   printf("hahahaerwer\t i : %d\n", i);
    // }
    void *pa = pgalloc_usr(1);
    map(as, va, pa, 0);
  }
  set_satp(updir);
}

void unprotect(AddrSpace *as) {
  set_satp(kas.ptr);
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}


void map(AddrSpace *as, void *va, void *pa, int prot) {
  // Calculate the index for the PDE and PTE
  uintptr_t va_num = (uintptr_t)va;
  uintptr_t pa_num = (uintptr_t)pa;
  // va_num = 0x80000000;
  int pde_idx = va_num >> 22;
  int pte_idx = (va_num >> 12) & 0x3FF;

  // Get the PDE and PTE pointers
  pte_t *pde = as->ptr + pde_idx * 4;
  pte_t *pte = as->ptr + (pde_idx << 10) * 4 + pte_idx * 4;

  // Set PTE attributes
  pte->present = 1;
  pte->read = prot & 1;
  pte->write = (prot >> 1) & 1;
  pte->phy = pa_num >> 12;
  // printf("pde : %p\t pte : %p\n", pde, pte);
  // printf("vaddr %p\n", va);
  // Set PDE attributes if not already set
  if (!pde->present) {
    pde->present = 1;
    pde->read = prot & 1;
    pde->write = (prot >> 1) & 1;
    pde->phy = ((uintptr_t)as->ptr + (pde_idx << 10) * 4) >> 12;
    // printf("pde : %p\t pte : %p\n", pde, pte);
    // printf("%p\n", pde->val);
    // printf("%p\n", pte->val);

    // printf("vaddr %p\n", va);
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

