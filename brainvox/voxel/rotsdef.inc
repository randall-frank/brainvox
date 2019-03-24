C
	Structure	/rotsrec/
C
C	rots(1-3) = volume rx,ry,rz
C	rots(4-6) = cutting plane rx,ry,rz
C	rots(7) = cutting plane Dist
C
		Integer*4	rots(7) 
C
C	Data for MP_FIT
C
		Real*4		sthickness
		Real*4		ipixel
		Real*4		slew(2)  !calib,additional
		Real*4		tilt(2)
		Real*4		x_off
		Real*4		xshift
		Integer*4	n_slices(2)  !num,view
		Character*80	outname
		Real*4		fpnorm(3)
		Real*4		fpxaxis(3)
		Real*4		fpyaxis(3)
		Real*4		fpcenter(3)
		Real*4		fppoints(4,3) !RF,LF,RR,LR
		Real*4		uprots(3)
		Real*4		uptrans(3)
		Real*4		upcenter(3)
		Real*4		upnorm(3)
		Real*4		upxaxis(3)
		Real*4		upyaxis(3)
	End Structure
C
