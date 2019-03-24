/* Copyright 2001-2002 Roger P. Woods, M.D. */
/* Modified 1/16/02 */

#include "AIR.h"
#include <errno.h>

int AIR_parse_int(const char *str, AIR_Error *errcode)

{
    errno=0;
    {
        char *ptr;
        long value=strtol(str, &ptr, 10);
        if(*ptr!='\0'){
            printf("%s: %d: ",__FILE__,__LINE__);
            printf("%s is not a valid integer\n",str);
            *errcode=AIR_STRING_NOT_INT_ERROR;
            return 0;
        }
        if(value>INT_MAX || value<INT_MIN){
            printf("%s: %d: ",__FILE__,__LINE__);
            printf("%s is not a valid integer\n",str);
            *errcode=AIR_STRING_NOT_INT_ERROR;
            return 0;
        }
        if(errno!=0){
            printf("%s: %d: ",__FILE__,__LINE__);
            printf("%s is not a valid integer\n",str);
            *errcode=AIR_STRING_NOT_INT_ERROR;
            return 0;
        }
        *errcode=0;
        return (int)value;
    }
}
