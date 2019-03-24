/* Copyright 2001 Roger P. Woods, M.D. */
/* Modified 5/22/01 */

#include "AIR.h"
#include <float.h>
#include <errno.h>

float AIR_parse_float(const char *str, AIR_Error *errcode)

{
    errno=0;
    {
        char *ptr;
        double value=strtod(str, &ptr);
        if(*ptr!='\0'){
            printf("%s: %d: ",__FILE__,__LINE__);
            printf("%s is not a valid float\n",str);
            *errcode=AIR_STRING_NOT_FLOAT_ERROR;
            return 0.0;
        }
        if(fabs(value)>FLT_MAX || (value!=0.0 && fabs(value)<FLT_MIN)){
            printf("%s: %d: ",__FILE__,__LINE__);
            printf("%s is not a valid float\n",str);
            *errcode=AIR_STRING_NOT_FLOAT_ERROR;
            return 0.0;
        }
        if(errno!=0){
            printf("%s: %d: ",__FILE__,__LINE__);
            printf("%s is not a valid float\n",str);
            *errcode=AIR_STRING_NOT_FLOAT_ERROR;
            return 0.0;
        }
        *errcode=0;
        return value;
    }
}
