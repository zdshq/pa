#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  int i;
  for(i = 0; s[i] != '\0'; i++);
  return i;
}

char *strcpy(char *dst, const char *src) {
  int i;
  for(i = 0; src[i] != '\0'; i++){
    dst[i] = src[i];
  }
  dst[i] = '\0';
  return dst; 
}

char *strncpy(char *dst, const char *src, size_t n) {
  int i;
  for(i = 0; src[i] != '\0' && i < n; i++){
    dst[i] = src[i];
  }
  if(i < n)
  {
    for(;i < n; i++)
      dst[i] = '\0';
  }
  return dst; 
}

char *strcat(char *dst, const char *src) {
  int i,j;
  for(i = 0; dst[i] != '\0'; i++);
  for(j = 0; src[j] != '\0'; j++){
    dst[i+j] = src[j];
  }
  dst[i+j] = '\0';
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  int i,j,len;
  for(i = 0; s1[i] != '\0'; i++);
  for(j = 0; s2[j] != '\0'; j++);
  len = (i >= j) ? j : i;
  for(int m = 0; m < len; m++){
    int q = (s1[m] == s2[m]) ? 0 : ((s1[m] > s2[m]) ? 1 : -1);
    if(q != 0){
      return q;
    }
  }
  return i - j;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  assert(s1);
  assert(s2);
  for(size_t i = 0; i < n && *s1 == *s2 && *s1 != '\0'; i++, s1++, s2++);
  
  if(n == 0 || *s1 == *s2) {
    return 0;
  } else {
    return *(unsigned char *)s1 < *(unsigned char *)s2 ? -1 : 1;
  }
}


void *memset(void *s, int c, size_t n) {
  for(int i = 0; i < n; i++)
  {
    *(char *)(s+i) = (char) c;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  char temp[n];
  char *temp2 = dst;
  for(int i = 0; i < n; i++){
    temp[i] = *(char *)(src+i);
  }
  for(int i = 0; i < n; i++){
    temp2[i] = temp[i];
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  char * temp1 = out;
  for(int i = 0; i < n; i++){
    temp1[i] = *(char *)(in+i);
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  assert(s1);
  assert(s2);
  const unsigned char *p1 = s1;
  const unsigned char *p2 = s2;

  for(size_t i = 0; i < n; i++, p1++, p2++) {
    if (*p1 != *p2) {
      return *p1 - *p2;
    }
  }
  return 0;
}


#endif
