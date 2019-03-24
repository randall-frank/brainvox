/* prototypes */
long int write_OBJ_file(char *file,long int nvert,long int ntris,
        float *verts,float *norms,long int *tris);
long int read_OBJ_file(char *file,long int *nverts,long int *ntris,
        float **pts,float **norms,long int **tris);
long int write_geom_file(char *file,long int nvert,long int ntris,
        float *verts,float *norms,long int *tris);
long int read_geom_file(char *file,long int *nverts,long int *ntris,
        float **pts,float **norms,long int **tris);
