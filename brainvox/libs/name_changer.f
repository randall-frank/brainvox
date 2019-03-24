C    ======================================================================     
C    THIS SOFTWARE IS THE PROPERTY OF:                                          
C         The Image Analysis Facility                                     
C         133 MRC     
C         The University of Iowa, Iowa City, Iowa                               
C    ======================================================================     
C                                                                               
C    -- OVERVIEW --------------                                                 
C                                                                               
C    MODULE NAME    : Name_Changer 
C    VERSION        : 1.0 
C    LOCATION       : [Richards.work.hatnhead] 
C    LAST EDIT      :  
C    STATUS         :  Development
C    LANGUAGE       :  VAX FORTRAN
C    PACKAGE        :  HATNHEAD
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     :  D. Richards
C    DATE           :  30 Oct. 1989
C    LINE-BY-LINER  :
C    APPROVAL       :
C
C    DESCRIPTION    :                                                           
C	This routine accepts a base filename and a slice number
C    and converts them according to the wildcards.  For a better
C    description see the SDD for HATNHEAD.
C                                                                               
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C	Modified 24 Jan 90 by rjf
C		Routine correctly handles slice=0 with '*' and returns
C		error=.true. if slice<0.  Also changed to allow any
C		length strings...
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :
C			Call Name_Changer(In_Name,Slice_Number,Out_Name,Error)
C    INPUTS         :   In_Name, Slice_Number                      
C    OUTPUTS        :   Out_Name,Error                 
C
C    LIMITATIONS    :                                                           
C    NOTES          :                                                           
C                                                                               
C                                                                               
C    ======================================================================     
                                            
	Subroutine Name_Changer(In_Name,Slice,Out_Name,Error)

	IMPLICIT NONE
	Character*(*)	In_Name
	Character*(*)	Out_Name
	Integer*4	Slice
	Logical		Error

	Integer*4	Wild1,Wild2
	Integer*4	Place1,Place2
	Integer*4	Temp
	Integer*4	Num_Place,Length
	Integer*4	Slice_Start
	Integer*4	Num_Slice
	Integer*4	L1,L2,strlen
	External	strlen

	Character*5	Temp2,Slice_Str
	Character*220	Part1,Part2
C
C   first check for errors in filename specification
C
	error =.false.
	If (slice .lt. 0) then
		Error =.true.
		Return
	Endif
	Wild1 = INDEX(In_Name,'*')
	Place1 = INDEX(In_Name,'%')

	If(Place1 .ne. 0 .and. Wild1 .ne. 0) Then
		Error = .true.
		Return
	Endif

	If(Place1 .eq. 0 .and. Wild1 .eq. 0) Then
		Error = .true.
		Return
	Endif

	Wild2 = INDEX(In_Name(Wild1+1:),'*')

	If(Wild2 .ne. 0) Then
		Error = .true.
		Return
	Endif

	Temp = Place1+1
	Do While( In_Name(Temp:Temp) .eq. '%')
		Temp = Temp + 1
	Enddo

	Place2 = INDEX(In_Name(Temp:),'%')

	If(Place2 .ne. 0) Then
		Error = .true.
		Return
	Endif

	Num_Place = Temp - Place1
	If(Slice .ge. 10**Num_Place .and. Place1 .gt. 0) Then
		Error = .true.
		Return
	Endif

C
C  The filename is valid
C
	Length = strlen(In_Name)

	write(Slice_Str,10) Slice        !Num to string conversion
10	Format(I5.5)
                              
	Slice_Start = 1
	Do While(Slice_Str(Slice_Start:Slice_Start) .eq. '0')
		Slice_Start = Slice_Start + 1
	Enddo
	If (slice .eq. 0) slice_start=slice_start-1

	Part1 = ' '
	Part2 = ' '

	If( Wild1 .ne. 0) Then
            If(Wild1 .ne. 1) Part1 = In_Name(1:Wild1-1)
            If(Wild1 .ne. Length) Part2 = In_Name(Wild1+1:)
            L1 = strlen(Part1)
            L2 = strlen(Part2)
            If (Wild1 .eq. 1) Then
		Out_Name=SLice_Str(Slice_Start:)//Part2(1:L2)
            Else
		Out_Name=Part1(1:L1)//SLice_Str(Slice_Start:)//Part2(1:L2)
            Endif
	Endif

	If( Place1 .ne. 0) Then
	    If(Place1 .ne. 1) Part1 = In_Name(1:Place1-1)
	    If(Temp-1 .ne. Length) Part2 = In_Name(Temp:)
	    L1 = strlen(Part1)
	    L2 = strlen(Part2)
            If (Place1 .eq. 1) Then
 	   	Out_Name=Slice_str(6-Num_Place:)//Part2(1:L2)
            Else
 	   	Out_Name=Part1(1:L1)//Slice_str(6-Num_Place:)//Part2(1:L2)
	    Endif
 	Endif

	Return
	End
