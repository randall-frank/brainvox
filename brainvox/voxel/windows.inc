C
C	This structure allows for multiple 2D windows controlled from
C	the main control panel.  Each can be in any mode.
C
	Structure /wind_struct/

		Integer*4	offset2d(2)	!origin (pixels at ll) 
		Integer*4	wid		!GL window ID (wid=-1=non-exist)
		Real*4		grid		!grid spacing
		Integer*4	gcol		!grid color
		Integer*4	zoom2d		!zoom factor
		Integer*4	mode		!slices,arb,MP_FIT
		Integer*4	inum2d		!slice number
		Integer*4	n_slice		!MP_FIT slice number
		Integer*4	rots(4)		!arb rx,ry,rz,d
		Logical		dirty		!resample needed flag
		Integer*4	im2d_ptr	!pointer to image data
		Integer*4	tex2d_ptr	!pointer to texture data
		Integer*4	histo(257)	!image histogram+# of pixels
		Integer*4	thisto(257)	!texture histogram
		Integer*4	texture_mode	!0=none,1=var1,2=var2
		Integer*4	cursor_locked	!3D cursor changes are tracked
	End Structure
C
	Integer*4	MAX_WINDOWS
	Parameter	(MAX_WINDOWS = 20)
C
C	Modes
C
	Integer*4	WIN_ARB_MODE
	Parameter	(WIN_ARB_MODE = 1)
	Integer*4	WIN_SLICE_MODE
	Parameter	(WIN_SLICE_MODE = 0)
	Integer*4	WIN_MPFIT_MODE
	Parameter	(WIN_MPFIT_MODE = 2)
C
C	Global structures
C
	Record	/wind_struct/	wind2d(MAX_WINDOWS)
C
C	index of current window and popup menu handles
C
	Integer*4	curwin,windows_menu,im_menu2,slice_menu,win_save
	common	/windowstuff/wind2d,curwin,windows_menu,
     +			im_menu2,slice_menu,win_save
C
C	3D cursor handling (display,tracking and location(x,y,z,color))
C
	Integer*4	cursor3d_disp,cursor3d_track,cursor3d_pos(4),
     +			cursor3d_menu,cursor3d_mode
	common	/cursor3dstuff/cursor3d_disp,cursor3d_track,
     +			cursor3d_pos,cursor3d_menu,cursor3d_mode
C
