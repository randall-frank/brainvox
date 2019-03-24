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
C    MODULE NAME    : texture_clip
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 19 May 95       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	routines to interface brainvox with the projected
C			trace clipping routines (clip_volume.c)
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

        Subroutine clip_texture(points,numpts,myset,mode,oper,
     +			ival,ival2)

        Implicit None
        Include 'sgiuif.h'
        Include '../libs/vt_inc.f'

        Include 'voxeluif.inc'
        Include 'measure.inc'

        Real*8          verts(4096*3)
	Integer*4	iverts(4096*3)
        Integer*4       nverts,i,col,inum
        Integer*4       points(4096,4),numpts,myset(*)
        Real*4          squ
	Integer*4	ival,oper,what,mode,ival2
C
C	arrow changes
C
	Call wait_cursor
C
C	Unpack mode/oper/val into the params for the clipping call
C
	If (mode .eq. IPC_TEXUTIL) Then
C
C	Utility functions
C
		If (oper .eq. IPC_TEX_UTIL_MULTC) Then
			what = -6
		Else if (oper .eq. IPC_TEX_UTIL_MULTV) Then
			what = -7
		Else if (oper .eq. IPC_TEX_UTIL_SWAP) Then
			what = -8
		Else if (oper .eq. IPC_TEX_UTIL_ADDC) Then
			what = -9
		Else if (oper .eq. IPC_TEX_UTIL_ADDV) Then
			what = -10
		Else if (oper .eq. IPC_TEX_UTIL_TEX2VOL) Then
			what = -11
		Else if (oper .eq. IPC_TEX_UTIL_VOL2TEX) Then
			what = -12
		Else if (oper .eq. IPC_TEX_UTIL_FFILL) Then
C
C	Get the points and do the flood fill (on the texture volume)
C
        		Call vl_getsqu(squ,myset)
        		inum = realimages + (realimages-1)*interpimages
C
C	get the points in matrix space
C
        		nverts = 0
        		Do i=numpts,1,-1
                		If (points(i,4) .eq. linedata.color) Then
                        		iverts(1+nverts*3)=points(i,1)
                        		iverts(2+nverts*3)=points(i,2)
                        		iverts(3+nverts*3)=points(i,3)
                        		nverts = nverts + 1
                		Endif
        		Enddo
C
C	Do the flood fill
C
			Call do_flood_fill(myset,iverts,%val(nverts),
     +				%val(ival),%val(ival2),%val(1))
			goto 1000
C
		Else if (oper .eq. IPC_TEX_UTIL_REBOUND) Then
			Call recalc_volume_limits(myset)
			goto 1000
		Else
			goto 1000
		Endif
		Call fill_volume(myset,%val(1),%val(what),verts,%val(ival))
		goto 1000
	Else if (mode .eq. IPC_TEXFILL) Then
C
C	fill the volume
C
		If (oper .eq. IPC_TEX_CL_PAINT) Then
			what = -3
		Else if (oper .eq. IPC_TEX_CL_TAG) Then
			what = -5
		Else if (oper .eq. IPC_TEX_CL_UNTAG) Then
			what = -4
		Else
			goto 1000
		Endif
		Call fill_volume(myset,%val(1),%val(what),verts,%val(ival))
		goto 1000
	Else
C
C	Clip to region
C
		If (oper .eq. IPC_TEX_CL_PAINT) Then
			what = ival
		Else if (oper .eq. IPC_TEX_CL_TAG) Then
			what = -2
		Else if (oper .eq. IPC_TEX_CL_UNTAG) Then
			what = -1
		Else
			goto 1000
		Endif
	Endif
C
        Call vl_getsqu(squ,myset)
        inum = realimages + (realimages-1)*interpimages
C
C	get the points and convert into voxel (cubic:0,0,0) space
C
        nverts = 0
        Do i=numpts,1,-1
                If (points(i,4) .eq. linedata.color) Then
                        verts(1+nverts*3)=points(i,1)-(0.5*imagesize)
                        verts(2+nverts*3)=points(i,2)-(0.5*imagesize)
                        verts(3+nverts*3)=(points(i,3)-(0.5*inum))*squ
                        nverts = nverts + 1
                Endif
        Enddo
C
C	Do the clipping operation
C
	Call fill_volume(myset,%val(1),%val(what),verts,%val(nverts))
C
C	All done!
C
1000	Call arrow_cursor

        Return
        End

