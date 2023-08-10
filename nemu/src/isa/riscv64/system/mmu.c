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
    uint32_t pdir = (uint32_t *)(cpu.csr[4] << 12); // 获得页表基地址
    if(pdir == NULL){
      return MMU_FAIL;
    }
    uint32_t pde_index = vaddr >> 22;
    uint32_t pte_index = vaddr >> 12 & 0x3ff;
    uint32_t pde = pdir + ((uintptr_t)vaddr >> 22) * 4; // 获得一级页表的物理地址
    pde = paddr_read(pde, 4);
    uint32_t pte = pde >> 12 + 4 * pte_index;
    printf("pde : %x\n", pde);
    // if((((pte) >> 2))  == NULL){
    //   return MMU_FAIL;
    // }
    // uint32_t *pde = (uint32_t *)((uintptr_t)(*pte) >> 12);
    // if(pde == NULL){
    //   return MMU_FAIL;
    // }
    // if ((*pde >> 12) == ((uintptr_t)vaddr >> 12)) {
    //     return MMU_DIRECT;
    // }
    // else{
    //   return MMU_TRANSLATE;
    // }
    return 0;
}

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
    uint32_t *pdir = (uint32_t *)(cpu.csr[4] << 12); // 获得页表基地址
    if(pdir == NULL){
      return MMU_FAIL;
    }
    uint32_t *pte = pdir + ((uintptr_t)vaddr >> 22) * 4; // 获得一级页表的物理地址
    if(pte == NULL){
      return MMU_FAIL;
    }
    uint32_t *pde = (uint32_t *)((uintptr_t)(*pte) >> 12);
    if(pde == NULL){
      return MMU_FAIL;
    }
    if ((*pde >> 12) == ((uintptr_t)vaddr >> 12)) {
        return MMU_DIRECT;
    }
    else{
      return MMU_TRANSLATE;
    }
    return ((*pde) & (~0xfff)) + (vaddr & 0xfff);
}
