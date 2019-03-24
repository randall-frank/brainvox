C *************************************************************************
C 									  *
C  		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
C 									  *
C   These coded instructions, statements, and computer programs  contain  *
C   unpublished  proprietary  information of Silicon Graphics, Inc., and  *
C   are protected by Federal copyright law.  They  may  not be disclosed  *
C   to  third  parties  or copied or duplicated in any form, in whole or  *
C   in part, without the prior written consent of Silicon Graphics, Inc.  *
C 									  *
C ************************************************************************


C  definitions for returned values of get* routines 


C  bits in value returned by getbuffer 

       integer*4   BCKBUF
       parameter ( BCKBUF = 1 ) 	
       integer*4   FRNTBU
       parameter ( FRNTBU = 2 ) 	
       integer*4   DRAWZB
       parameter ( DRAWZB = 4 ) 	

C  values returned by getdisplaymode 

       integer*4   DMRGB
       parameter ( DMRGB = 0 ) 	
       integer*4   DMSING
       parameter ( DMSING = 1 ) 	
       integer*4   DMDOUB
       parameter ( DMDOUB = 2 ) 	
       integer*4   DMRGBD
       parameter ( DMRGBD = 5 ) 	

C  values returned by getmonitor 

       integer*4   HZ30
       parameter ( HZ30 = 0 ) 	
       integer*4   HZ60
       parameter ( HZ60 = 1 ) 	
       integer*4   NTSC
       parameter ( NTSC = 2 ) 	
       integer*4   HDTV
       parameter ( HDTV = 3 ) 	
       integer*4   VGA
       parameter ( VGA 	= 4 ) 	
       integer*4   IRIS3K
       parameter ( IRIS3K = 5 ) 	
       integer*4   PR60
       parameter ( PR60 = 6 ) 	
       integer*4   PAL
       parameter ( PAL 	= 9 ) 	
       integer*4   HZ30SG
       parameter ( HZ30SG = 11 ) 	
       integer*4   A343
       parameter ( A343 = 14 ) 	
       integer*4   STRREC
       parameter ( STRREC = 15 ) 	
       integer*4   VOF0
       parameter ( VOF0 = 16 ) 	
       integer*4   VOF1
       parameter ( VOF1 = 17 ) 	
       integer*4   VOF2
       parameter ( VOF2 = 18 ) 	
       integer*4   VOF3
       parameter ( VOF3 = 19 ) 	
       integer*4   SGI0
       parameter ( SGI0 = 20 ) 	
       integer*4   SGI1
       parameter ( SGI1 = 21 ) 	
       integer*4   SGI2
       parameter ( SGI2 = 22 ) 	
       integer*4   HZ72
       parameter ( HZ72 = 23 ) 	
       integer*4   HZ50
       parameter ( HZ50 = 24 ) 	
       integer*4   HZ76
       parameter ( HZ76 = 25 ) 	
       integer*4   HZ70
       parameter ( HZ70 = 26 ) 	
       integer*4   STRBOT
       parameter ( STRBOT = 27 ) 	
       integer*4   STRTOP
       parameter ( STRTOP = 28 ) 	
       integer*4   PRESEN
       parameter ( PRESEN = 29 ) 	
       integer*4   HZ48
       parameter ( HZ48 = 30 ) 	
       integer*4   HZ75
       parameter ( HZ75 = 31 ) 	
       integer*4   SQRREC
       parameter ( SQRREC = 32 ) 	
       integer*4   SQRBOT
       parameter ( SQRBOT = 33 ) 	
       integer*4   SQRTOP
       parameter ( SQRTOP = 34 ) 	
       integer*4   SGIVID
       parameter ( SGIVID = 999 ) 	
C  VOF IDs from 1000 on are reserved for custom customer codes 

C  values returned by getvideo(GLVIDEO_REG) 

       integer*4   GLVIDE
       parameter ( GLVIDE = 8388608 ) 	
       integer*4   GLVGEN
       parameter ( GLVGEN = 1 ) 	
       integer*4   GLVUNB
       parameter ( GLVUNB = 2 ) 	
       integer*4   GLVSRE
       parameter ( GLVSRE = 4 ) 	
       integer*4   GLVSGR
       parameter ( GLVSGR = 8 ) 	
       integer*4   GLVSBL
       parameter ( GLVSBL = 16 ) 	
       integer*4   GLVSAL
       parameter ( GLVSAL = 32 ) 	
       integer*4   GLVTTL
       parameter ( GLVTTL = 128 ) 	
					
       integer*4   GLVTSC
       parameter ( GLVTSC = GLVTTL ) 	
       integer*4   GLVGRE
       parameter ( GLVGRE = 256 ) 	
					

C  individual hit bits returned by gethitcode 

       integer*4   LEFTPL
       parameter ( LEFTPL = 1 ) 	
       integer*4   RIGHTP
       parameter ( RIGHTP = 2 ) 	
       integer*4   BOTTOM
       parameter ( BOTTOM = 4 ) 	
       integer*4   TOPPLA
       parameter ( TOPPLA = 8 ) 	
       integer*4   NEARPL
       parameter ( NEARPL = 16 ) 	
       integer*4   FARPLA
       parameter ( FARPLA = 32 ) 	


C  obsolete symbols 


       integer*4   NOBUFF
       parameter ( NOBUFF = 0 ) 	
       integer*4   BOTHBU
       parameter ( BOTHBU = 3 ) 	

       integer*4   DMINTS
       parameter ( DMINTS = 3 ) 	
       integer*4   DMINTD
       parameter ( DMINTD = 4 ) 	

       integer*4   MONSPE
       parameter ( MONSPE = 32 ) 	
       integer*4   MONA
       parameter ( MONA = 5 ) 	
       integer*4   MONB
       parameter ( MONB = 6 ) 	
       integer*4   MONC
       parameter ( MONC = 7 ) 	
       integer*4   MOND
       parameter ( MOND = 8 ) 	

       integer*4   MONALL
       parameter ( MONALL = 12 ) 	
       integer*4   MONGEN
       parameter ( MONGEN = 13 ) 	

       integer*4   CMAPMU
       parameter ( CMAPMU = 0 ) 	
       integer*4   CMAPON
       parameter ( CMAPON = 1 ) 	

