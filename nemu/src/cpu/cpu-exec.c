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

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>
#include <sdb.h>

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 10

CPU_state cpu = { .csr[mstatus] = 0xa00001800 };
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;
extern t_func_info func_info[50];
extern int64_t func_index;
extern int64_t call_index;

void device_update();

// static void check_watchpoint(){
//   uint32_t new_value;
//   WP *_head = get_watchpoint_head();
//   bool e;
//   for (WP *temp = _head; temp != NULL; temp = temp->next){
//     new_value = expr(temp->str, &e);
//     if(temp->old_value != new_value){
//       nemu_state.state = NEMU_STOP;
//       printf("watchpoint %d : %s\r\n",temp->NO, temp->str);
//       printf("old value = %u\r\n", temp->old_value);
//       printf("New value = %u\r\n", new_value);
//       temp->old_value = new_value;
//     }
//   }
// }



static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
  if (ITRACE_COND) { log_write("%s\n", _this->logbuf); }
#endif
// #ifdef CONFIG_MTRACE_COND
//    log_mem_write("%s\n", _this->logbuf); 
// #endif
#ifdef CONFIG_FTRACE_COND
  // func_log_write(_this->logbuf);
  // if(find_str)
  if (find_str(_this->logbuf, "jal") || find_str(_this->logbuf, "jalr") ){
    if(_this->isa.inst.val == 0x00008067){
      printf("myret\n");
      for(int i = 0; i < func_index; i++){
        if(_this->pc >= func_info[i].start && _this->pc < (func_info[i].start + func_info[i].size)){
          char str[100];
          sprintf(str,"pc:%lx\t%ld:ret func:%s\n", _this->pc, ++call_index, func_info[i].func_name);
          printf("%s", str);
        }
      }
    }
    else{
      for(int i = 0; i < func_index; i++){
        if(_this->dnpc >= func_info[i].start && _this->dnpc < (func_info[i].start + func_info[i].size)){
          char str[100];
          sprintf(str,"pc:%lx\t%ld:call func:%s\n", _this->pc, call_index++, func_info[i].func_name);
          printf("%s", str);
        }
      }         
    }
 
  }

#endif
  if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); }
  // memcpy(_this->ringbuf[_this->count], _this->logbuf, 128);
  // _this->count++;
  // _this->count %= 50;
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));
  // check_watchpoint();
}

static void exec_once(Decode *s, vaddr_t pc) {
  s->pc = pc;
  s->snpc = pc;
  isa_exec_once(s);
  cpu.pc = s->dnpc;
#ifdef CONFIG_ITRACE
  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  int ilen = s->snpc - s->pc;
  int i;
  uint8_t *inst = (uint8_t *)&s->isa.inst.val;
  for (i = ilen - 1; i >= 0; i --) {
    p += snprintf(p, 4, " %02x", inst[i]);
  }
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0) space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;

#ifndef CONFIG_ISA_loongarch32r
  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
      MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst.val, ilen);
#else
  p[0] = '\0'; // the upstream llvm does not support loongarch32r
#endif
#endif
}

// void show(Decode *_this){
//   for(int i = 0; i < 50; i++){
//     if(_this->ringbuf[i][0] != 0){
//       if ((_this->count+50 - 1) % 50 == i){
//         printf("-->");
//       }
//       else 
//         printf("   ");
//       printf("%s\n", _this->ringbuf[i]);      
//     }

//   }
// }

static void execute(uint64_t n) {
  Decode s;
  // for(int i = 0; i < 51; i++)
  //   memset(s.ringbuf[i], 0, 128);
  
  for (;n > 0; n --) {
    exec_once(&s, cpu.pc);
    g_nr_guest_inst ++;
    trace_and_difftest(&s, cpu.pc);
    if (nemu_state.state != NEMU_RUNNING) 
    {
      // if (nemu_state.halt_ret != 0)
        // show(&s);
      break;
    }
    IFDEF(CONFIG_DEVICE, device_update());
    word_t intr = isa_query_intr();
    if (intr != INTR_EMPTY) {
      // Log("wuyuwuyu cpu.pc\n");

      cpu.pc = isa_raise_intr(intr, cpu.pc);
      // Log("cpu.pc  %lx\n", cpu.pc);
    }
  }
}

static void statistic() {
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void assert_fail_msg() {
  isa_reg_display();
  statistic();
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INST_TO_PRINT);
  switch (nemu_state.state) {
    case NEMU_END: case NEMU_ABORT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time();

  execute(n);

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (nemu_state.state) {
    case NEMU_RUNNING: nemu_state.state = NEMU_STOP; break;

    case NEMU_END: case NEMU_ABORT:
      Log("nemu: %s at pc = " FMT_WORD,
          (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
            ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          nemu_state.halt_pc);
      // fall through
    case NEMU_QUIT: statistic();
  }
}
