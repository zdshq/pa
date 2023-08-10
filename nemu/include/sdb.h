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

#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

#define MAX_TOKEN_SIZE 200// token max size
#define PMEM_READ_BLOCK_SIZE 4 // mem read one block size
#define WATCH_STR_SIZE 100
typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char str[WATCH_STR_SIZE];
  uint32_t old_value; 
} WP;

word_t expr(char *e, bool *success);
int64_t StrToInt(char *str, uint32_t len);
WP* new_wp();
void free_wp(int No);
void show_watchpoint();
WP *get_watchpoint_head(void);

void save_quickshot(FILE * fd);

#endif
