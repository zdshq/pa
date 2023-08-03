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
  printf("e_phoff : %d\n",(uint32_t)e.e_phoff);
  printf("e_phnum : %d\n",e.e_phnum);
  printf("e_entry : %d\n",*(uint32_t *)(&e.e_entry+1));
  printf("--------------------------ehdr end--------------------------\n");
}

void show_phdr(Elf64_Phdr p){
  printf("--------------------------phdr info-------------------------\n");
  printf("p_type : %d\t",(uint32_t)p.p_type);
  printf("p_offset : %d\t",(uint32_t)p.p_offset);
  printf("p_vaddr : %d\t",(uint32_t)p.p_vaddr);
  printf("p_paddr : %d\n",(uint32_t)p.p_paddr);
  printf("p_filesz : %d\t",(uint32_t)p.p_filesz);
  printf("p_memsz : %d\t",(uint32_t)p.p_memsz);
  printf("p_flags : %d\t",(uint32_t)p.p_flags);
  printf("p_align : %d\n",(uint32_t)p.p_align);
  printf("--------------------------phdr end--------------------------\n");
}

static uintptr_t loader(PCB *pcb, const char *filename) {
  printf("1111\n");
  Elf64_Ehdr ehdr;
  Elf64_Phdr phdr;
  ramdisk_read(&ehdr, 0, sizeof(Elf64_Ehdr));
  assert(*(uint64_t *)ehdr.e_ident == 0x10102464c457f);
  show_ehdr(ehdr);
  for(u_int8_t i = 0; i < ehdr.e_phnum; i++){
    ramdisk_read(&phdr, ehdr.e_phoff + i * sizeof(Elf64_Phdr), sizeof(Elf64_Phdr));
    show_phdr(phdr);    
  }
  printf("\n%s\n",ehdr.e_ident);
  return 0;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  // Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

