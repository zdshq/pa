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

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)
#define PC   cpu.pc
#define Mr vaddr_read
#define Mw vaddr_write
#define IMMI_SIZE 12
#define TO_RD     1

enum {
  TYPE_I, TYPE_U, TYPE_S, TYPE_J, TYPE_B, TYPE_R,
  TYPE_N, // none
};

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
#define immJ() do { *imm = (SEXT(BITS(i, 19, 12), 8) << 12) | (SEXT(BITS(i, 20, 20),1) << 11) \
      | (SEXT(BITS(i, 30, 21),10) << 1) | (SEXT(BITS(i, 31, 30),10) << 20);} while(0)
#define immB() do { *imm = (SEXT(BITS(i, 7, 7), 1) << 11) | (SEXT(BITS(i, 11, 8), 4) << 1) \
      | (SEXT(BITS(i, 30, 25), 6) << 5 | (SEXT(BITS(i, 31, 31), 1) << 12)); } while(0)

#define handle_bne          \
{                           \
  if(src1 != src2)          \
    s->dnpc = PC + imm;     \
}

#define handle_beqz          \
{                           \
  if(src1 == src2)          \
    s->dnpc = PC + imm;     \
}

#define handle_blt          \
{                           \
  if((int32_t)src1 < (int32_t)src2)          \
    s->dnpc = PC + imm;     \
}


#define handle_slti          \
{                           \
  if((int32_t)src1 < (int32_t)imm)          \
    R(rd) = 1;            \
  else                    \
    R(rd) = 0;            \
}

#define handle_sltiu          \
{                           \
  if((uint32_t)src1 < (uint32_t)imm)          \
    R(rd) = 1;            \
  else                    \
    R(rd) = 0;            \
}

#define handle_sra          \
{                           \
  if((int)src1 >=0)          \
    R(rd) = src1 >> src2 ;           \
  else {       \
    int temp = src1 >> src2;         \
    for (int i = src2; i > 0; i--){ \
      temp |= 0x1<<(32-src2);\
    }\
    R(rd) = temp;\
  }                   \
}

#define COM_TO_SOURI(imm, len, to) \
    do { \
        int sign = ((imm) >> ((len) - 1)) & 1; \
        if (sign) { \
            (imm) -= 1; \
            (imm) = ~(imm); \
            for (int i = 31; i > (len) - 1; i--) { \
                (imm) &= (~(1 << i)); \
            } \
            if(to)                \
              R(rd) = (src1) - (imm); \
            else  \
              s->dnpc = (src1) - (imm);\
        } else { \
            if(to)                \
              R(rd) = (src1) + (imm); \
            else  \
              s->dnpc = (src1) + (imm);\
        } \
    } while (0)

int com_to_sour(int imm, int len){
  int sign = (imm >> (len-1)) & 1;
  if(sign)
  {
    imm -= 1;
    imm = ~imm;
    for(int i = 31; i > len-1; i--)
    {
      imm &= (~(1 << i));
    }    
  }
  return imm;
}


static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd     = BITS(i, 11, 7);
  switch (type) {
    case TYPE_I: src1R();          immI(); break;
    case TYPE_U:                   immU(); break;
    case TYPE_S: src1R(); src2R(); immS(); break;
    case TYPE_J:                   immJ(); break;
    case TYPE_B: src1R(); src2R(); immB(); break;
    case TYPE_R: src1R(); src2R();         break;
  }
}

static int decode_exec(Decode *s) {
  int rd = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}

  INSTPAT_START();
  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    , U, R(rd) = SEXT(imm,32));
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(rd) = PC + imm);
  INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal    , J, {R(rd) = PC + 4; s->dnpc = PC + imm;});
  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi   , I, R(rd) = src1+imm);
  INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti   , I, R(rd) = (int32_t)src1 < (int32_t)imm);
  INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu  , I, R(rd) = src1 < imm);
  INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr   , I, R(rd) = s->pc + 4, s->dnpc = (src1 + imm)&((uint32_t)(-1) << 1));
  INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori   , I, R(rd) = src1 ^ imm);
  INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori    , I, R(rd) = src1 | imm);
  INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi   , I, R(rd) = src1 & imm);

  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw     , I, R(rd) = Mr(src1 + imm, 4));
  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw     , S, Mw(src1 + imm, 4, src2));
  INSTPAT("??????? ????? ????? ??? ????? 11001 11", sb     , S, {R(rd) = PC + 4; s->dnpc = src1 + (imm & ~1);});
  INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne    , B, if(src1 != src2){s->dnpc = s->pc + imm;}); 
  INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq    , B, if(src1 == src2){s->dnpc = s->pc + imm;}); 
  INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge    , B, {if( (int32_t)src1 >= (int32_t)src2 ){s->dnpc = s->pc + imm;}}); // need to 
  INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt    , B, if( (int32_t)src1 <  (int32_t)src2 ){s->dnpc = s->pc + imm;}); // need to
  INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu   , B, if(src1 <  src2){s->dnpc = s->pc + imm;});
  INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu   , B, if(src1 >= src2){s->dnpc = s->pc + imm;});

  INSTPAT("0000000 ????? ????? 001 ????? 00100 11", slli   , R, R(rd) = src1<<src2); 
  INSTPAT("0000000 ????? ????? 101 ????? 00100 11", srli   , R, R(rd) = src1>>src2); 
  INSTPAT("0100000 ????? ????? 101 ????? 00100 11", srai   , R, handle_sra);
  INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add    , R, R(rd) = src1+src2); 
  INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem    , R, R(rd) = (int32_t)src1 % (int32_t)src2); 
  INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div    , R, R(rd) = (int32_t)src1/(int32_t)src2); 
  INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub    , R, R(rd) = src1-src2); 
  INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll    , R, R(rd) = src1<<src2); 
  INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt    , R, R(rd) = (int32_t)src1<(int32_t)src2 ? 1 : 0);
  INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu   , R, R(rd) = (uint32_t)src1<(uint32_t)src2 ? 1 : 0);  
  INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl    , R, R(rd) = src1>>src2);
  INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra    , R, handle_sra); 
  INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul    , R, R(rd) = src1*src2); 
  INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor    , R, R(rd) = src1^src2); 

  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
  INSTPAT_END();

  R(0) = 0; // reset $zero to 0
  printf("inst.c:76 : %x\r\n",PC);
  return 0;
}

int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}
