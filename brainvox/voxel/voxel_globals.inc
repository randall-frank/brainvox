C
C	Global data for the brainvox_voxel program
C
C	Data for the control and 3D windows
C	
        Integer*4       zoom3d,image3dsize,wid,wid2,winput
        Integer*4       offset3d(2)
        Integer*4       save_ptr,zbufptr
	Integer*4	gcol1,im_menu
	Real*4		grid1

        Common /voxel_winds/zoom3d,offset3d,save_ptr,zbufptr,winput,
     +		wid,wid2,image3dsize,gcol1,im_menu,grid1

	Real*8		myset(1200)
	Integer*4       rendermode,use_16bit,over_render,use_hardware
	Integer*4	auto_render,use_hwinterp

	Common /voxel_render/myset,rendermode,use_16bit,over_render,
     +		use_hardware,auto_render,use_hwinterp

