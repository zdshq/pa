#include <proc.h>
#include <elf.h>
#include <stdio.h>
#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

size_t ramdisk_read(void *buf, size_t offset, size_t len);

void show_ehdr(Elf64_Ehdr e){
  printf("--------------------------ehdr info-------------------------\n");
  printf("e_phoff : %ld\n",e.e_phoff);
  printf("e_phnum : %d\n",e.e_phnum);
  printf("--------------------------ehdr end--------------------------\n");
}

static uintptr_t loader(PCB *pcb, const char *filename) {
  printf("1111\n");
  Elf64_Ehdr ehdr;
  ramdisk_read(&ehdr, 0, sizeof(Elf64_Ehdr));
  show_ehdr(ehdr);
  printf("\n%s\n",ehdr.e_ident);
  return 0;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  // Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

