C
C	Includes for the arbitrary angle trace window
C
	Structure	/arbset/
		Real*4		islice
		Integer*4	base_slice
		Integer*4	num_slices
		Integer*4	match_slice
		Integer*4	rots(3)
		Character*(40)	name
	End structure

	Record/ditem/	arb_uif(30)
	Record/arbset/	new_settings,undo_settings
	Record/arbset/	cur_settings(max_rgns)  !if arb_mode .ne. 0
	Integer*4	arb_mode
	Integer*4	oldslices     !# of previously loaded ROI slices
	Real*8       	myset(1100)   !vset for arb mode

	Common/arb_gbls/arb_uif,arb_mode,new_settings,cur_settings,
     +		oldslices,myset,undo_settings
C
