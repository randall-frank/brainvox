C
C	The current MAP-3 state
C
	Integer*4	max_rois
	Parameter	(max_rois = 256)
	Integer*4	max_names
	Parameter	(max_names = 256)

	Integer*4	oper_init
	Integer*4	oper_reread
	Integer*4	oper_up
	Integer*4	oper_down
	Integer*4	oper_home
	Parameter	(oper_init = 0)
	Parameter	(oper_reread = 1)
	Parameter	(oper_up = 2)
	Parameter	(oper_down = 3)
	Parameter	(oper_home = 4)

	Integer*4	cwd_home
	Integer*4	cwd_dir
	Integer*4	cwd_pat_dir
	Integer*4	cwd_rois
	Parameter	(cwd_home = 0)
	Parameter	(cwd_dir = 1)
	Parameter	(cwd_pat_dir = 2)
	Parameter	(cwd_rois = 3)

	Integer*4	sw_none
	Integer*4	sw_linear
	Integer*4	sw_invlin
	Parameter	(sw_none = 0)
	Parameter	(sw_linear = 1)
	Parameter	(sw_invlin = 2)

        Structure       /MAP3/

		Integer*4	num
		Integer*4	n_image
		Integer*4	size_weight
		Real*4		offset
		Real*4		exp

		Integer*4	arb_mode(max_rois)
		Real*4		value(max_rois)

		Character*80	roi(max_rois)
		Character*256	dir(max_rois)

	End Structure

	Structure	/slist/

		Integer*4	num
		Integer*4	top
		Integer*4	select

		Integer*4	tag(max_names)
		Character*80	names(max_names)

	End Structure

