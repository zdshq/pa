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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

#include "sdb.h"
#include <memory/vaddr.h>

enum {
  TK_NOTYPE = 256,

  /* TODO: Add more token types */
  TK_EQ, 
  TK_NUMBER,
  TK_NEG,
  TK_REGS,
  TK_NEQ,
  TK_AND,
  TK_P
};

static struct rule {
  const char *regex;
  uint32_t token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"\\*", '*'},         // multiply
  {"-", '-'},           // minus
  {"([0-9][0-9]*)|(0[xX][0-9a-f]+)", TK_NUMBER},    // number
  {"\\(", '('},         // Left parenthesis
  {"\\)", ')'},          // right parenthesis
  {"\\$((0)|(ra)|(tp)|(sp)|(a[0-7])|(t[0-8])|(rs)|(fp)|(s[0-8])|(pc))",
  TK_REGS},
  {"!=", TK_NEQ},
  {"&&", TK_AND} 
};

static struct token_node
{
  int64_t value;
  uint32_t token_type;
} array[MAX_TOKEN_SIZE];

uint32_t myindex = 0;

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  uint32_t i;
  char error_msg[128];
  uint32_t ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  uint32_t type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static uint32_t nr_token __attribute__((used))  = 0;

uint32_t HextoInt(char c)
{
  if(c >= '0' && c <= '9')
    return (c - '0');
  else                      // handle the digit >= 'a'
    return (c - 'a' + 10);
}

int64_t StrToInt(char *str, uint32_t len)
{
  int64_t num = 0;
  if(*(str+1) == 'x' || *(str+1) == 'X')
  {
    for(int64_t i = 2; i < len; i++)
    {
      num *=16;
      num += HextoInt(str[i]);
    }
  }
  else
  {
    for(int64_t i = 0; i < len; i++)
    {
      num *= 10;
      num += str[i]-'0';
    }    
  }

  return num;
}

static bool make_token(char *e) {
  uint32_t position = 0;
  uint32_t i;
  regmatch_t pmatch;
  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        uint32_t substr_len = pmatch.rm_eo;

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //     i, rules[i].regex, position, substr_len, substr_len, substr_start);


        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_EQ: 
            array[myindex].value=0;
            break;
          case TK_NUMBER: 
            array[myindex].value=StrToInt(substr_start,substr_len);
            break;
          case '*': 
            array[myindex].value=0;
            break;
          case '+':
            array[myindex].value=0;
            break;         
          case TK_NOTYPE:
            break;
          case '(': 
            array[myindex].value=0;
            break;    
          case ')': 
            array[myindex].value=0;
            break;    
          case '-': 
            array[myindex].value=0;
            break; 
          case TK_AND:  
            array[myindex].value=0;
            break;
          case TK_NEQ:
            array[myindex].value = 0;
            break;
          case TK_P:
            array[myindex].value = 0;
            break;
          case TK_REGS:
            bool e;
            char s[3];
            memcpy(s, substr_start+1, 2);
            s[2] = 0;
            array[myindex].value = isa_reg_str2val(s,&e);
            assert(e);
            array[myindex].token_type = TK_NUMBER;
            break;
          default: assert(0);
        }
        position += substr_len;
        if(rules[i].token_type != TK_NOTYPE)
        {
          array[myindex].token_type=rules[i].token_type;
          myindex++;
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

uint32_t find_main_operation(int p,int q){
  int flag = 0;
  int i;
  for(i = q; i >= p; i--){
    if(array[i].token_type == '(')
      flag--;
    else if(array[i].token_type == ')')
      flag++;
    if(flag)
      continue;
    if(array[i].token_type == '+' || array[i].token_type == '-')
      return i;
  }
  for(i = q; i >= p; i--){
    if(array[i].token_type == '(')
      flag--;
    else if(array[i].token_type == ')')
      flag++;
    if(flag)
      continue;
    if(array[i].token_type == '*' || array[i].token_type == '/')
      return i;
  } 
  for(i = q; i >= p; i--){
    if(array[i].token_type == '(')
      flag--;
    else if(array[i].token_type == ')')
      flag++;
    if(flag)
      continue;
    if(array[i].token_type == '*' || array[i].token_type == '/')
      return i;
  }   
  for(i = q; i >= p; i--){
    if(array[i].token_type == '(')
      flag--;
    else if(array[i].token_type == ')')
      flag++;
    if(flag)
      continue;
    if(array[i].token_type == TK_EQ || array[i].token_type == TK_NEQ
    || array[i].token_type == TK_AND)
      return i;
  }

  for(i = p; i <= q; i++){
    if(array[i].token_type == '(')
      flag--;
    else if(array[i].token_type == ')')
      flag++;
    if(flag)
      continue;
    if(array[i].token_type == TK_P || array[i].token_type == TK_NEG)
      return i;
  }   
  assert(0);
}


bool  check_parentheses(int p, int q){
  if(array[p].token_type == '(' && array[q].token_type == ')')
  {
    int flag = 0,flag1 = 0;
    for(int i = q; i >= p; --i){
      if(array[i].token_type == '(')
        flag--;
      else if(array[i].token_type == ')'){
        flag++;
        flag1++;
      }
    }
    if(flag == 0)
      return true;
  }
    
  return false;
}

int64_t eval(int p,int q) {
  int64_t val1,val2,op;
  if (p > q) {
    return 0;
  }
  else if (p == q) {
    return array[p].value;
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1);
  }
  else {
    op = find_main_operation(p ,q);
    val1 = eval(p, op - 1);
    val2 = eval(op + 1, q);
    switch (array[op].token_type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      case TK_EQ: return val1 == val2;
      case TK_P: return vaddr_read(val2, PMEM_READ_BLOCK_SIZE);
      case TK_AND: return val1 && val2;
      case TK_NEQ: return !(val1 == val2);
      case TK_NEG: return -val2;
      default: assert(0);
    }
  }
}

word_t expr(char *e, bool *success) {
  myindex = 0;
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  for (int i = 0; i < myindex; i++) {
    if (array[i].token_type == '*' && (i == 0 || (array[i - 1].token_type != TK_NUMBER 
    && array[i - 1].token_type != ')'))) {
      array[i].token_type = TK_P;
    }
  }
  for (int i = 0; i < myindex; i++){
    if(array[i].token_type == '-' && (i == 0 || (array[i - 1].token_type != TK_NUMBER 
    && array[i - 1].token_type != ')'))){
        array[i].token_type = TK_NEG;
        while (array[++i].token_type == '-'){
          array[i].token_type = TK_NEG;
        }
    }
  }


  /* TODO: Insert codes to evaluate the expression. */
  *success = true;
  return eval(0,myindex-1);
}
