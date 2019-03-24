C
C	Structure for the Disk Space conversion
C
	Structure	/disk_conv/
		Integer*4	valid
		Integer*4	display  !1=lines 2=plane
		Real*4		points(3,3)  !Center,p1,p2
		Real*4		xaxis(3)  ! vectors
		Real*4		yaxis(3)
		Real*4		zaxis(3)
	End Structure
C
	Record	/disk_conv/	dspace
C
	Common /matthoward/dspace
C
	Integer*4	PT_C
	Parameter	(PT_C = 1)
	Integer*4	PT_P1
	Parameter	(PT_P1 = 2)
	Integer*4	PT_P2
	Parameter	(PT_P2 = 3)
C
	Real*4	D_RADIUS
	Parameter (D_RADIUS = 26.9)   ! 26.9mm radius  from 2.125inch dia
	Integer*4	DI_RADIUS
	Parameter (DI_RADIUS = 27) 
C
C       useful defines
C
	Real*4	TOLERANCE
	Parameter (TOLERANCE = 5.0)
C
