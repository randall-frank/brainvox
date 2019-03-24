/* Copyright 2002 Roger P. Woods, M.D. */
/* Modified 1/16/02 */

#include "AIR.h"
#include <errno.h>

signed char AIR_parse_schar(const char *str, AIR_Error *errcode)

{
    errno=0;
    {
        char *ptr;
        long value=strtol(str, &ptr, 10);
        if(*ptr!='\0'){
            printf("%s: %d: ",__FILE__,__LINE__);
            printf("%s is not a valid signed character\n",str);
            *errcode=AIR_STRING_NOT_SCHAR_ERROR;
            return (signed char)0;
        }
        if(value>(long)SCHAR_MAX || value<(long)SCHAR_MIN){
            printf("%s: %d: ",__FILE__,__LINE__);
            printf("%s is not a valid signed character\n",str);
            *errcode=AIR_STRING_NOT_SCHAR_ERROR;
            return (signed char)0;
        }
        if(errno!=0){
            printf("%s: %d: ",__FILE__,__LINE__);
            printf("%s is not a valid signed character\n",str);
            *errcode=AIR_STRING_NOT_SCHAR_ERROR;
            return (signed char)0;
        }
        *errcode=0;
        return (signed char)value;
    }
}
