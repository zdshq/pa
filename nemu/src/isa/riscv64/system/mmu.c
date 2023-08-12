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
#include <memory/vaddr.h>
#include <memory/paddr.h>


/**
 * @param type : 读写类型，目前根据kiss原则，先不做区分
*/
int isa_mmu_check(vaddr_t vaddr, int len, int type) {
  // static int a = 0;
  // assert(a == 0);
    uint32_t pdir = (uint32_t)(cpu.csr[4] << 12); // 获得页表基地址
    if(pdir == 0){
      // printf("11\n");
      return MMU_FAIL;
    }
    uint32_t pde_index = vaddr >> 22;
    uint32_t pte_index = vaddr >> 12 & 0x3ff;
    uint32_t pde = paddr_read(pdir + pde_index * 4, 4) & (~0xfff); // 获得一级页表的物理地址
    uint32_t pte = paddr_read(pde + pte_index * 4, 4);
    if((pte & (1 << 2)) != 4)
    {
      printf("22 pde: %x pte: %x vaddr : %lx\n", pde, pte, vaddr);
      assert(0);
      return MMU_FAIL;
    }
    // if()
    // if(vaddr < 0x80000000)
      // printf("vaddr : %lx\n", vaddr);
    if ((pte >> 12) == (vaddr >> 12)) {
      // printf("33\n");
        return MMU_DIRECT;
    }
    else{
      printf("44\n");
      printf("22 pte: %x vaddr : %lx\n", pde, vaddr);
      return MMU_TRANSLATE;
    }
    return 0;
}

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  uint32_t pdir = (uint32_t)(cpu.csr[4] << 12); // 获得页表基地址
  if(pdir == 0){
      // printf("11\n");
    return MMU_FAIL;
  }
  uint32_t pde_index = vaddr >> 22;
  uint32_t pte_index = vaddr >> 12 & 0x3ff;
  uint32_t pde = paddr_read(pdir + pde_index * 4, 4) & (~0xfff); // 获得一级页表的物理地址
  uint32_t pte = paddr_read(pde + pte_index * 4, 4) & (~0xfff);
  printf("translate : %x", pte + (vaddr & 0xfff));
  return pte + (vaddr & 0xfff);
}
