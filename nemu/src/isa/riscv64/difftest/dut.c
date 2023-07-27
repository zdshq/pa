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
#include <cpu/difftest.h>
#include "../local-include/reg.h"

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  if ((memcmp(ref_r->gpr, cpu.gpr, sizeof(word_t) * 33) == 0))
    return true;
  printf("---------------------------dut------------------------------\n");
  for (size_t i = 0; i < 16; i++) {
    printf("%s:%16p\t\t%s:%16p\n", reg_name(i, 64), (void*)cpu.gpr[i], reg_name(i + 16, 64), (void*)cpu.gpr[i + 16]);
  }
  printf("\tpc:%16p\n", (void*)cpu.pc);
  printf("---------------------------ref------------------------------\n");
  for (size_t i = 0; i < 16; i++) {
    printf("%s:%16p\t\t%s:%16p\n", reg_name(i, 64), (void*)ref_r->gpr[i], reg_name(i + 16, 64), (void*)ref_r->gpr[i + 16]);
  }
  printf("\tpc:%16p\n", (void*)ref_r->pc);
  return false;
}

void isa_difftest_attach() {
}
