#include <utils.h>

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

int find_str(const char *s1, const char *s2){
  for(int i = 0; i < strlen(s1); i++)
  {
    if (s1[i] == s2[0]){
      if(strncmp(s1+i, s2, strlen(s2)) == 0)
        return 1;
    }
  }
  return 0;
}

