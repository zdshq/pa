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

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <memory/vaddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);


static int cmd_si(char *args) {
  long unsigned int temp;
  if(args != NULL)
    assert(sscanf(args,"%lu",&temp) == 1);
  else 
    temp = 1;
  cpu_exec(temp);
  return 0;
}

static int cmd_info(char *args) {
  if(strcmp("r",args) == 0){
    isa_reg_display();
  }
  else if(strcmp("w",args) == 0){
    show_watchpoint();
  }
  return 0;
}

static int cmd_p(char *args){
  bool e;
  printf("%lu\r\n",expr(args,&e));
  return 0;
}

static void show_memery(vaddr_t s, uint32_t len)
{
  uint32_t temp = s + len;
  while(s < temp)
  {
    uint32_t recevie = vaddr_ifetch(s,PMEM_READ_BLOCK_SIZE);
    printf("%lx:%x\r\n", s, recevie);
    s += 4;    
  }
  

}

static int cmd_x(char *args){
  char *a = strtok(args, " ");
  char *str = args + strlen(a) + 1;
  uint32_t len = StrToInt(a, strlen(a));
  bool e;
  uint32_t temp = expr(str,&e);
  show_memery(temp , len);
  return 0;
}

static int cmd_w(char *args){
  bool e;
  WP *Ptemp = new_wp();
  strcpy(Ptemp->str, args);
  uint32_t temp = expr(args, &e);
  Ptemp->old_value = temp;
  return 0;
}

static int cmd_d(char *args){
  bool e;
  uint32_t temp = expr(args, &e);
  free_wp(temp);
  return 0;
}

extern void *vmem;
extern uint8_t pmem[CONFIG_MSIZE];
// void save_quickshot(FILE * fd){
//   printf("1122\r\n");
//   fwrite(pmem, CONFIG_MSIZE/2, 1, fd);
//   printf("1122\r\n");
//   fwrite(vmem, 300*400, 1, fd);
//   fwrite(&cpu, sizeof(CPU_state), 1, fd);
// }

static int cmd_s(char *args){
  char filename[100] = "/home/zsx/ics2022/nemu/quickshot/1";
  // strcat(filename, args);
  FILE *fd = fopen(filename, "w");
  // FILE *fd1 = fopen(filename, "rw");
  assert(fd != NULL);
  printf("1122\r\n");
  fwrite(pmem, CONFIG_MSIZE, 1, fd);
  printf("1122\r\n");
  fwrite(vmem, 300*400, 1, fd);
  fwrite(&cpu, sizeof(CPU_state), 1, fd);
  fclose(fd);
  return 1;
}

static int cmd_l(char *args){
  char filename[100] = "/home/zsx/ics2022/nemu/quickshot/1";
  // strcat(filename, args);
  FILE *fd = fopen(filename, "rb");
  // FILE *fd1 = fopen(filename, "rw");
  assert(fd != NULL);
  printf("1122\r\n");
  printf("123: %ld\n",fread(pmem, 12, 1, fd));
  printf("1122\r\n");
  assert(fread(vmem, 300*400, 1, fd));
  assert(fread(&cpu, sizeof(CPU_state), 1, fd));
  fclose(fd);
  return 1;
}
static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  {"si", "execute N step, default N = 1", cmd_si},
  {"x", "scan expr earn N", cmd_x},
  {"info", "show program status", cmd_info},
  {"p", "caculate the expr", cmd_p},
  {"w", "watch variable", cmd_w},
  {"d", "delete watchpoint", cmd_d},
  {"s", "save quicksort", cmd_s},
  {"l", "load quicksort", cmd_l},

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}


void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }
  char str[] = "(0x1+2+0x3+6+3*(1+3))";
  bool e;
  expr(str,&e);
  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
