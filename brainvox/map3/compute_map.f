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
C    MODULE NAME    : compute_map
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox_map3
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 1 Mar 96        
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routines to compute a MAP-3 (to texture RAM) from a
C			setup.
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
	Subroutine compute_map3(map)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
        Include '../libs/vt_inc.f'
        Include '../libs/vt_protos.f'
        Include 'map3uif.inc'
        Include 'map3.inc'

	Record /MAP3/   map
C
	Integer*4	i,j,k,l,m
	Integer*4	fp,t0,maxnum,hist(256)
	Integer*4	T
	Character*256	cmd
	Character*200	tmp1,tmp2,tmpfile,tmp3,tmp4
C
	Integer*4	strlen,bv_getpid,bv_unlink,bv_time,bv_os
	External	strlen,bv_getpid,bv_unlink,bv_time,bv_os
C
C	Show progress
C
	maxnum = map.num
	Call draw_status_bar(float(0)/float(maxnum),
     +		"Computing MAP-3...}")
C
C	Create the temporary file for input
C
	Call bv_gettempdir(tmp1)
	Write(tmpfile,1) tmp1(1:strlen(tmp1)),bv_getpid()
1	Format(A,"BVOX_MAP3_",I8.8)
2	Format(A)
C
	Open(66,file=tmpfile,form='formatted',status='new',err=910)
C
C	Build the input file
C
	Do i=1,map.num
C
C	For each volume
C
C	New tagfile
C
		write(66,2,err=900) "NEWTAGFILE"
		cmd = map.dir(i)
		j=strlen(cmd)
		cmd = cmd(1:j)//"/Trace_Slicing.tag"//char(0)
		write(66,2,err=900) cmd(1:index(cmd,char(0))-1)
C
C	ROIname
C
20	Format(A,F8.4,A)
30	Format(A,F8.4," ARB",A)
		tmp1 = map.roi(i)
		j = strlen(tmp1)
		If (map.arb_mode(i) .eq. 1) Then
			write(cmd,30) tmp1(1:j),map.value(i),char(0)
		Else
			write(cmd,20) tmp1(1:j),map.value(i),char(0)
		Endif
		write(66,2,err=900) cmd(1:index(cmd,char(0))-1)
C
	Enddo
C
	Close(66)
C
C	Open the painting application
C
C	Select output destinations
C
5	Format("BV_SHMEM",I12.12)
	write(tmp1,5) shm_texture
C
C	N image option flag
C
	If (map.n_image .eq. 1) Then
		tmp2 = "-N"
	Else
		tmp2 = " "
	Endif
C
C	Size weighting option flag
C
	If (map.size_weight .eq. sw_none) Then
		tmp3 = " "
	Else
C
C	Find MRI volume size
C
		Do j=1,256
			hist(j) = 0
		Enddo
		j = imagesize*imagesize*
     +			(realimages+((realimages-1)*interpimages))
		Call data_histo(%val(images_addr),j,hist)
		T=0
		Do j=2,256
			T = T + float(hist(j))
		Enddo
C
C	Inverse
C
		If (map.size_weight .eq. sw_invlin) Then
			write(tmp3,81) T
		Else
			write(tmp3,80) T
		Endif
C
C	build option text
C
80	Format("-S",I12.12)
81	Format("-S-",I12.12)
C
	Endif
C
C	Build the command
C
      
        tmp4 = "brainvox_arb_roi_paint"
        Call bv_prepend_path(tmp4)
	i = strlen(tmp1)
	j = strlen(tmp2)
	k = strlen(tmpfile)
	l = strlen(tmp3)
	m = strlen(tmp4)
	If (bv_os() .eq. 0) then
	   write(cmd,11) tmpfile(1:k),tmp4(1:m),map.exp,map.offset,tmp1(1:i),
     +		tmp2(1:j),tmp3(1:l),char(0)
11	Format("more ",A," | ",A," _patient junk.tag ",
     +		F9.4," ",F9.4," ",A," ",A," ",A," ",A)
	Else
	   write(cmd,10) tmpfile(1:k),tmp4(1:m),map.exp,map.offset,tmp1(1:i),
     +		tmp2(1:j),tmp3(1:l),char(0)
10	Format("cat ",A," | ",A," _patient junk.tag ",
     +		F9.4," ",F9.4," ",A," ",A," ",A," ",A)
	Endif
C
	write(*,*) "MAP3 command:",cmd(1:index(cmd,char(0)))
C
	Call p_open(cmd,"r"//char(0),fp)
	If (fp .eq. 0) Then
		Call bv_error("Unable to start MAP-3 generation.}",0)
		goto 920
	Endif
C
C	read back the results
C
	k = 1
	i = 1
	Do while (i .ne. 0)
		i = 255
		Call p_read_line(cmd,i,fp)
		If (i .gt. 0) Then
			j = index(cmd,"BV_PROGRESS")
			If (j .ge. 1) Then
				read(cmd(j+11:i),*) k,maxnum
			Else
				If (i .gt. 0) write(*,2) cmd(1:i)
			Endif
			If (bv_time()-t0 .gt. 5) Then
				t0 = bv_time()
				j = 1
			Endif
			If (j .ge. 1) Then
	                	Call draw_status_bar(float(k-1)/
     +				   float(maxnum),"Computing MAP-3...}")
			Endif
		Endif

	Enddo
C
C	Done (close the painting application)
C
	Call p_close(fp)
C
	Call draw_status_bar(float(maxnum)/float(maxnum),
     +                  "Computing MAP-3...}")
C
	goto 920
C
C	Errors
C
900	continue
C
	Close(66)
C
910	continue  
C
	Call bv_error("Unable to generate MAP-3 input file.}",0)
C
920	continue
C
	j = bv_unlink(tmpfile(1:strlen(tmpfile))//char(0))
C
	Return
	End
