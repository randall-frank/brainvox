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
C    MODULE NAME    : calc_sa 
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
C    DATE           : 9 Sept 91       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	This routine computes the surface area enclosed by
C			an ROI using the Zbuffer.
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
C    NOTES          :      This computation is only valid for a specific
C			rotation/opacity setting.  It must be recomputed
C			should these parameters change.
C                                                                               
C    ====================================================================== 
	Subroutine calc_sa(points,numpts,color,zbuffer,image,dy,dx,SA,
     +		sa_flag)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'voxel_globals.inc'

	Real*4		SA
	Integer*4	points(4096,4)  ! x,y,?,color
	Integer*4	trace(4096,2),numpts,color
	Integer*2	zbuffer(*)
	Integer*4	image(*)
	Integer*4	dx,dy
	Real*4		z1,z2,z3,z4,v1(3),v2(3)
	Real*4		ftemp,norm1(3),norm2(3)
	Integer*4	I,x,y,jj
	Integer*4	ptr,list(4096),count
	logical		error
C
	Integer*4	vdata(1024*1024),outptr
	Integer*4	sa_flag

	Real*4		OUT_THERE
	Parameter	(OUT_THERE = -3600.0)

C
C	Assume bad
C
	SA = 0.0
C
C	Get the points in question
C
	count = 0
	Do i=1,numpts
		If (points(i,4) .eq. color) then
			count = count + 1
			trace(count,1) = points(i,1)
			trace(count,2) = points(i,2)
		Endif
	Enddo
C
C	did we get here by mistake ???
C
	if (count  .lt. 3) then
		return
	Endif
C	type *,count,"points"
C
C	close it off
C
	count = count + 1
	trace(count,1) = trace(1,1)
	trace(count,2) = trace(1,2)
C
C	call polyfill
C
	ptr=4096
	Call polyfill(trace,count,list,ptr)
	if (ptr .eq. -1) then
		write(*, 13)
13	Format('Unable to complete roi fill operation. Roi too complex.')
		return
	Endif
	If (ptr .lt. 3) return
C	type *,ptr,"ptr"
C
C	SA_FLAG = 0 = no computation
C	SA_FLAG = 1 = computation only
C	SA_FLAG = 2 = computation and FLIP file
C	SA_FLAG = 3 = computation and CFLIP file
C
	If ((sa_flag .eq. 2).or.(sa_flag .eq. 3)) then
		outptr = 4
		vdata(1) = '00005423'X
		vdata(2) = 0
		vdata(3) = 0
	Endif
C
C
C	for each scan line
C
	Do i=1,ptr-2,3
C
C	and each pixel 
C
		Y=list(i+2)
		If ((y .ge. 0).and.(y .lt. image3dsize-2)) Then
		If (list(i) .lt. 0) list(i) = 0
		If (list(i+1) .ge. image3dsize-2) list(i+1) = image3dsize-2
		If (list(i) .le. list(i+1)) then
C
		Do X=list(i),list(i+1)-1
C
C	The rect (current point is (1)):
C		(2)-----(3)
C	         | A \ B |
C		(1)-----(4)	
C
C	Get the four Z values
C
			jj=y*dx+x+1
			z1 = zbuffer(jj)
			z2 = zbuffer(jj+dx)
			z3 = zbuffer(jj+dx+1)
			z4 = zbuffer(jj+1)
C
C	Add the triangles IF the points are not at infinity
C
			If ((z1 .gt. OUT_THERE).and.(z2 .gt. OUT_THERE).and.
     +			   (z3 .gt. OUT_THERE).and.(z4 .gt. OUT_THERE)) then
C
C	recover the subpixel stuff
C
				z1 = z1/8.0
				z2 = z2/8.0
				z3 = z3/8.0
				z4 = z4/8.0
C
C	get the vectors
C
C	V12
				v1(1) = 0
				v1(2) = 1
				v1(3) = (z2 - z1)
C	V14
				v2(1) = 1
				v2(2) = 0
				v2(3) = (z4 - z1)
C
C	compute area of triangle
C			
				Call onetri(v2,v1,ftemp,norm1)
				SA = SA + abs(ftemp)
C
C	get the vectors
C
C	V32
				v1(1) = -1
				v1(2) = 0
				v1(3) = (z2 - z3)
C	V34
				v2(1) = 0
				v2(2) = -1
				v2(3) = (z4 - z3)
C
C	compute area of triangle
C			
				Call onetri(v1,v2,ftemp,norm2)
				SA = SA + abs(ftemp)
C
C	Add to quad mesh list
C
				If (sa_flag .eq. 2) then
				If (outptr .lt. (1024*1024 - 50)) then
					Call addtomesh(float(x+1),
     +		float(y),z4,norm2(1),norm2(2),norm2(3),vdata,outptr)
					Call addtomesh(float(x+1),
     +		float(y+1),z3,norm2(1),norm2(2),norm2(3),vdata,outptr)
					Call addtomesh(float(x),
     +		float(y+1),z2,norm1(1),norm1(2),norm1(3),vdata,outptr)
					Call addtomesh(float(x),
     +		float(y),z1,norm1(1),norm1(2),norm1(3),vdata,outptr)
				vdata(2) = vdata(2) + 4
				Endif
				Else if (sa_flag .eq. 3) then
				If (outptr .lt. (1024*1024 - 50)) then
					jj=y*dx+x+1
					call getRGBcol(image,norm1,jj+1)
					Call addtomesh(float(x+1),
     +		float(y),z4,norm1(1),norm1(2),norm1(3),vdata,outptr)
					call getRGBcol(image,norm1,jj+dx+1)
					Call addtomesh(float(x+1),
     +		float(y+1),z3,norm1(1),norm1(2),norm1(3),vdata,outptr)
					call getRGBcol(image,norm1,jj+dx)
					Call addtomesh(float(x),
     +		float(y+1),z2,norm1(1),norm1(2),norm1(3),vdata,outptr)
					call getRGBcol(image,norm1,jj)
					Call addtomesh(float(x),
     +		float(y),z1,norm1(1),norm1(2),norm1(3),vdata,outptr)
				vdata(2) = vdata(2) + 4
				Endif
				Endif
			Endif
		Enddo
		Endif
		Endif
	Enddo
C
C	done
C
	if ((sa_flag .eq. 2).or.(sa_flag .eq.3)) then
	Call avflipdata(vdata(4),vdata(2))
	Call writebinfile("Voxel.surf ",vdata(1),(outptr-1)*4,error)
	Endif
C
	return
	end
C
C	Subroutine to compute the area defined by two vectors in space
C
	Subroutine onetri(v1,v2,area,r)

	Implicit None

	Integer*4	i
	Real*4		v1(3),v2(3)
	Real*4		area,r(3)
C
C	assume bad
C
	area = 0.0
C
C	compute X prod		1   2   3    
C				v11 v12 v13 v11 v12
C				v21 v22 v23 v21 v22
C
	r(1) = ((v1(2)*v2(3)) - (v1(3)*v2(2)))
	r(2) = ((v1(3)*v2(1)) - (v1(1)*v2(3)))
	r(3) = ((v1(1)*v2(2)) - (v1(2)*v2(1)))
C
C	1/2 length of resultant vector is the area
C
	area = sqrt(r(1)**2.0 + r(2)**2.0 + r(3)**2.0)
C	
C	Normalize the vector
C
	Do i=1,3
		r(i) = r(i)/area
	Enddo
C
	area = area*0.5
C
C	done
C
	return
	end
C
C	FLIP format routines
C
	Subroutine	addtomesh(x,y,z,nx,ny,nz,data,ptr)

	Implicit None

	Real*4		x,y,z
	Integer*4	ptr
	Real*4		nx,ny,nz
	Real*4		data(*)

	data(ptr) = (nx)
	data(ptr+1) = (ny)
	data(ptr+2) = (nz)
	data(ptr+3) = (x)
	data(ptr+4) = (y)
	data(ptr+5) = (z)
	ptr = ptr + 6

	return
	End

	Subroutine avflipdata(data,count)         

	Implicit None

	Integer*4	I,j,count,ptr
	Real*4		data(*)
	Real*4		maxv(3),minv(3),v,mean(3),scale
C
C	get max,min
C
	Do I=1,3
		minv(i) = 100000
		maxv(i) = -100000
	Enddo
	Do I=1,count
		ptr = ((I-1)*6)+1
		Do j=1,3
			v = data(ptr+2+j)
			if (v .lt. minv(j)) minv(j) = v
			if (v .gt. maxv(j)) maxv(j) = v
		Enddo
	Enddo
C
C	Subtract out the means
C
	Do J=1,3
		mean(j) = (maxv(j) + minv(j))/2.0
	Enddo
	scale = maxv(1) - minv(1)
	Do I=1,count
		ptr = ((I-1)*6)+1
		Do j=1,3
			data(ptr+2+j) = (data(ptr+2+j) - mean(j))/scale
		Enddo
	Enddo
C
	return
	end

	Subroutine getRGBcol(image,col,jj)

	Implicit None

	Integer*4	jj,image(*),I,h

	Real*4		col(3)

	I = image(jj)

	h = '000000ff'X
	col(1) = float(iand(i,h))/255.0
	h = '0000ff00'X
	col(2) = float(iand(i,h))/(255.0**2.0)
	h = '00ff0000'X
	col(3) = float(iand(i,h))/(255.0**3.0)

	return
	End
