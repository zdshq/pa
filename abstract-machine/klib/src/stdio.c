#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <string.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

enum{
    INT_TYPE = 0,
    STR_TYPE,
    LF_TYPE,
    TYPE_END
};

typedef struct t_escape_type
{
    int type;
    char str[4];
}escape_type;


escape_type find_escape(const char *fmt, int *seek){
    escape_type type_table[TYPE_END + 1] = {
        {INT_TYPE, "%d"},
        {STR_TYPE, "%s"},
        {LF_TYPE, "\n"},
        {TYPE_END, ""}
    };
    int i;
    for(i = *seek; fmt[i] != '\0'; i++){
        if(fmt[i] == '%' || fmt[i] == '\n'){
            for(int j = 0; j != TYPE_END; j++){
                int len = strlen(type_table[j].str);
                if(strncmp(fmt + i, type_table[j].str, len) == 0)
                {
                    *seek = i + strlen(type_table[j].str);
                    return type_table[j];
                }
                    
            }
        }
    }
    *seek = i;
    return type_table[TYPE_END];
}

int myitoa(int a, char* out){
    int len = 0, int_temp = a;
    do{
        len++;
        int_temp /= 10;
    }while(int_temp);
    for(int i = 0; i < len; i++){
        out[len - 1 - i] = a % 10  + '0';
        a /= 10;
    }
    return len;
}

int sprintf(char *out, const char *fmt,...)
{
    va_list ap;
    escape_type temp;
    memset(out,0,strlen(out));
    int i;
    int count = 0;
    int index = 0;
    int last_i = 0;
    int int_temp;
    char *str_temp;
    va_start(ap, fmt);
    for (i = 0; fmt[i] != '\0'; ++i) {
        temp = find_escape(fmt, &i);
        if (temp.type == TYPE_END)
        {
            continue;
        }
        switch (temp.type)
        {
        case INT_TYPE:
            int_temp = va_arg(ap, int);
            strncpy(out + index, fmt + last_i, i - (strlen(temp.str)) - last_i);
            index += i - last_i -strlen(temp.str);
            index += myitoa(int_temp, out+index);
            break;
        case STR_TYPE:
            str_temp = va_arg(ap, char *);
            strncpy(out + index, fmt + last_i, i - (strlen(temp.str)) - last_i);
            index += i - last_i -strlen(temp.str);
            strncpy(out+index, str_temp, strlen(str_temp));   
            index += strlen(str_temp);             
            break;    
        case LF_TYPE:
            strncpy(out+index, "\n", 2);
            index += 2;    
            count--; 
            break;   
        default:
            break;
        }
        last_i = i;
        i--;
        count++;
    }
    va_end(ap);
    out[index] = '\0';
    return count;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
