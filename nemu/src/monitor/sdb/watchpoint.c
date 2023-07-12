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

#include "sdb.h"

#define NR_WP 32



static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(){
  WP *temp = free_;
  free_ = free_->next;
  temp->next = head;
  head = temp;
  assert(temp != NULL);
  return temp;
}


void free_wp(int No){
  assert(No < NR_WP);
  WP *P = NULL;
  if (head->NO == No){
    P = head;
    head = head->next;
  }
  else {            
    for(WP *temp = head; temp != NULL; temp = temp->next){
      if(temp->next->NO == No){
        P = temp->next;
        temp->next = temp->next->next;
        break;
      }
    }    
  }
  assert(P);
  P->next = free_;
  free_ = P;
}

void show_watchpoint(){
  printf("NUM \t TYPE\t Enb\t What\r\n");
  for(WP *temp = head; temp != NULL; temp = temp->next){
    printf("%d\t watchpoint\t keep\t y\t %s\r\n", temp->NO, temp->str);
  }
}

