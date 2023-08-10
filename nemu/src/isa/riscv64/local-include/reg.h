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

#ifndef __RISCV64_REG_H__
#define __RISCV64_REG_H__

#include <common.h>

static inline int check_reg_idx(int idx) {
  IFDEF(CONFIG_RT_CHECK, assert(idx >= 0 && idx < 32));
  return idx;
}

// csr 地址映射
static inline int check_csr_idx(int idx) {
  switch (idx) {
  case 0x305: return 0;break; // mtvec
  case 0x341: return 1;break; // mepc
  case 0x300: return 2;break; // mstatus
  case 0x342: return 3;break; // mcause
  case 0x384: return 4;break; // satp
  default:  panic("csr error, addr 0x%x", idx); break;
  }
}

#define csr(idx) (cpu.csr[check_csr_idx(idx)])
#define csrmepc (cpu.csr[mepc])
#define csrmstatus (cpu.csr[mstatus])
#define gpr(idx) (cpu.gpr[check_reg_idx(idx)])

static inline const char* reg_name(int idx, int width) {
  extern const char* regs[];
  return regs[check_reg_idx(idx)];
}

#endif
