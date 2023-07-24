/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#include <elf.h>
#include <isa.h>
extern uint64_t g_nr_guest_inst;
FILE *log_fp = NULL;
FILE *mem_fp = NULL;
FILE *func_fp = NULL;
extern t_func_info func_info[100];
extern int64_t func_index;
void init_log(const char *log_file) {
  log_fp = stdout;
  if (log_file != NULL) {
    FILE *fp = fopen(log_file, "w");
    Assert(fp, "Can not open '%s'", log_file);
    log_fp = fp;
  }
  Log("Log is written to %s", log_file ? log_file : "stdout");
}

void init_mem_log(const char *mem_file) {
  mem_fp = stdout;
  if (mem_file != NULL) {
    FILE *fp = fopen(mem_file, "w");
    Assert(fp, "Can not open '%s'", mem_file);
    mem_fp = fp;
  }
  Log("Memlog is written to %s", mem_file ? mem_file : "stdout");
}

void init_elf(const char *elf_file, const char *func_file){
    if(elf_file == NULL && func_file == NULL)
      return;
    FILE* fp = fopen(elf_file, "rb");
    FILE* fp1 = fopen(func_file, "rb");
    Elf64_Ehdr ehdr;
    assert(fread(&ehdr, sizeof(Elf64_Ehdr), 1, fp) == 1);
    Elf64_Shdr shdr[ehdr.e_shnum];
    fseek(fp, ehdr.e_shoff, SEEK_SET);
    assert(fread(shdr, sizeof(Elf64_Shdr), ehdr.e_shnum, fp) == ehdr.e_shnum);
    Elf64_Sym sym[1000];
    char buffer[1024*4];
    memset(sym, 0x7f, sizeof(sym));
    // int count = 0;;
    for(int i = 0; i < ehdr.e_shnum; i++){
        if(shdr[i].sh_type == SHT_SYMTAB){
            fseek(fp, shdr[i].sh_offset, SEEK_SET);
            assert(fread(sym, sizeof(Elf64_Sym), (shdr[i+1].sh_offset - shdr[i].sh_offset) / sizeof(Elf64_Sym), fp) == (shdr[i+1].sh_offset - shdr[i].sh_offset) / sizeof(Elf64_Sym));
            fseek(fp, shdr[i+1].sh_offset, SEEK_SET);
            assert(fread(buffer, sizeof(char), shdr[i+1].sh_size, fp) == shdr[i+1].sh_size);
        }
    }
    for(int i = 0; sym[i].st_info < 127; i++){
        if(ELF64_ST_TYPE(sym[i].st_info) == STT_FUNC){
          strcpy(func_info[func_index].func_name, buffer+sym[i].st_name);
          func_info[func_index].start = sym[i].st_value;
          func_info[func_index++].size = sym[i].st_size;
        }
    }
    func_fp = fp1;
    Log("Elflog is written to %s", func_file);
}

bool log_enable() {
  return MUXDEF(CONFIG_TRACE, (g_nr_guest_inst >= CONFIG_TRACE_START) &&
         (g_nr_guest_inst <= CONFIG_TRACE_END), false);
}
