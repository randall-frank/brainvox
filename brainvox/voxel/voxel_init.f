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
C    MODULE NAME    : voxel_init
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : Brainvox
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 15 Jun 95       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Breakout of voxel.f into modules...
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
	Subroutine build_menus(dialog,savep_menu,loadp_menu,grid_menu,
     +			im_menu,back_menu,peterror)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  

        Include '../libs/vt_inc.f'
        Include '../libs/vt_protos.f'
        Include 'voxeluif.inc'

        Include 'measure.inc'
        Include 'rotsdef.inc'
        Include 'palettes.inc'
        Include 'windows.inc'
C
        Record /ditem/  dialog(100)
C
	Character*250   tempstr,tstr
	Integer*4	savep_menu,loadp_menu,grid_menu,im_menu,back_menu
	logical		peterror

	Integer*4	i,j,bv_time,notes_menu,bv_os

	External	bv_time,bv_os

C       menus
C
        savep_menu = newpup()
        loadp_menu = newpup()
        tempstr = "Save pointset %t%s}"
        Call addtop(savep_menu,tempstr,index(tempstr,"}")-1,0)
        tempstr = "Load pointset %t%s}"
        Call addtop(loadp_menu,tempstr,index(tempstr,"}")-1,0)

        grid_menu = newpup()
        im_menu = newpup()
        im_menu2 = newpup()
        back_menu = newpup()
        dialog(10).aux=newpup()
        dialog(13).aux=newpup()
        dialog(14).aux=newpup()
        dialog(12).aux=newpup()
        dialog(measurecolor).aux=newpup()
        dialog(statemenu).aux=newpup()
        dialog(mpfitmenu).aux=newpup()
        dialog(pnttypemenu).aux=newpup()
C
C       Menus for the polygon selection popups
C
        Do J=0,3
                dialog(pointmenus+j).aux=newpup()
                Do i = 1,numrgns
                        write(tempstr,9)
     +                          rgnnames(i)(1:index(rgnnames(i)," ")),i
9       Format(A,"%r1%i%x",I2.2,"}")
                        Call addtop(dialog(pointmenus+j).aux,tempstr,
     +                          index(tempstr,"}")-1,0)
                Enddo
        Enddo
C
C       Menus for the multi-windows
C
        windows_menu = newpup()
        Do i=1,MAX_WINDOWS
                write(tempstr,787) i,9100+i
787     Format("2DImage",I2.2," %i%x",I4.4,"}")
                Call addtop(windows_menu,tempstr,index(tempstr,"}")-1,0)
        Enddo
C
C       Menus for the 3D cursor
C
        cursor3d_menu = newpup()
        tempstr = "Display cursor %i%x9200"
     +          //"|Display cut planes %i%x9203}"
        Call addtop(cursor3d_menu,tempstr,index(tempstr,"}")-1,0)
C
C 	Notes menu
C
	notes_menu = newpup()
	tempstr = "Erase = CTRL+LMB%d%x9800"
     +		//"|Backup = ALT+LMB or ALT+SHIFT+LMB or X+LMB%d%x9801"
     +		//"|Line = SHIFT+LMB%d%x9802"
     +		//"|3D Cursor = ALT+CTRL+LMB or X+CTRL+LMB%d%x9802"
     +		//"|Pan = SHIFT+RMB or MMB%d%x9803}"
        Call addtop(notes_menu,tempstr,index(tempstr,"}")-1,0)
	tempstr = "Trace@Z=0 = Z + LMB%d%x9804}"
        Call addtop(notes_menu,tempstr,index(tempstr,"}")-1,0)
C
C       Menus for the space type
C
        tempstr="Coordinate space %t|Brainvox %r1%x1|"//
     +          "Talairach %r1%I%x2}"
        Call addtop(dialog(pnttypemenu).aux,
     +          tempstr,index(tempstr,"}")-1,0)
C#ifndef NO_DSPACE
C        tempstr="Howard Disk %r1%x3}"
C        Call addtop(dialog(pnttypemenu).aux,
C     +          tempstr,index(tempstr,"}")-1,0)
C#endif
C
C       Menus for MP_FIT mode
C
        slice_menu = Newpup()
        tempstr = "Slice 01 %I%r5%x100}"
        Call addtop(slice_menu,tempstr,index(tempstr,"}")-1,0)
        Do j=1,14
                write(tempstr,6) j+1,j+100
6       Format("Slice ",I2.2," %r5%x",I3.3,"}")
                Call addtop(slice_menu,tempstr,index(tempstr,"}")-1,0)
        Enddo

        tempstr = "View first slice %I%x1|View other slices %i%x2|"//
     +        "View fixed polygon %i%x3|View construction lines %i%x7}"
        Call addtop(dialog(mpfitmenu).aux,tempstr,index(tempstr,"}")-1,
     +		0)
        tempstr = "Solid resampled slice %l%i%x6|"//
     +        "Overlay voxel image %l%I%x5}"
        Call addtop(dialog(mpfitmenu).aux,tempstr,index(tempstr,"}")-1,
     +		0)
        tempstr = "Resample to output files %x4|"//
     +        "Resample pointsets only %x8|MP_FIT settings files %x9|"
     +        //"Select resampled slice view %l%m}"
        Call addtop(dialog(mpfitmenu).aux,tempstr,index(tempstr,"}")-1,
     +          slice_menu)
        tempstr = "Calibrate glasses (RT,RE,LT,LE) %x10|"//
     +          "Load glasses fit %x11|Save glasses fit %l%x12}"
C
C       Right temple
C       Right ear
C       Left temple
C       Left ear
C
        Call addtop(dialog(mpfitmenu).aux,tempstr,index(tempstr,"}")-1,
     +		i)
        tempstr = "Xoff:positive to patient's left %d%x13}"
        Call addtop(dialog(mpfitmenu).aux,tempstr,index(tempstr,"}")-1,
     +		i)
C
C       State menu
C
C#ifdef NO_MP_FIT
C        tempstr = "Slices %I%r4%x0|Arbitrary %r4%x1|MP_FIT %d%l%r4%x2}"
C#else
        tempstr = "Slices %I%r4%x0|Arbitrary %r4%x1|MP_FIT %l%r4%x2}"
C#endif
        Call addtop(dialog(statemenu).aux,tempstr,index(tempstr,"}")-1,
     +		0)
        tempstr = "Select window %l%m|Autosampling %I%x1000}"
        Call addtop(dialog(statemenu).aux,tempstr,index(tempstr,"}")-1,
     +          windows_menu)
C
C       Background menu rollover
C
        tempstr = "Grayscale %t%s|0.00 %I%r3%x5000|0.25 %r3%x5025}"
        Call addtop(back_menu,tempstr,index(tempstr,"}")-1,0)
        tempstr = "0.50 %r3%x5050|0.75 %r3%x5075|1.00 %r3%x5100}"
        Call addtop(back_menu,tempstr,index(tempstr,"}")-1,0)
C
C       Menus for the loading and saving of points
C
        Do I=0,19
                Call get_point_name(i,tstr,'./ ')
                j = 200
                Do while (tstr(j:j) .eq. ' ')
                        j = j - 1
                Enddo
                write(tempstr,8) tstr(1:j),I+3000
                Call addtop(loadp_menu,tempstr,index(tempstr,"}")-1,0)
8       Format("Load ",A," %x",I4.4,"}")
                write(tempstr,7) tstr(1:j),I+4000
                Call addtop(savep_menu,tempstr,index(tempstr,"}")-1,0)
7       Format("Save ",A," %x",I4.4,"}")
        Enddo
        Call update_pset_menu(loadp_menu,3000,'./ ',0)
        Call update_pset_menu(savep_menu,4000,'./ ',1)
C
C       view popup
C
        i = Newpup()
        tempstr = "Views %t|Top %x1|Bottom %x2|Right %x3|Left %x4|"
     +          //"Front %x5|Back %x6|Ortho right %x7|Ortho left %x8}"
        Call addtop(i,tempstr,index(tempstr,"}")-1,0)
        tempstr = "Views %m}"
        Call addtop(dialog(10).aux,tempstr,index(tempstr,"}")-1,i)
        tempstr = "Background color %l%m|Send 2D pnts to trace %x100|"
     +            //"Reproject 2D pnts to 3D %l%x101}"
        Call addtop(dialog(10).aux,tempstr,index(tempstr,"}")-1,
     +		back_menu)
        tempstr = "Load point set %m}"
        Call addtop(dialog(10).aux,tempstr,index(tempstr,"}")-1,
     +		loadp_menu)
        tempstr = "Rename point sets... %x132|Save point set %l%m}"
        Call addtop(dialog(10).aux,tempstr,index(tempstr,"}")-1,
     +		savep_menu)
C Movieola
        tempstr = "Movieola window... %x137}"
        Call addtop(dialog(10).aux,tempstr,index(tempstr,"}")-1,0)
        tempstr = "Texture control dialog... %l%x108}"
        Call addtop(dialog(10).aux,tempstr,index(tempstr,"}")-1,0)
        tempstr = "Orientation overlays %i%x135}"
        Call addtop(dialog(10).aux,tempstr,index(tempstr,"}")-1,0)

C Talairach submenu (get PET data first)

        i = Newpup()
        tempstr = "MRI volume %x113}"
        Call addtop(i,tempstr,index(tempstr,"}")-1,0)

	If (.not. peterror) Then

        tempstr = "PET injection 1 %x114|PET injection 2 %x115|"
     +          //"PET injection 3 %x116|PET injection 4 %x117}"
        	Call addtop(i,tempstr,index(tempstr,"}")-1,0)

        tempstr = "PET injection 5 %x118|PET injection 6 %x119|"
     +          //"PET injection 7 %x120|PET injection 8 %x121}"

        	Call addtop(i,tempstr,index(tempstr,"}")-1,0)

	Endif

C Talairach stuff
C#ifndef NO_TAL_SPACE
        j = Newpup()
        tempstr = "Talairach Space %t}"
        Call addtop(j,tempstr,index(tempstr,"}")-1,0)

        tempstr = "Calib Talairach (CA,CP,pl) %x110|"
     +		//"Load Talairach %x111}"
        Call addtop(j,tempstr,index(tempstr,"}")-1,0)

        tempstr = "Save Talairach %x112|Resample Talairach %m|"
     +          //"Overlay Talairach %l%i%x130}"
        Call addtop(j,tempstr,index(tempstr,"}")-1,i)

        tempstr = "St Louis variant mods %x136}"
        Call addtop(j,tempstr,index(tempstr,"}")-1,0)
        tempstr = "Talairach support %l%m}"
        Call addtop(dialog(10).aux,tempstr,index(tempstr,"}")-1,j)
C#endif
        tempstr = "Read points from _hotspots %x131}"
        Call addtop(dialog(10).aux,tempstr,index(tempstr,"}")-1,0)
C
C       clipping toggle
C
        tempstr = "Clip objects to surface %i%x133}"
        Call addtop(dialog(10).aux,tempstr,index(tempstr,"}")-1,0)
        tempstr = "Shade clipped pointsets %i%x138}"
        Call addtop(dialog(10).aux,tempstr,index(tempstr,"}")-1,0)
        tempstr = "Fit Arb image plane to pnts %x134}"
        Call addtop(dialog(10).aux,tempstr,index(tempstr,"}")-1,0)
C
C       Matt Howard Disk Space (dspace) (500-504)
C
C#ifndef NO_DSPACE
C        i = Newpup()
C        tempstr = "Howard Disk %t|Calibrate space (CT,X,CCW) %x500}"
C        Call addtop(i,tempstr,index(tempstr,"}")-1,0)
C        tempstr = "Load calibration %x504|Save calibration %x503}"
C        Call addtop(i,tempstr,index(tempstr,"}")-1,0)
C        tempstr = "Lines display %i%x501|Plane display %i%x502}"
C        Call addtop(i,tempstr,index(tempstr,"}")-1,0)
C        tempstr = "Howard Disk support %m}"
C        Call addtop(dialog(10).aux,tempstr,index(tempstr,"}")-1,i)
C#endif
C
C       RJF options
C
        If (bv_test_option(OPTS_RJF)) Then
                Call bv_seed_frand(bv_time())
                j =  Newpup()
                tempstr="Random points %x1000|1000 points %x1001}"
                Call addtop(j,tempstr,index(tempstr,"}")-1,0)
                tempstr = "RJF - options %m}"
                Call addtop(dialog(10).aux,tempstr,
     +			index(tempstr,"}")-1,j)
        Endif
C
C       preview popup
C
        j = Newpup()
        tempstr="Full quality %I%r1%x1|2x speed %r1%x2|"//
     +			"3x speed %r1%x3|"//
     +                  "4x speed %r1%x4|5x speed %r1%x5}"
        Call addtop(j,tempstr,index(tempstr,"}")-1,0)

        tempstr="Rendering quality %m%l}"
        Call addtop(dialog(12).aux,tempstr,index(tempstr,"}")-1,j)
        tempstr="Render with texture volume %i%x100|"//
     +		"Hardware acceleration %i%x102|"//
     +          "Auto rendering %i%x103|"//
     +          "Reload hardware texture %l%x104}"
        Call addtop(dialog(12).aux,tempstr,index(tempstr,"}")-1,0)
C Lighting options
        tempstr = "Quicklight the image %x139|"//
     +          "Lighting controls... %l%x140}"
        Call addtop(dialog(12).aux,tempstr,index(tempstr,"}")-1,0)
C Filter submenu
        i = Newpup()
        tempstr = "Image filters %t|Median filter %x602|"
     +        //"Average filter %x603|"
     +        //"Zaverage filter %x604|Zmedian filter %x605}"
        Call addtop(i,tempstr,index(tempstr,"}")-1,0)
C Filter
        tempstr = "Filters %l%m}"
        Call addtop(dialog(12).aux,tempstr,index(tempstr,"}")-1,i)
        tempstr = "Overlay render %x101}"
        Call addtop(dialog(12).aux,tempstr,index(tempstr,"}")-1,0)
C
C       color popups
C
        tempstr="Trace Color %t|Black %s%x0|Red %x1|Green %x2|"//
     +          "Yellow %x3|Blue %x4}"
        Call addtop(dialog(13).aux,tempstr,index(tempstr,"}")-1,0)
        tempstr = "Magenta %x5|Cyan %x6|White %x7}"
        Call addtop(dialog(13).aux,tempstr,index(tempstr,"}")-1,0)
        tempstr = "Coral%x8|Chartreuse%x9|SpringGreen%x10|"//
     +		"SlateBlue%x11|BlueViolet%x12|OrangeRed%x13}"
        Call addtop(dialog(13).aux,tempstr,index(tempstr,"}")-1,0)
C
        tempstr="Measure Color %t|Black %s%x0|Red %x1|Green %x2|"//
     +          "Yellow %x3|Blue %x4}"
        Call addtop(dialog(measurecolor).aux,tempstr,
     +		index(tempstr,"}")-1,0)
        tempstr = "Magenta %x5|Cyan %x6|White %x7}"
        Call addtop(dialog(measurecolor).aux,tempstr,
     +		index(tempstr,"}")-1,0)
        tempstr = "Coral%x8|Chartreuse%x9|SpringGreen%x10|"//
     +		"SlateBlue%x11|BlueViolet%x12|OrangeRed%l%x13}"
        Call addtop(dialog(measurecolor).aux,tempstr,
     +		index(tempstr,"}")-1,0)
C#ifdef NO_SURF
C        tempstr = "Surfaces disabled %d}"
C        Call addtop(dialog(measurecolor).aux,tempstr,
C     +		index(tempstr,"}")-1,0)
C#else
C       tempstr="Save FLIP surface %x100|Save SurfView surface %x101}"
C       Call addtop(dialog(measurecolor).aux,tempstr,
C    +		index(tempstr,"}")-1,0)
C       tempstr="Run SurfView (FLIP) %x103|Run SurfView %x102}"
C       Call addtop(dialog(measurecolor).aux,tempstr,
C    +		index(tempstr,"}")-1,0)
C#endif
C
C       clear traces popup
C
        tempstr="Clear Traces %t|All traces %s%x100|"//
     +		"Voxel traces %x102|Image traces %x101}"
        Call addtop(dialog(14).aux,tempstr,index(tempstr,"}")-1,0)
        tempstr = "Black %x0|Red %x1|Green %x2|Yellow %x3|Blue %x4}"
        Call addtop(dialog(14).aux,tempstr,index(tempstr,"}")-1,0)
        tempstr = "Magenta %x5|Cyan %x6|White %x7}"
        Call addtop(dialog(14).aux,tempstr,index(tempstr,"}")-1,0)
        tempstr = "Coral%x8|Chartreuse%x9|SpringGreen%x10|"//
     +		"SlateBlue%x11|BlueViolet%x12|OrangeRed%x13}"
        Call addtop(dialog(14).aux,tempstr,index(tempstr,"}")-1,0)
C
C       Grid menu rollover (number = 1000 + 2*mm)
C
        tempstr = "None %x1000|0.2 cm %x1004|0.5 cm %x1010|"//
     +                  "1 cm %x1020|2 cm %x1040}"
        Call addtop(grid_menu,tempstr,index(tempstr,"}")-1,0)
        tempstr = "3 cm %x1060|4 cm %x1080|5 cm %x1100|10 cm %x1200}"
        Call addtop(grid_menu,tempstr,index(tempstr,"}")-1,0)
C
C       zoom/save popup
C
        tempstr = "Save image %x100}"
        Call addtop(im_menu,tempstr,index(tempstr,"}")-1,0)
        If (bv_os() .eq. 0) then
           tempstr = "Copy image %x101}"
           Call addtop(im_menu,tempstr,index(tempstr,"}")-1,0)
        Endif
        tempstr = "Zoom 1 %x1|Zoom 2 %x2|Zoom 3 %x3}"
        Call addtop(im_menu,tempstr,index(tempstr,"}")-1,0)
        tempstr = "Zoom 4 %x4|Zoom 5 %x5|Grid %m}"
        Call addtop(im_menu,tempstr,index(tempstr,"}")-1,grid_menu)
        tempstr = "3D cursor %m}"
        Call addtop(im_menu,tempstr,index(tempstr,"}")-1,
     +          cursor3d_menu)
	Call keyboard_create_menu(im_menu)
        tempstr = "Help %m}"
        Call addtop(im_menu,tempstr,index(tempstr,"}")-1,notes_menu)
C
        tempstr = "Select this window %x9001|"//
     +                  "Add new window %x9002|"//
     +                  "Close this window %x9003|"//
     +                  "Select window %l%m}"
        Call addtop(im_menu2,tempstr,index(tempstr,"}")-1,windows_menu)
C
        tempstr = "Save image %x100}"
        Call addtop(im_menu2,tempstr,index(tempstr,"}")-1,0)
        If (bv_os() .eq. 0) then
           tempstr = "Copy image %x101}"
           Call addtop(im_menu2,tempstr,index(tempstr,"}")-1,0)
        Endif
        tempstr="Zoom 1 %x1|Zoom 2 %x2|Zoom 3 %x3|Zoom 4 %x4|"
     +		//"Zoom 5 %x5}"
        Call addtop(im_menu2,tempstr,index(tempstr,"}")-1,0)

        tempstr = "Grid %l%m}"
        Call addtop(im_menu2,tempstr,index(tempstr,"}")-1,grid_menu)

        tempstr = "Image palette color %I%r1%x8000|"//
     +          "Image texture function 1 %r1%x8001|"//
     +          "Image texture function 2 %l%r1%x8002}"
        Call addtop(im_menu2,tempstr,index(tempstr,"}")-1,0)

        tempstr = "Track 3D cursor %i%x8003}"
        Call addtop(im_menu2,tempstr,index(tempstr,"}")-1,0)

        tempstr = "3D cursor %m}"
        Call addtop(im_menu2,tempstr,index(tempstr,"}")-1,
     +          cursor3d_menu)
	Call keyboard_create_menu(im_menu2)
        tempstr = "Help %m}"
        Call addtop(im_menu2,tempstr,index(tempstr,"}")-1,notes_menu)
C
	Return
	End
