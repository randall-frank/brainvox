
typedef struct {
        VLFLOAT64  zstretch;
        VLFLOAT64  jitter_frac;
        VLINT32     mult_min;
        VLINT32     mult_max;
        VLFLOAT64  rl_x;
        VLFLOAT64  rl_y;
        VLFLOAT64  rl_z;
        VLFLOAT64  rl_inten;
        VLFLOAT64  rl_srcinten;
        VLINT32 *l_table;
        VLINT32 hz_near;
        VLINT32 hz_far;
/* maybe the wrong place for this one... */
	VLINT32	*z_slice_mask;
} vl_light_params;

extern vl_light_params vl_light_globals;

