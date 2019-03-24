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
C    MODULE NAME    : brw   
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        :   Brainvox
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 8 Sept 94      
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routines to calibrate BRW coords and convert to/from
C			BRW/MM for the MRI head
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
C
	Subroutine bv_calib_brw()

	Implicit None

	Include 'mri_brw_inc.f'

	Real*4	points(9,3)
	Character*80	file
	Integer*4	I

	BRW_valid = 0

	file = "_brwpts"
	Open(66,file=file,form="formatted",status="old",
     +		readonly,err=999)
	Do While(.true.)
		read(66,*,err=998,end=998) i
		read(66,*,err=998,end=998) points(i,1),points(i,2),
     +			points(i,3)
	Enddo
998	Close(66)

	Call bv_calibrate_brw(points)
	Type 10
	Return

10	Format("BRWsystem - BRW coordinates calibrated.")
11	Format("BRWsystem - Warning, BRW coordinates not calibrated.")

999	Continue
	Type 11
	Return
	End
C
C	Take coords in MM of the rods and convert to BRW
C
#undef DEBUG
C
	Subroutine bv_calibrate_brw(points)

	Implicit None

	Include 'mri_brw_inc.f'

	Real*4		points(9,3),ratio(3)
	Integer*4	i,j
C
C MRI image                ^
C                    R     | +y  L
C                O 1       |       9
C                          |       8
C                         o+----------> +x
C                  2                
C                  3               7
C                       4   5 6
C
C	Ratio is the fraction of the hex side 
C
	i = 1
	Do j=1,3
		ratio(j) = sqrt( (points(i+1,1) - points(i,1))**2 +
     +			(points(i+1,2) - points(i,2))**2 +
     +                  (points(i+1,3) - points(i,3))**2) /
     +			sqrt( (points(i+2,1) - points(i,1))**2 +
     +			(points(i+2,2) - points(i,2))**2 +
     +                  (points(i+2,3) - points(i,3))**2) 
		i = i + 3
	Enddo
#ifdef DEBUG
	type *,"Ratios:",ratio(1),ratio(2),ratio(3)
#endif
C
C	Compute BRW coords (at the base)
C
	BRW(1,1) = -side_offset
	BRW(1,2) = side_length/2.0
	BRW(1,3) = delta_z-z_up
C
	BRW(3,1) = -side_offset
	BRW(3,2) = -side_length/2.0
	BRW(3,3) = -z_up
C
	BRW(4,1) = -back_length/2.0
	BRW(4,2) = -back_offset
	BRW(4,3) = BRW(1,3)
C
	BRW(6,1) = back_length/2.0
	BRW(6,2) = BRW(4,2)
	BRW(6,3) = BRW(3,3)
C
	BRW(7,1) = -BRW(1,1)
	BRW(7,2) = BRW(1,2)
	BRW(7,3) = BRW(1,3)
C
	BRW(9,1) = -BRW(3,1)
	BRW(9,2) = BRW(3,2)
	BRW(9,3) = BRW(3,3)
C
C	Use ratio to determine where between the rods
C
	BRW(2,1) = (ratio(1) * (BRW(3,1) - BRW(1,1))) + BRW(1,1)
	BRW(2,2) = (ratio(1) * (BRW(3,2) - BRW(1,2))) + BRW(1,2)
	BRW(2,3) = (ratio(1) * (BRW(3,3) - BRW(1,3))) + BRW(1,3)
C
	BRW(5,1) = (ratio(2) * (BRW(6,1) - BRW(4,1))) + BRW(4,1)
	BRW(5,2) = (ratio(2) * (BRW(6,2) - BRW(4,2))) + BRW(4,2)
	BRW(5,3) = (ratio(2) * (BRW(6,3) - BRW(4,3))) + BRW(4,3)
C
	BRW(8,1) = (ratio(3) * (BRW(9,1) - BRW(7,1))) + BRW(7,1)
	BRW(8,2) = (ratio(3) * (BRW(9,2) - BRW(7,2))) + BRW(7,2)
	BRW(8,3) = (ratio(3) * (BRW(9,3) - BRW(7,3))) + BRW(7,3)
#ifdef DEBUG
	do i=1,9
	type *,"BRW",i,BRW(i,1),BRW(i,2),BRW(i,3)
	enddo
#endif
C
	Do i=1,9
		Do j=1,3
			MRI(i,j) = points(i,j)
		Enddo
	Enddo
#ifdef DEBUG
	do i=1,9
	type *,"MRI",i,MRI(i,1),MRI(i,2),MRI(i,3)
	enddo
#endif
C
C	Now, the MM points(2.5.8) are equiv to BRW points(2.5.8)
C
C	Compute the X,Y,Z Axis for MRI space
C	centroid is 2 and xaxis is through 5
C
	Do i=1,3
		MRIxaxis(i) = points(5,i) - points(2,i)
C	temp y axis
		MRIyaxis(i) = points(8,i) - points(2,i)
		BRWxaxis(i) = BRW(5,i) - BRW(2,i)
C	temp y axis
		BRWyaxis(i) = BRW(8,i) - BRW(2,i)
	Enddo
C
C	true X vectors
C
	Call normalize_vec(MRIxaxis)
	Call normalize_vec(BRWxaxis)
C
C	temp Ys
C
	Call normalize_vec(MRIyaxis)
	Call normalize_vec(BRWyaxis)
C
C	compute Z axis using X cross Y
C
	Call xprod_vec(MRIxaxis,MRIyaxis,MRIzaxis)
	Call xprod_vec(BRWxaxis,BRWyaxis,BRWzaxis)
	Call normalize_vec(MRIzaxis)
	Call normalize_vec(BRWzaxis)
C
C	compute true Y axis using Z cross X
C
	Call xprod_vec(MRIzaxis,MRIxaxis,MRIyaxis)
	Call xprod_vec(BRWzaxis,BRWxaxis,BRWyaxis)
	Call normalize_vec(MRIyaxis)
	Call normalize_vec(BRWyaxis)
C
	BRW_valid = 1
C
#ifdef DEBUG
	type *,"MRIx",MRIxaxis(1),MRIxaxis(2),MRIxaxis(3)
	type *,"MRIy",MRIyaxis(1),MRIyaxis(2),MRIyaxis(3)
	type *,"MRIz",MRIzaxis(1),MRIzaxis(2),MRIzaxis(3)
	type *,"BRWx",BRWxaxis(1),BRWxaxis(2),BRWxaxis(3)
	type *,"BRWy",BRWyaxis(1),BRWyaxis(2),BRWyaxis(3)
	type *,"BRWz",BRWzaxis(1),BRWzaxis(2),BRWzaxis(3)
#endif
	Return
	End
C
C	Routine for conversion of MRI to BRW
C
	Subroutine MRI_to_BRW(in,out)

	Implicit None

	Include 'mri_brw_inc.f'

	Real*4	in(3),out(3)
	Real*4	t(3),v(3)
	Integer*4	i
C
	If (BRW_valid .eq. 0) Then
		out(1) = in(1)
		out(2) = in(2)
		out(3) = in(3)
		Return
	Endif
C
C	Point 2 is the centroid
C
	Do i=1,3
		t(i) = in(i) - MRI(2,i)
	Enddo
C
C	dot product for coords along MRIaxis
C
	Call dprod_vec(t,MRIxaxis,v(1))
	Call dprod_vec(t,MRIyaxis,v(2))
	Call dprod_vec(t,MRIzaxis,v(3))
C
C	These coords are the same as the BRW axis coords
C	so convert to BRW space starting at BRW centroid
C
	Do i=1,3
		t(i) = BRW(2,i)
	Enddo
C
C	and moving along BRWaxis
C
	Do i=1,3
		t(i) = t(i) + (BRWxaxis(i)*v(1))
	Enddo
	Do i=1,3
		t(i) = t(i) + (BRWyaxis(i)*v(2))
	Enddo
	Do i=1,3
		t(i) = t(i) + (BRWzaxis(i)*v(3))
	Enddo
C
C	that's it!
C
	Do i=1,3
		out(i) = t(i)
	Enddo
C
	Return
	End
C
C	Routine for conversion of BRW to MRI
C
	Subroutine BRW_to_MRI(in,out)

	Implicit None

	Include 'mri_brw_inc.f'

	Real*4	in(3),out(3)
	Real*4	t(3),v(3)
	Integer*4	i
C
	If (BRW_valid .eq. 0) Then
		out(1) = in(1)
		out(2) = in(2)
		out(3) = in(3)
		Return
	Endif
C
C	Point 2 is the centroid
C
	Do i=1,3
		t(i) = in(i) - BRW(2,i)
	Enddo
C
C	dot product for coords along BRWaxis
C
	Call dprod_vec(t,BRWxaxis,v(1))
	Call dprod_vec(t,BRWyaxis,v(2))
	Call dprod_vec(t,BRWzaxis,v(3))
C
C	These coords are the same as the MRI axis coords
C	so convert to MRI space starting at MRI centroid
C
	Do i=1,3
		t(i) = MRI(2,i)
	Enddo
C
C	and moving along MRIaxis
C
	Do i=1,3
		t(i) = t(i) + (MRIxaxis(i)*v(1))
	Enddo
	Do i=1,3
		t(i) = t(i) + (MRIyaxis(i)*v(2))
	Enddo
	Do i=1,3
		t(i) = t(i) + (MRIzaxis(i)*v(3))
	Enddo
C
C	that's it!
C
	Do i=1,3
		out(i) = t(i)
	Enddo
C

	Return
	End
C
C	Vector handling routines 
C
C	Normalize
C
	Subroutine normalize_vec(v)

	Implicit None

	Real*4	v(3)
	Real*4	d

	d = sqrt(v(1)**2 + v(2)**2 + v(3)**2)

	v(1) = v(1) / d 
	v(2) = v(2) / d 
	v(3) = v(3) / d 
	
	Return
	End
C
C	Xproduct
C
	Subroutine xprod_vec(x,y,z)

	Implicit None

	Real*4	x(3),y(3),z(3)

        z(1) = (x(2)*y(3)) - (x(3)*y(2))
        z(2) = (x(3)*y(1)) - (x(1)*y(3))
        z(3) = (x(1)*y(2)) - (x(2)*y(1))

	Return
	End
C
C	dotproduct
C
	Subroutine dprod_vec(x,y,d)

	Implicit None

	Real*4  x(3),y(3),d

	d = (x(1)*y(1)) + (x(2)*y(2)) + (x(3)*y(3))
	
	Return
	End
