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
C    MODULE NAME    : p2dtop3d, p3dtop2d
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
C    DATE           : 20 Sep 89       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routines to convert points from 2D to 3D and
C			vice versa.
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
C	routine to convert 2D + Z point into 3D
C
	Subroutine p2dtop3d(p2d,p3d,myset,wx,wy)

	Implicit None

	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'

	Integer*4	p2d(3),p3d(3),myset(*),wx,wy,inum

	Real*4		mat(9),inv(9),p(3),squeeze
C
C	get the conversion matrix
C	
	Call vl_getmatrix(mat,inv,myset)
	Call vl_getsqu(squeeze,myset)
C	if (squeeze .ne. 1.0) write(*, *)"Squeeze = ",squeeze
C
C	convert to cube centered coords
C
	p(1) = p2d(1) - (0.5*imagesize)
	p(2) = p2d(2) - (0.5*imagesize)
	inum = realimages + (realimages-1)*interpimages
	p(3) = (p2d(3) - (0.5*inum))*squeeze   
C
C	mult the point by the matrix
C
	Call vox_matmult(p,mat,p)
C
C	Here we would translate if needed
C
C
C	convert to output image space
C
	p3d(1) = p(1) + (wx*0.5)
	p3d(2) = p(2) + (wy*0.5)
	p3d(3) = p(3)
C
	Return
	End
C
C	routine to convert 3D point into 2D + Z
C
	Subroutine p3dtop2d(p3d,p2d,myset,wx,wy,thres)

	Implicit None

	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'

	Integer*4	p2d(3),p3d(3),myset(*)
	Integer*4	pi(3),i,inum,thres,wx,wy,j
	
	Real*4		mat(9),inv(9),p(3),p2(3),pt(3),squeeze
	Integer*4       zbuff_ptr,image_ptr,zdx,zdy
C
C	get the conversion matrix
C	
	Call vl_getmatrix(mat,inv,myset)
	Call vl_getsqu(squeeze,myset)
C	if (squeeze .ne. 1.0) write(*, *)"Squeeze = ",squeeze
	Call vl_getbufs(image_ptr,zbuff_ptr,zdx,zdy,myset)
C
C	compute number of real images
C
	inum = realimages + (realimages-1)*interpimages
C
C	first convert from image space into space
C
	p(1) = p3d(1) - (wx*0.5)
	p(2) = p3d(2) - (wy*0.5)
	Call get_z_value(%val(zbuff_ptr),zdx,zdy,p3d(1),p3d(2),p(3))
C
C	Thres of -999 means to map the point to the plane z=0
C
	If (thres .eq. -999) Then
		p(3) = 0
	Endif
C
C	Subtract 0.5 from Z (voxlib adds 0.5)
C
	p(3) = p(3) - 0.5
C
C	mult by inverse matrix
C
	Call vox_matmult(p,inv,p2)
C
C	Place back into volume space
C
	p2d(1) = p2(1) + (0.5*imagesize)
	p2d(2) = p2(2) + (0.5*imagesize)
	p2d(3) = (p2(3)/squeeze) + (0.5*inum)
C
C	within the volume?
C
	if (p2d(1) .lt. 0) goto 100
	if (p2d(2) .lt. 0) goto 100
	if (p2d(3) .lt. 0) goto 100
	if (p2d(1) .ge. imagesize) goto 100
	if (p2d(2) .ge. imagesize) goto 100
	if (p2d(3) .ge. inum) goto 100
	p3d(3) = p(3)

	Return
C
C	invalid point
C
100	Continue

	p2d(1) = -1

	Return

	End
C
C	Matrix multiplier
C
	Subroutine vox_matmult(in,mat,out)

	Implicit None

	Real*4	in(3),out(3),temp(3),mat(9)

C
C	mult in*mat
C
	temp(1) = in(1)*mat(1) + in(2)*mat(2) + in(3)*mat(3)
	temp(2) = in(1)*mat(4) + in(2)*mat(5) + in(3)*mat(6)
	temp(3) = in(1)*mat(7) + in(2)*mat(8) + in(3)*mat(9)
C
C	copy temp to out
C
	out(1) = temp(1)
	out(2) = temp(2)
	out(3) = temp(3)
C
	return
	end
C
C	Routine to probe the zbuffer
C
	Subroutine get_z_value(zbuffer,dx,dy,x,y,z)

	Implicit None

	Integer*2	zbuffer(*)
	Integer*4	dx,dy,x,y
	Real*4		z

	z = float(zbuffer(y*dx+x+1))/8.0
	
	Return
	End
C
C	routine to get a pixel value (in three space)
C
	subroutine get_pix(j,pi,image,dx,dy)

	Implicit None
	
	Integer*4	j,pi(3),dx,dy
	byte		image(*)

	j = image((pi(3)*dx*dy) + (pi(2)*dx) + pi(1) + 1)
	if (j .lt. 0) j = j + 256
	
	return
	end
C
C	routine to convert 3D point into 2D + Z
C
	Subroutine p3dtop2d_orig(p3d,p2d,myset,wx,wy,thres)

	Implicit None

	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'

	Integer*4	p2d(3),p3d(3),myset(*)
	Integer*4	pi(3),i,inum,thres,wx,wy,j
	Integer*4	lpal(0:255),rpal(0:255)
	
	Real*4		mat(9),inv(9),p(3),p2(3),pt(3),squeeze
	logical 	int
C
C	get the conversion matrix
C	
	Call vl_getmatrix(mat,inv,myset)
	Call vl_getsqu(squeeze,myset)
C	if (squeeze .ne. 1.0) write(*, *)"Squeeze = ",squeeze
	Call vl_getpals(rpal,lpal, myset)
	int = .false.
C
C	compute number of real images
C
	inum = realimages + (realimages-1)*interpimages
C
C	first convert from image space into space
C
	p(1) = p3d(1) - (wx*0.5)
	p(2) = p3d(2) - (wy*0.5)
C
C	now shoot along Z values from effective +inf to -inf
C
	Do i=imagesize,-imagesize,-1
		p(3) = float(i)
C
C	mult by inverse matrix
C
		call vox_matmult(p,inv,p2)
C
C	integrity check
C
		goto 1000    !bypass integrity check for now...
C
		call vox_matmult(p2,mat,pt)
		if ((.not. int) .and.(
     +			 (abs(pt(1)-p(1)) .gt. 0.1).or.
     +			 (abs(pt(2)-p(2)) .gt. 0.1).or.
     +			 (abs(pt(3)-p(3)) .gt. 0.1)
     +				)) then
			write(*, *)"Integrity check failed!!!"
			write(*, 10)mat(1),mat(2),mat(3),mat(4),mat(5),mat(6),
     +				mat(7),mat(8),mat(9),
     +				inv(1),inv(2),inv(3),inv(4),inv(5),inv(6),
     +				inv(7),inv(8),inv(9)
10	Format("Original matrix (FORTRAN):",/,
     +	       3F7.4,/,3F7.4,/,3F7.4,/,
     +		"Inverse matrix (FORTRAN):",/,
     +	       3F7.4,/,3F7.4,/,3F7.4)
			int = .true.
		Endif
C
1000		Continue
C
C	add offsets so we're in 3d image index space
C
		pi(1) = p2(1) + (0.5*imagesize)
		pi(2) = p2(2) + (0.5*imagesize)
		pi(3) = (p2(3)/squeeze) + (0.5*inum)
C
C	Valid point?
C
		if ((pi(1) .lt. 0) .or. (pi(1) .ge. imagesize)) goto 100
		if ((pi(2) .lt. 0) .or. (pi(2) .ge. imagesize)) goto 100
		if ((pi(3) .lt. 0) .or. (pi(3) .ge. inum)) goto 100
C
C	get the pixel value
C
		Call get_pix(j,pi,%val(images_addr),imagesize,imagesize)
		j = iand(255,ishft(rpal(j),-24))
		if (j .gt. thres) goto 200
C
C	no go... try again
C
100		continue	
	Enddo
C
C	no intersection
C
	p2d(1) =-1
	return
C
C	found it!!
C
200	Continue
C
C	return the point
C
	p3d(3) = i
	p2d(1) = pi(1)
	p2d(2) = pi(2)
	p2d(3) = pi(3)
C
	Return
	End
