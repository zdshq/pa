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

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  #ifdef CONFIG_ETRACE_COND
    char buf[100];
    sprintf(buf, "pc:0x%lx\t No;%ld\n", epc, NO);
    log_exec_write("%s",buf);
  #endif
  cpu.csr[mepc] = epc;
  cpu.csr[mcause] = NO;
  cpu.csr[mstatus] |= 0x1800;
  printf("\nmstatus : 0x%lx\tpc : 0x%lx\n",cpu.csr[mstatus], cpu.pc);
  return cpu.csr[mtvec];
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
