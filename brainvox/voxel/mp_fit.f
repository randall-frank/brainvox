	Program Brainvox_MP_FIT

	Implicit None

	Include '../libs/vt_inc.f'
	Include 'rotsdef.inc'

	Logical		error

	Integer*4	status,i,j,opts,dz
	Real*4		ip,is
	Character*120	tstr,airfile
	Character*200	progname,roi,rgn
	Real*8       	myset(1100)

	Record  /rotsrec/       rots

	Integer*4	iargc

	External 	iargc
C
C       Guess the system path
C
        Call getarg(0,tstr)
        Call bv_set_app_name(tstr)
C
C	Init ROTS struct
C
        rots.sthickness = 6.64
        rots.ipixel = 1.0
        rots.n_slices(1) = 15  ! count
        rots.n_slices(2) = 0  ! view
        rots.x_off = 0.0        ! xoffset
        rots.xshift= 0.0
        rots.outname = "sampled_%%%.pic "
        Do i= 1,2
                rots.slew(i) = 0
                rots.tilt(i) = 0
        Enddo
        Do i= 1,3
                rots.uptrans(i) = 0.0
                rots.uprots(i) = 0.0
                rots.fpnorm(i) = 0.0
                Do j=1,4
                        rots.fppoints(j,i) = 0.0
                Enddo
                rots.upxaxis(i) = 0.0
                rots.upyaxis(i) = 0.0
                rots.upnorm(i) = 0.0
        Enddo
        rots.upxaxis(1) = 1.0
        rots.upyaxis(2) = 1.0
        rots.upnorm(3) = 1.0
        rots.fpnorm(3) = 1.0
C
C	Get command line
C
	Call getarg(0,progname)
	If (iargc() .lt. 4) then
C
C	print version and program ID
C
	tstr = com_date
	i = 15
	Do while (tstr(i:i) .eq. ' ') 
		i = i - 1
	Enddo

	write(*, 1)tstr(1:i),
     +		progname(1:index(progname," ")-1)
1	Format("(",A,") Usage:",A," [-AIR(file)] _glasses _patient",
     +      " settings.MPSET intemp [outtemp]")

		write(*, 2)
2	Format(/,
     +	"This program resamples a Brainvox dataset according to",/,
     +	"a saved MP_FIT settings file. It should be run in a",/,
     +	"Brainvox original images directory.",/,/,
     +	"-AIR(file) - name of a mallocunmover restart file",/,
     +	"_glasses - name of the saved glasses fit file",/,
     +	"_patient - name of the _patient file for the volume",/,
     +	"settings.MPSET - MP_FIT saved setting file",/,
     +	"intemp - filename template for the input images",/,
     +	"outtemp - filename template for the input images",/,/,
     +	"If outtemp is not specified, it is taken from the MP_FIT",/,
     +	"saved settings file.  intemp should template a stack of",/,
     +	"images of the same size as those in the _patient file.",/,
     +	"Ouput images are placed in a MP_FIT.dir subdirectory",/,
     +	"along with any resampled pointsets and _tal space."
     +	)
		Call bv_exit(1)
	Endif
C
C	look for options
C
	airfile(1:1) = char(0)
	opts = 0
10	opts = opts + 1
	Call getarg_nq(opts,tstr)

	If (tstr(1:4) .eq. '-AIR') Then
		i = index(tstr,' ')
		airfile = tstr(5:i)
		airfile(i-4:i-4) = char(0)
	Endif
	If (tstr(1:1) .eq. '-') goto 10
C
C	Attempt to read the _patient file
C
	Call getarg_nq(opts+1,tstr)
	volumedepth = 1  ! This is currently not in the _patient file
	Call read_patient(tstr,error)
	If (error) then
		write(*,*)"Unable to read the file:",tstr(1:index(tstr," ")-1)
		Call bv_exit(1)
	Endif
C
C	get filename template
C
	Call getarg_nq(opts+3,imagename)
	Call image_check(imagename,realimages,imagesize,dz,ip,is)
	If (realimages .lt. 1) Then
		write(*,*)"No images could be found to match:",imagename
		Call bv_exit(1)
	Endif
	write(*,*)"Input images:",imagename(1:index(imagename," ")-1)
C
C       Compute the amount of memory required for the images
C
        i = imagesize*imagesize*(realimages +
     +                  ((realimages-1)*interpimages))
C
C	Allocate memory for the images
C
	call fmalloc(images_addr,i)
	If (images_addr .eq. 0) Then
		write(*,*)"Unable to allocate memory for the volume"
		Call bv_exit(1)
	Endif
C
C	Load the images
C
	Call background_loadimages(error)
	If (error) Then
		write(*, *)"Unable to load image volume"
		Call bv_exit(1)
	Endif
C
C       init the data set
C
        Call    vl_init(myset)
        i = realimages+((realimages - 1) * interpimages)
        Call    vl_data(%val(imagesize),%val(imagesize),%val(i),
     +                  %val(images_addr),myset)
	Call vl_setsqu(squeezefactor,myset)
	Call vl_setinterp(%val(interpimages),myset)
C
C	Read the _glasses file 
C
	Call getarg_nq(opts,tstr)
	Call glasses_io(IPC_READ,rots)
	Call calc_fp(rots)
	Call calc_up(rots,interpixel)
C
C	Read the MP_FIT settings file 
C
	Call getarg_nq(opts+2,tstr)
	Call MP_FIT_load_settings(tstr,rots)
	Call calc_up(rots,interpixel)
C
C	Get the output templatename 
C
	If (iargc() .ge. opts+4) Then
		Call getarg_nq(opts+4,tstr)
		rots.outname = tstr
	Endif
C
C	Load the AIR if any
C
	If (airfile(1:1) .ne. char(0)) Then
		write(*,*)"Loading AIR file:",airfile
		Call air_load(airfile,interpixel,interslice,
     +			imagesize,imagesize,
     +			realimages)
	Endif
C
C	Do the work
C
	Call save_planes(rots,myset,imagesize,interpixel,patientinfo,0)
C
C	Done
C
C	Call ffree(image_addr)

	Call bv_exit(0)

	End
C
C	Dummy routine to get the program to link
C
	Subroutine get_dlg_comp(filename,num,dlglen,dialog,status) 

	Return
	End
