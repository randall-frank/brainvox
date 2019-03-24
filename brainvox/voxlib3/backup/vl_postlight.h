
typedef struct {
        double  zstretch;
        double  jitter_frac;
        int     mult_min;
        int     mult_max;
        double  rl_x;
        double  rl_y;
        double  rl_z;
        double  rl_inten;
        double  rl_srcinten;
        long int *l_table;
        long int hz_near;
        long int hz_far;
} vl_light_params;

extern vl_light_params vl_light_globals;

