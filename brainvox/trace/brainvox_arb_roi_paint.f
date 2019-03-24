C    ======================================================================   
C    THIS SOFTWARE IS THE PROPERTY OF:                                          
C         The Image Analysis Facility                                     
C         133 MRC     
C         The University of Iowa, Iowa City, Iowa                               
C    ======================================================================     
C                                                                               
C    -- OVERVIEW --------------                                                 
C                                                                               
C    MODULE NAME    : brainvox_arb_roi_paint
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
C    DATE           : 19 Sept 95      
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    : Routine to support weighted arbitrary roi filling
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :  prog _patient tagfile.tag exp offset volume
C    INPUTS         :  _patient (ip,is,nslices)
C		       tagfile.tag (ARB settings+roinames)
C		       exp,offset (function params)
C		       
C		       stdin: Lines of the format "roiname value {ARB}"
C			one for each roi to be added to the volume
C
C    OUTPUTS        :  volume (8 bit weighted average volume)
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Program brainvox_arb_roi_paint

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include '../libs/vt_protos.f'
	Include	'tr_inc.f'
	Include 'vt_arb_inc.f'
	Include 'traceuif.inc'
C
C	Buffer the inputs
C
	Structure	/input_map/
		Integer*4	arbmode
		Real*4		value
		Character*256	roiname
		Character*256	tagfilename
	End Structure

	Integer*4		max_inputs,num_inputs
	Parameter		(max_inputs = 300)

	Record/input_map/	inputs(max_inputs)

	Integer*4	I,iargc,j,k
	Character*200	filename,outtemp,roi
	logical		found,error,write_N,use_floats,weight_size

	Character*256	orig_cwd,curtagfile

	Real*4		Param_exp,Param_off,value,T,squf,fW_val,fN_val
	Real*4		weight_size_max,tmpf

	Integer*4	n_images,hist(256)
	Integer*4	tex8_ptr,N_vol_ptr,W_vol_ptr,shmem_id
	Integer*4	npasses,tmp_vol_ptr,pass,dz
	Real*4		ip,is

	Integer*2	W_val,N_val
C
	Character*200	text
	Integer*4	bv_chdir,bv_getcwd,strlen
C
	external	iargc,bv_chdir,bv_getcwd,strlen
C
	write_N = .false.
	use_floats = .false.
	weight_size = .false.
	npasses = 1
C
C	Use line buffering for popen feedback
C
C#ifndef IRIX4
C	Call setlinebuf(6)
C#endif
C
C	Guess the system path
C
	Call getarg(0,text)
	Call bv_set_app_name(text)
C
C	get command line
C
	If (iargc() .lt. 5) Then
		Call getarg(0,text)
		i = strlen(text)
        	write(*, 1)"9 Feb 96",text(1:i)
1       Format("(",A,")",/,"Usage:",A,
     +		" _patient arb.tag exp offset outtemp [opts]")
		write(*, 1004)
1004	Format(/,"Options: -N     Compute an N map.",/,
     +           "         -S(n)  Region size weight the map",
     +           " relative to (n) voxels.")
		write(*, 1001)
		write(*, 1002)
		write(*, 1003)
1001	Format(/,"        Input is lines of the form 'roiname value",
     +		" {ARB}' where")
1002	Format("        the ARB flag selects arbitrary tracing mode.",
     +		"  Value should",/,"        be in the range 0.0-1.0")
1003	Format("        A line containing 'NEWTAGFILE' should be ",
     +		"followed by a line containing",
     +       /,"        the name of a new tagfile which will be read",
     +		" for the following ROIs")

		Call bv_exit(1)
	Endif
C
C	Read _patient file
C
	Call getarg_nq(1,text)
	Call read_patient(text,error)
	If (error) Then
		write(*, 2)"Unable to read _patient file"
2	Format("Error: ",A)
3	Format("Error: ",A,A)
		Call bv_exit(1)
	Endif
	If (realimages .eq. 0) Then
		imagename = "../"//imagename
		Call image_check(imagename,realimages,imagesize,dz,ip,is)
		If (realimages .ne. 0) Then

	write(*, 100)realimages,volumedepth,imagesize,imagesize,imagename
100     Format(" Found ",I5,"x",I1," byte images of size ",I5,"x",I5,
     +          " using template:",/,A200)
C
C	Need to manually compute squeezefactor and interpimages!!!
C	because of BAD first image search call
C
        squf = 10.0
        interpimages = -1
        do while ((squf .gt. 1.00).and.(interpimages .lt. 20))
                interpimages = interpimages +1
                squf = (float(realimages)*interslice)/
     +    (float(realimages + (interpimages*(realimages-1)))*interpixel)
        Enddo
C
        squeezefactor = squf
C
		Endif
	Endif
	If (realimages .eq. 0) Then
		write(*, 2)"Unable to determine the number of slices to use"
		Call bv_exit(1)
	Endif
C
C	Read the tag file
C
	i = bv_getcwd(orig_cwd,%val(255))
	If (i .eq. 0) Then
		write(*,2)"Unable to determine the current working directory"
		Call bv_exit(1)
	Endif
C
	Call getarg_nq(2,text)
	curtagfile = text
	Call Read_New_Tagfile(text,i,orig_cwd)
	if (i .ne. 0) Then
		write(*, 3)"Unable to read the tagfile:",
     +			text(1:strlen(text))
	Endif
C
C	Get the function params
C
	Param_exp = -10.0
	Param_off = -0.5
	Call getarg(3,text)
	Read(text,*,err=5,end=5) Param_exp
C10	Format(*)
5	Call getarg(4,text)
	Read(text,*,err=6,end=6) Param_off
6	Continue
	write(*,*) "Parameters:",Param_exp,Param_off
C
C	Get the output volume name
C
	Call getarg(5,outtemp)
C
C	Read options
C
	Do i=6,iargc()
		Call getarg(i,text)
		If (text(1:2) .eq. "-N") Then
			write(*, *)"Computing an N image."
			write_N = .true.
		Else if (text(1:2) .eq. "-S") Then
			weight_size = .true.
			weight_size_max = 1.0
			read(text(3:),*,err=4,end=4) weight_size_max
4			If (weight_size_max .eq. 0.0) weight_size_max=1.0
			write(*, *)"Compute size weighted map (",
     +				weight_size_max,")"
		Else
			j = strlen(text)
			write(*, 3)"Unknown option:",text(1:j)
			Call bv_exit(1)
		Endif
	Enddo
C
C	check for use of floats
C
	If (weight_size .or. (Param_exp .ne. 0) .or. 
     +		(Param_off .ne. 0)) Then
C
		use_floats = .true.
		npasses = 2
		write(*, *)"Using floating point representation."
		write(*, *)"Number of passes:",npasses
	Endif
C
C	can the dataset be evenly divided?
C
	If (mod(realimages,npasses) .ne. 0) Then
	write(*,2)"The number of realimages must be evenly divisible by the"
     +		//" number of passes required."
		Call bv_exit(1)
	Endif
C
C	Allocate memory
C
        n_images = realimages+((realimages - 1) * interpimages)	
C
	i = realimages*2   ! 16 bit ints
	If (use_floats) i = (realimages/npasses)*4  ! floats for weighted
C
	call fmalloc(W_vol_ptr,imagesize*imagesize*i)
	call fmalloc(N_vol_ptr,imagesize*imagesize*i)
C
	If (use_floats) Then
C
C	buffer the output volume (without interpolations)
C
		call fmalloc(tmp_vol_ptr,imagesize*imagesize*realimages)
	Endif
C
C	Is the destination a shared memory volume???
C
	If (outtemp(1:8) .eq. "BV_SHMEM") Then
		Read(outtemp(9:),*) shmem_id 
		write(*, *)"Using Brainvox shared memory ID:",shmem_id
		Call attach_shared_mem(%val(shmem_id),tex8_ptr)
	Else
		shmem_id = -1
		call fmalloc(tex8_ptr,imagesize*imagesize*n_images)
	Endif
C
C	check pointers
C
	If ((tex8_ptr .eq. 0).or.(W_vol_ptr .eq. 0).or.
     +		(N_vol_ptr .eq. 0)) Then
		write(*, 2)"Unable to allocate enough memory"
		Call bv_exit(1)
	Endif
	If (use_floats .and. (tmp_vol_ptr .eq. 0)) Then
		write(*, 2)"Unable to allocate enough memory"
		Call bv_exit(1)
	Endif
C
C	Just in case
C
	texture_addr = tex8_ptr
	images_addr = tex8_ptr
C
C       init the vset (both 8 bit volumes) to tex8_ptr
C
        Call    vl_init(myset)
        Call    vl_data(%val(imagesize),%val(imagesize),%val(n_images),
     +                  %val(tex8_ptr),myset)
        Call    vl_aux_data(%val(1),%val(tex8_ptr),myset)
C
C       set the squeeze factor
C
        If (realimages .gt. 2) then
                squeezefactor = (float(realimages)*interslice)/
     +    (float(realimages + (interpimages*(realimages-1)))*interpixel)

        	Call vl_setsqu(squeezefactor,myset)
        	Call vl_setinterp(%val(interpimages),myset)
        Endif
C
C	collect the inputs
C
	num_inputs = 0
C
C	Read the stdin lines
C
	write(*, 40)
40	Format("Reading roiname/value/ARB inputs:")
C
	Do While(1)
C
C	Get the line
C
200		Do i=1,200
			text(i:i) = " "
		Enddo
		Read(*,20,err=7,end=7) text
20		Format(A)
		If (index(text,"NEWTAGFILE") .ne. 0) Then

			Read(*,20,err=7,end=7) text
			curtagfile = text
		Else 
C
C	Parse into "roiname value {flag}"
C
			j = 0
			i = 1
			Do while((text(i:i) .lt. '!').and.(i .lt. 200))
				i = i + 1
			Enddo
			If (i .gt. 200) Then
				j = -1
				goto 8
			Endif
			j = i
			Do while((text(i:i) .ge. '!').and.(i .lt. 200))
				i = i + 1
			Enddo
			If (i .gt. 200) Then
				j = -1
				goto 8
			Endif
			roi = text(j:i-1)//" "

			Read(text(i:),*,err=8,end=8) value
			If (index(text(i:),"ARB") .ge. 1) Then
				arb_mode = 1
			Else
				arb_mode = 0
			Endif
			If (j .eq. -1) Then
8				write(*, *)"Invalid input:",text
				goto 200
			Endif
C
C	Save off the current input
C
			num_inputs = num_inputs + 1
			If (num_inputs .gt. max_inputs) Then
	write(*, *)"Maximum number of input ROIs (300) exceeded. ",
     +		"Some ROIs will be skipped."
				num_inputs = max_inputs
			Else
				inputs(num_inputs).roiname=roi
				inputs(num_inputs).tagfilename=curtagfile
				inputs(num_inputs).arbmode=arb_mode
				inputs(num_inputs).value=value
			Endif
		Endif
C
C	Parse next input line
C
	Enddo
C
7	Continue
C
C	Compute the MAP-3
C
	write(*, *)"Working on ",num_inputs," ROIs"
	Call flush(6)
C
C	For each pass
C
	Do pass=1,npasses
C
C	zero the weight and n volumes
C
	If (use_floats) Then
		i = (realimages/npasses)*imagesize*imagesize
C
C	clear the volumes
C
		Call fclear(%val(N_vol_ptr),i,0.0)
		If (.not. write_N) Then
			Call fclear(%val(W_vol_ptr),i,0.0)
		Endif
	Else
		i = realimages*2*imagesize*imagesize
		Call memclear(%val(N_vol_ptr),i)
		If (.not. write_N) Then
			Call memclear(%val(W_vol_ptr),i)
		Endif
	Endif

C
	Do i=1,num_inputs
C
C	Allow for feedback
C
		If (shmem_id .ne. -1) Then
			write(*, *) "BV_PROGRESS ",i+((pass-1)*num_inputs),
     +				num_inputs*npasses
		Endif
C
C	Read the tagfile
C
		text = inputs(i).tagfilename
		Call Read_New_Tagfile(text,j,orig_cwd)
		If (j .ne. 0) Then
			write(*, 3) "Unable to read the tagfile:",
     +				text(1:strlen(text))
		Endif
C
C	Get the inputs
C
		roi = inputs(i).roiname
		arb_mode =inputs(i).arbmode
		value = inputs(i).value
C
C	Calc the values N=weight  W=weight*value
C
		If ((Param_exp .eq. 0).and.(Param_off .eq. 0)) Then

			N_val = 255
			T = 1.0
			fN_val = T

		Else

			T = 0.1+0.9/(1.0+exp(Param_exp*(value+Param_off)))
			If (T .lt. 0.0) T = 0.0
			If (T .gt. 1.0) T = 1.0
			N_val = T*255.0  
			If (N_val .lt. 1) N_val = 1 ! [1-255]
			fN_val = T

		Endif
		T = T*value
		If (T .lt. 0.0) T = 0.0
		If (T .gt. 1.0) T = 1.0
		fW_val = T
		W_val = T*255.0  ! [0-255]
C
C	Select the proper arb_mode
C
		j = strlen(roi)
		If (arb_mode .ne. 0) Then
		    Call roiname_to_arbmode(roi(1:j),arb_mode)
		    If (arb_mode .ne. 1) Then
			write(*, *) "Using custom arb mode for ",roi(1:j)
		    Else
			write(*, *) "Using general arb mode for ",roi(1:j)
		    Endif
		Else
			write(*, *) "Using slice mode for ",roi(1:j)
		Endif
C
		Call flush(6)
C
C	Paint this ROI into the 8bit texture volume
C
		Call memclear(%val(tex8_ptr),imagesize*imagesize
     +			  *n_images)
		Call offline_texture_op(IPC_TEX_CL_PAINT,1,roi)
C
C	Take care of weighting by size (adjust weights)
C
		If (weight_size) Then
C
C	Get the volume of the region mask (tex8_ptr)
C
			Do j=1,256
				hist(j) = 0
			Enddo
			j = imagesize*imagesize*n_images
			Call data_histo(%val(tex8_ptr),j,hist)
			T = 0
			Do j=2,256
				T = T + float(hist(j))
			Enddo
			tmpf = T
C
C	Ratio from 0.0 to weight_size_max
C	
			T = T/abs(weight_size_max)
			If (T .gt. 1.0) T = 1.0
C
C	If inverse mapping (size<0) then invert the weight
C
			If (weight_size_max .lt. 0) Then
				T = 1.0 - T
			Endif
C
			write(*, 767)tmpf,weight_size_max,T
767	Format("Rgn vol:",F15.1," Ref vol:",F15.1," Size weight:",F10.5)
C
C	Apply the size weight
C
			fN_val = fN_val * T
			fW_val = fW_val * T
C
		Endif
C
C	Report final weights
C
		write(*, 766)value,fW_val,fN_val
766	Format("Rgn value:",F10.5," Adj value:",F10.5,
     +		" Adj N value:",F10.5)
C
C	Increment the N volume
C
		If (use_floats) Then
			j = (pass-1)*(realimages/npasses)
			Call Inc_fVolume(%val(tex8_ptr),%val(N_vol_ptr),
     +		  		fN_val,imagesize,imagesize,
     +				realimages/npasses,interpimages,j)
		Else 
			Call Inc_Volume(%val(tex8_ptr),%val(N_vol_ptr),
     +		  		N_val,imagesize,imagesize,realimages,
     +				interpimages)
		Endif
C
C	Increment the W volume (if needed)
C
		If (.not. write_N) Then
			If (use_floats) Then
				j = (pass-1)*(realimages/npasses)
				Call Inc_fVolume(%val(tex8_ptr),
     +					%val(W_vol_ptr),fW_val,imagesize,
     +					imagesize,realimages/npasses,
     +					interpimages,j)
			Else
				Call Inc_Volume(%val(tex8_ptr),
     +					%val(W_vol_ptr),W_val,imagesize,
     +					imagesize,realimages,interpimages)
			Endif
		Endif
C
	Enddo
C
C	Perform the division
C
	write(*, 30)
30	Format("Computing the average volume...")
C
C	Compute the average volume (weight/n)
C
	If (use_floats) Then
C
C	Non-weighted (integer volumes)
C	N or W volume
C
		If (write_N) then
C
C	N
C
			j = (pass-1)*(realimages/npasses)
			Call Copy_fVolume(%val(N_vol_ptr),%val(tmp_vol_ptr),
     +				imagesize,imagesize,realimages/npasses,j)
		Else 
C
C	W
C
			j = (pass-1)*(realimages/npasses)
			Call Div_fVolumes(%val(W_vol_ptr),%val(N_vol_ptr),
     +				%val(tmp_vol_ptr),imagesize,imagesize,
     +				realimages/npasses,j)
		Endif
C
	Else
C
C	Non-weighted (integer volumes)
C	N or W volume
C
		If (write_N) then
C
C	N
C
			Call Copy_Volume(%val(N_vol_ptr),%val(tex8_ptr),
     +				imagesize,imagesize,realimages,interpimages)
		Else 
C
C	W
C
			Call Div_Volumes(%val(W_vol_ptr),%val(N_vol_ptr),
     +				%val(tex8_ptr),imagesize,imagesize,
     +				realimages,interpimages)
		Endif
C
	Endif
C
C	Next pass
C
	Enddo
C
C	If in multi-pass mode, copy the tmp data back into texture memory
C
	If (npasses .gt. 1) Then
C
C	Copy tmp_vol_ptr data to tex8_ptr
C
	    Do i=1,realimages
		j = tex8_ptr + (i-1)*imagesize*imagesize*(interpimages+1)
		k = tmp_vol_ptr + (i-1)*imagesize*imagesize
		Call memcopy(%val(j),%val(k),imagesize*imagesize)
	    Enddo
C
	Endif
C
C	Write the result to disk
C
	i = bv_chdir(orig_cwd)
	If (i .ne. 0) Then
		write(*, 2)"Unable to change the current working directory"
		Call bv_exit(1)
	Endif
C
C	Save to disk 
C
	If (shmem_id .eq. -1) Then
		Call volume_saveimages(outtemp,error,0,1)
	Else
C
C	or to memory Interpolate texture volume!!!
C
		Call interp_volume(tex8_ptr,imagesize,imagesize,
     +			realimages,interpimages)
	Endif
C
C	Clean up any mess
C
	If (use_floats) Then
		Call ffree(tmp_vol_ptr)
	Endif
	If (shmem_id .eq. -1) Then
		Call ffree(tex8_ptr)
	Else
		Call detach_shared_mem(%val(tex8_ptr),i)
	Endif
	Call ffree(W_vol_ptr)
	Call ffree(N_vol_ptr)

	Call bv_exit(0)
C
	End
C
C	Increment routine (add if the mask is non-zero)
C
	Subroutine Inc_Volume(mask,W_vol,W_val,dx,dy,
     +		realimages,interpimages)

	Implicit None

	Byte		mask(*)
	Integer*2	W_vol(*)
	Integer*2	W_val
	Integer*4	dx,dy,realimages,interpimages

	Integer*4	i,size,p1,p2,j

	size = dx*dy

	Do i=1,realimages
		p1 = 1 + (i-1)*size
		p2 = 1 + (i-1)*(size*(interpimages+1)) 
		Do j=1,size
			If (mask(p2) .ne. 0) Then
				W_vol(p1) = W_vol(p1) + W_val
			Endif
			p1 = p1 + 1
			p2 = p2 + 1
		Enddo
	Enddo

	Return
	End
C
C	Real Increment routine (add if the mask is non-zero)
C
	Subroutine Inc_fVolume(mask,W_vol,W_val,dx,dy,
     +		realimages,interpimages,soffset)

	Implicit None

	Byte		mask(*)
	Real*4		W_vol(*)
	Real*4		W_val
	Integer*4	dx,dy,realimages,interpimages,soffset

	Integer*4	i,size,p1,p2,j

	size = dx*dy

	Do i=1,realimages
		p1 = 1 + (i-1)*size
		p2 = 1 + (i-1)*(size*(interpimages+1))
C
C	Pass offset
C
		p2 = p2 + soffset*(size*(interpimages+1))
		Do j=1,size
			If (mask(p2) .ne. 0) Then
				W_vol(p1) = W_vol(p1) + W_val
			Endif
			p1 = p1 + 1
			p2 = p2 + 1
		Enddo
	Enddo

	Return
	End
C
C	Divide routine
C
	Subroutine Div_Volumes(W_vol,N_vol,out_vol,dx,dy,z,interp)

	Implicit None

	Integer*2	W_vol(*),N_vol(*)
	Byte		out_vol(*)
	Integer*4	dx,dy,z,interp
	Real*4		W,N,V
	Integer*4	i,j,k,size,p1,p2

	size=dx*dy
	Do k=1,z
		p1 = 1 + (k-1)*size
		p2 = 1 + (k-1)*(size*(interp+1))
		Do i=1,size
			If (N_vol(p1) .eq. 0) Then
				out_vol(p2) = 0
			Else 
				W = W_vol(p1)
				N = N_vol(p1)
				V = (W/N)*255.0
				If (V .gt. 255) V = 255
				If (V .le. 1) V = 1
				j = V
				If (j .ge. 128) j=j-256
				out_vol(p2) = j
			Endif
			p1 = p1 + 1
			p2 = p2 + 1
		Enddo
	Enddo

	Return
	End
C
C	Divide routine (reals)
C
	Subroutine Div_fVolumes(W_vol,N_vol,out_vol,dx,dy,z,soffset)

	Implicit None

	Real*4		W_vol(*),N_vol(*)
	Byte		out_vol(*)
	Integer*4	dx,dy,z,soffset
	Real*4		W,N,V
	Integer*4	i,j,k,size,p1,p2
 
	size=dx*dy
	Do k=1,z
		p1 = 1 + (k-1)*size
C
C	Pass offset
C
		p2 = 1 + (k-1)*size+soffset*size
		Do i=1,size
			If (N_vol(p1) .le. 0.0) Then
				out_vol(p2) = 0
			Else 
				W = W_vol(p1)
				N = N_vol(p1)
				V = (W/N)*255.0
				If (V .gt. 255.0) V = 255.0
				If (V .le. 1.0) V = 1.0
				j = V
				If (j .ge. 128) j=j-256
				out_vol(p2) = j
			Endif
			p1 = p1 + 1
			p2 = p2 + 1
		Enddo
	Enddo

	Return
	End
C
C	Copy routine
C
	Subroutine Copy_Volume(N_vol,out_vol,dx,dy,z,interp)

	Implicit None

	Integer*2	N_vol(*)
	Byte		out_vol(*)
	Integer*4	dx,dy,z,interp
	Real*4		W,N,V
	Integer*4	i,j,k,size,p1,p2

	size=dx*dy
	Do k=1,z
		p1 = 1 + (k-1)*size
		p2 = 1 + (k-1)*(size*(interp+1))
		Do i=1,size
			If (N_vol(p1) .eq. 0) Then
				out_vol(p2) = 0
			Else 
				V = N_vol(p1)/255
				If (V .gt. 255) V = 255
				If (V .le. 0) V = 1
				j = V
				If (j .ge. 128) j=j-256
				out_vol(p2) = j
			Endif
			p1 = p1 + 1
			p2 = p2 + 1
		Enddo
	Enddo

	Return
	End
C
C	Copy routine (reals)
C
	Subroutine Copy_fVolume(N_vol,out_vol,dx,dy,z,soffset)

	Implicit None

	Real*4		N_vol(*)
	Byte		out_vol(*)
	Integer*4	dx,dy,z,soffset
	Real*4		W,N,V
	Integer*4	i,j,k,size,p1,p2

	size=dx*dy
	Do k=1,z
		p1 = 1 + (k-1)*size
C
C	Pass offset
C
		p2 = 1 + (k-1)*size + soffset*size
		Do i=1,size
			If (N_vol(p1) .le. 0.0) Then
				out_vol(p2) = 0
			Else 
				V = N_vol(p1)
				If (V .gt. 255.0) V = 255.0
				If (V .le. 1.0) V = 1.0
				j = V
				If (j .ge. 128) j=j-256
				out_vol(p2) = j
			Endif
			p1 = p1 + 1
			p2 = p2 + 1
		Enddo
	Enddo

	Return
	End
C
C	Routine to change the Tagfile (and the CWD)
C
	Subroutine	Read_New_Tagfile(in,err,orig_cwd)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include '../libs/vt_protos.f'
	Include	'tr_inc.f'
	Include 'vt_arb_inc.f'
	Include 'traceuif.inc'

	Integer*4	err,i,bv_chdir,j,k,strlen
	Character*(*)	in,orig_cwd
	Character*256	text,file,dir
	logical		error

	external	bv_chdir,strlen

	err = 0
C
C	Original CWD (allows local refs to work)
C
	i = bv_chdir(orig_cwd)
	if (i .ne. 0) Then
		err = 1
		Return
	Endif
C
C	Zero terminate and copy
C
	i = strlen(in)
	text = in(1:strlen(in))//char(0)
C
C	Break into two pieces
C
	j = 0
	k = 0
	Do While (k .lt. i)
		If ((text(k:k) .eq. '/').or.(text(k:k) .eq. '\')) Then
			j = k
		Endif
		k = k + 1
	Enddo
	If (j .gt. 0) Then
		dir = text(1:j)//char(0)
		file = text(j+1:)
C
C	CWD to proper directory
C
		write(*, 10)"Changing directory to:",dir(1:j)
		i=bv_chdir(dir)
		If (i .ne. 0) Then
			err = 1
			Return
		Endif
	Else
		file = text
	Endif
C
C	Read the Tagfile
C
	write(*, 10)"Reading tagfile:",file(1:index(file,char(0))-1)
	Call Tagfile_Arb_IO_file(file,IPC_READ,error)
	If (error) err = 1
C
10	Format(A,A)

	Return
	End
C
C	Interpolate the texture volume by pixel replication
C
	Subroutine interp_volume(addr,dx,dy,dz,interp)

	Implicit None

	Integer*4	dx,dy,dz,interp,addr
	Integer*4	i,j,isize,start,stepsize,iend,dest

	start = 0
	isize = (dx*dy)
	stepsize = (interp+1)*isize

	Do i=1,dz-1
C
C       interpolate the images
C
                iend = start + stepsize
                dest = start + isize
C
		Do j=1,interp
			Call memcopy(%val(addr+dest),
     +				%val(addr+start),isize)
			dest = dest + isize
		Enddo
C
C       bump start along
C
                start = start + stepsize

	Enddo


	Return
	End
C
C	routine to clear a floating point volume
C
	Subroutine fclear(vol,num,val)

	Implicit None

	Real*4	vol(*),val

	Integer*4	num,i

	Do i=1,num
		vol(i) = val
	Enddo
	End
