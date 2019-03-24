C    ======================================================================     
C    THIS SOFTWARE IS THE PROPERTY OF:                                          
C         The Image Analysis Facility                                     
C         71 HBRF     
C         The University of Iowa, Iowa City, Iowa                               
C    ======================================================================     
C	Copyright (C) 1989 Image Analysis Facility, University of Iowa
C    ======================================================================     
C                                                                               
C    -- OVERVIEW --------------                                                 
C                                                                               
C    MODULE NAME    : _Main   
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : cpflip
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 6 jul 89       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	this program reads a binary format 8bit image file
C			and will pass it through a lookup table and/or
C			flip it over the x and/or y axis
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     cpflip image xsize ysize {.pal {-r -g -b}} {-x}{-y}
C    INPUTS         :          
C    OUTPUTS        : 
C
C    LIMITATIONS    :      
C    NOTES          :      The program will assume the image to be xsize by
C			ysize and will pass the bytes through the lut
C			in the .pal format file.  The default vector is the
C			red lut but the user may specify green or blue.
C                 
C    ====================================================================== 
C
