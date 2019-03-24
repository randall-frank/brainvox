/* Copyright 2001-2002 Roger P. Woods, M.D. */
/* Modified 1/16/02 */

#include "AIR.h"
#include <errno.h>

unsigned long int AIR_parse_ulong(const char *str, AIR_Error *errcode)

{
    errno=0;
    {
        char *ptr;
        unsigned long int value=strtoul(str, &ptr, 10);
        if(*ptr!='\0'){
            printf("%s: %d: ",__FILE__,__LINE__);
            printf("%s is not a valid unsigned long integer\n",str);
            *errcode=AIR_STRING_NOT_ULONG_ERROR;
            return 0;
        }
        if(errno!=0){
            printf("%s: %d: ",__FILE__,__LINE__);
            printf("%s is not a valid unsigned long integer\n",str);
            *errcode=AIR_STRING_NOT_ULONG_ERROR;
            return 0;
        }
        *errcode=0;
        return value;
    }
}
