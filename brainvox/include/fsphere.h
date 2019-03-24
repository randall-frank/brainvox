C *************************************************************************
C 									  *
C  		 Copyright (C) 1990, Silicon Graphics, Inc.		  *
C 									  *
C   These coded instructions, statements, and computer programs  contain  *
C   unpublished  proprietary  information of Silicon Graphics, Inc., and  *
C   are protected by Federal copyright law.  They  may  not be disclosed  *
C   to  third  parties  or copied or duplicated in any form, in whole or  *
C   in part, without the prior written consent of Silicon Graphics, Inc.  *
C 									  *
C ************************************************************************


C  sphere modes 
       integer*4   SPHTES
       parameter ( SPHTES = 0 ) 	
       integer*4   SPHDEP
       parameter ( SPHDEP = 1 ) 	
       integer*4   SPHPRI
       parameter ( SPHPRI = 2 ) 	
       integer*4   SPHHEM
       parameter ( SPHHEM = 3 ) 	
       integer*4   SPHORI
       parameter ( SPHORI = 4 ) 	

C  sphere types 
       integer*4   SPHOCT
       parameter ( SPHOCT = 0 ) 	
       integer*4   SPHICO
       parameter ( SPHICO = 1 ) 	
       integer*4   SPHBAR
       parameter ( SPHBAR = 2 ) 	
       integer*4   SPHCUB
       parameter ( SPHCUB = 3 ) 	
       integer*4   SPHBIL
       parameter ( SPHBIL = 4 ) 	

C  tesselation depth limit 
       integer*4   SPHMXD
       parameter ( SPHMXD = 30 ) 	

C  sphere drawing types 
       integer*4   SPHMSH
       parameter ( SPHMSH = 0 ) 	
       integer*4   SPHPOL
       parameter ( SPHPOL = 1 ) 	
       integer*4   SPHLIN
       parameter ( SPHLIN = 2 ) 	
       integer*4   SPHPNT
       parameter ( SPHPNT = 3 ) 	
       integer*4   SPHBIT
       parameter ( SPHBIT = 4 ) 	

C
C  START Sphere Library Function Declarations
C

       integer*4    sphdra
       integer*4    sphgnp
       integer*4    sphmod

C
C  END Sphere Library Function Declarations
C

