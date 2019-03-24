**********************************************
* Inc   file for brainvox_trace program...   *
* This file defines all the globals and any  *
* needed parameters...        17 June 91 rjf *
**********************************************
* Copyright (C) 1991 Image Analysis Facility *
*                    University of Iowa      *
**********************************************
*
*	Integer*4	filetype !0=hemis 1=rrgns 2=lrgns (replaced by cur_dir)
	Integer*4	skip     !skip=2 for PD/T2 or 1 for single images
	Integer*2 	PD(512*512)  !holder for PD
	Integer*2	T2(512*512)  !holder for T2
*
	Logical		background(max_rgns)   !true = backgroun disp on
	Integer*4	rois(max_rgns,4096,2)  ! rois for current image
	Logical		pd_l	! PD being displayed??
	Integer*4	cur_slice
	Integer*4	cur_roi
	Integer*4	zoom	! current zoom
	Integer*4	wx,wy	!current window port size
	Integer*4	editstate,edit(3,2)  ! editing state ie # of points
					     ! defined in edit(pt,xy)

	Integer*4	autolevel    
	Integer*4	palette(768),b(3),c(3),cpalette(258),tpalette(768)
*
*	holders for sgiuif dialogs
*
	record/ditem/	state1(50)
	record/ditem/	state2(50)
*
*	New stuff for brainvox 1.1
*
	Real*4		Xoffset		! in pixels
	Real*4		Yoffset		! in pixels
	Real*4		zeroslice	! in slices??
	Character*10	Units		! units label
*
*	Added for bimodal tracing 1.6
*
	Integer*4	exact_tracing
*
	common/global/pd,t2,wx,wy,
     +		background,rois,pd_l,
     +		cur_slice,cur_roi,zoom,state1,state2,editstate,edit,skip,
     +		autolevel,palette,b,c,cpalette,tpalette,
     +		Xoffset,Yoffset,zeroslice,exact_tracing,
     +		units
*
*	Private messages
*
	Integer*4	IPC_T_MODECHANGE
	Parameter	(IPC_T_MODECHANGE = 0)
