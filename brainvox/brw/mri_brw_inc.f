	
	Real*4          MRI(9,3),BRW(9,3)
	Real*4          MRIxaxis(3),MRIyaxis(3),MRIzaxis(3)
	Real*4          BRWxaxis(3),BRWyaxis(3),BRWzaxis(3)
	Integer*4	BRW_valid

	common	/brw_common/MRI,BRW,MRIxaxis,MRIyaxis,MRIzaxis,
     +		BRWxaxis,BRWyaxis,BRWzaxis,BRW_valid

	Real*4          side_length,delta_z,z_down,back_length
	Real*4		back_offset,side_offset,z_up

	Parameter       (side_length = 170.0)
	Parameter       (side_offset = (186.0/2.0) )
	Parameter       (back_length = 130.0)
	Parameter       (back_offset = 110.0)
	Parameter       (delta_z = (94.5*2.0) )
	Parameter       (z_down = 25.62)
	Parameter	(z_up = ((delta_z/2.0)-z_down) )

