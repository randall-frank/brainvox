#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <shlobj.h>
#endif

#include "prefs.h"

static int read_prefs = 0;

static int noreaper = 0;
static int irisglmouse = 0;
static float backuprate = 6.667;
static int hwinterp = 0;

static void init_prefs(void)
{
   int wrt = 0;
   if (getenv("BRAINVOX_REAPER")) {
      if (strstr(getenv("BRAINVOX_REAPER"),"NO")) noreaper = 1;
   }
   if (getenv("BRAINVOX_SWAP_BUTTONS"))  {
      if (atoi(getenv("BRAINVOX_SWAP_BUTTONS")) == 0) irisglmouse = 1;
   }
   file_pref_(&wrt);
   read_prefs = 1;
}

void file_pref_(int *wrt)
{
   char tstr[1024] = {0};
   FTag tags[10] = {0};
   long int err,num,update;

#ifndef WIN32
   if (getenv("HOME")) {
      strcat(tstr,getenv("HOME"));
      strcat(tstr,"/");
   }
   strcat(tstr,".brainvox_prefs");
#else
   if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA|CSIDL_FLAG_CREATE,
                    NULL, SHGFP_TYPE_CURRENT, tstr))) {
      strcat(tstr,"\\");
   }
   strcat(tstr,"brainvox_prefs");
#endif

   strcpy(tags[0].name,"F_BV_BACKUPRATE");
   tags[0].data = (void *)(&backuprate);

   strcpy(tags[1].name,"I_BV_USEIRISGLMOUSE");
   tags[1].data = (void *)(&irisglmouse);

   strcpy(tags[2].name,"I_BV_DISABLEREAPER");
   tags[2].data = (void *)(&noreaper);

   strcpy(tags[3].name,"I_BV_USE3DHWINTERP");
   tags[3].data = (void *)(&hwinterp);

   num = 4;
   if (*wrt) {
      update = 1;
      write_tagged_file_(tstr,"Brainvox pres",&update,&num,tags,&err);
   } else {
      read_tagged_file_(tstr,&num,tags,&err);
   }
}
void FILE_PREF(int *wrt)
{
file_pref_(wrt);
}

void get_pref_i_(int *which,int *value)
{
   if (!read_prefs) init_prefs();
   if (*which == PREF_NOREAPER) {
      *value  = noreaper;
      return;
   }
   if (*which == PREF_IRISGLMOUSE) {
      *value  = irisglmouse;
      return;
   }
   if (*which == PREF_3DHWINTERP) {
      *value  = hwinterp;
      return;
   }
   return;
}
void GET_PREF_I(int *which,int *value)
{
get_pref_i_(which,value);
}

void get_pref_f_(int *which,float *value)
{
   if (!read_prefs) init_prefs();
   if (*which == PREF_BACKUPRATE) {
      *value  = backuprate;
      return;
   }
   return;
}
void GET_PREF_F(int *which,float *value)
{
get_pref_f_(which,value);
}
void set_pref_i_(int *which,int *value)
{
   if (!read_prefs) init_prefs();
   if (*which == PREF_NOREAPER) {
      noreaper = *value;
      return;
   }
   if (*which == PREF_IRISGLMOUSE) {
      irisglmouse = *value;
      return;
   }
   if (*which == PREF_3DHWINTERP) {
      hwinterp = *value;
      return;
   }
   return;
}
void SET_PREF_I(int *which,int *value)
{
set_pref_i_(which,value);
}
void set_pref_f_(int *which,float *value)
{
   if (!read_prefs) init_prefs();
   if (*which == PREF_BACKUPRATE) {
      backuprate = *value;
      return;
   }
   return;
}
void SET_PREF_F(int *which,float *value)
{
set_pref_f_(which,value);
}

