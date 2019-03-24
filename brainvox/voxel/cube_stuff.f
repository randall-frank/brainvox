C    ======================================================================     
C    THIS SOFTWARE IS THE PROPERTY OF:                                          
C         The Image Analysis Facility                                     
C         71 HBRF     
C         The University of Iowa, Iowa City, Iowa                               
C    ======================================================================     
C       Copyright (C) 1989 Image Analysis Facility, University of Iowa
C    ======================================================================     
C                                                                               
C    -- OVERVIEW --------------                                                 
C                                                                               
C    MODULE NAME    : cube_stuff.f
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox_voxel
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 31 Dec 92       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	a routines to draw the cube view
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     
C    INPUTS         :          
C    OUTPUTS        : 
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 

	Subroutine draw_cube(myset,rots,MP_disp)

	Implicit None

        Include 'fdevice.h'
        Include 'fgl.h'
        Include 'sgiuif.h'
        Include '../libs/vt_inc.f'
        Include 'voxeluif.inc'

        Include 'measure.inc'
        Include 'rotsdef.inc'
	Include 'windows.inc'

        Record  /rotsrec/       rots
	Integer*4	myset(*)
	Integer*4	MP_disp,i,h
	Real*4		frots(12)

        Call pushvi
        Call viewpo(5,105,5,105)
        Call ortho(-225.0,225.0,-225.0,225.0,-225.0,225.0)
	h = '00555555'X
        Call cpack(h)
	Call clear
C	Call zclear
	h = '00ffffff'X
        Call cpack(h)
C	Call zbuffe(.true.)
        Call vl_drawcube(myset)
        if (wind2d(curwin).mode .eq. 1) then
                Do i = 1,3
                        frots(i) = wind2d(curwin).rots(i)
                Enddo
                frots(4) = wind2d(curwin).rots(4)-256
                Call vl_splane(%val(imagesize),%val(imagesize),
     +                  frots(1),frots(4),myset)
        Else if (wind2d(curwin).mode .eq. 2) then
C
C       Draw MP_FIT graphics overlay
C
                Call draw_mp_fit(rots,myset,imagesize,
     +                  interpixel,MP_disp)
        Endif
C	Call zbuffe(.false.)
        Call popvie

	Return
	End
