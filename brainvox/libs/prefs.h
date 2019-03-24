typedef struct {
        char            name[256];
        long int        found;
        void            *data;
} FTag;

#define PREF_NOREAPER		0
#define PREF_IRISGLMOUSE	1
#define PREF_BACKUPRATE		2
#define PREF_3DHWINTERP		3

void get_pref_f_(int *which,float *value);
void get_pref_i_(int *which,int *value);
void set_pref_f_(int *which,float *value);
void set_pref_i_(int *which,int *value);
void file_pref_(int *wrt);

void    read_tagged_file_(char *file,long int *num,FTag *tags,long int *err);
void    write_tagged_file_(char *file,char *note,long int *update,
                long int *num,FTag *tags,long int *err);

