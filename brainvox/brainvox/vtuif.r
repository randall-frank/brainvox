;
;	dialog box for the brainvox setup menu   
;
1	;DLG_HEADER
0
0
----
0 0 700 235  ;rect
0   ;paint the box
;
2       ;DLG_COLOR
0
0  
;LR LG LB FR FG FB IR IG IB B1RB1GB1BB2RB2GB2BB3RB3GB3BTR TG TB bkRbkGbkB
;000000000128128128000033033000052052000110110075113113255255255084084084
000000000128128128000017017000051051000119119085119119255255255085085085
0 0 0 0  
0
;
6	;DLG_TEXT
43	;length
0	;non-clickable
Descriptive Patient and Scan Information:
5 180 430 200  
0
;
6 	;DLG_TEXT
19	;          
0	;              
Imagename Template:       
5 80 200 100 
0
;
6 	;DLG_TEXT
25	;          
0	;              
Interpixel distance (mm): 
5 55 250 75  
0
;
6 	;DLG_TEXT
25	;          
0	;              
Interslice distance (mm): 
5 30 250 50  
0
;
11	;DLG_BUTTON
8
0
Continue
5  5  105 25  
0
;
11	;DLG_BUTTON
5
0
Abort
110 5  210 25  
0
;
7       ;DLG_EDIT
76      ;tlength
1       ;tpos
hello   
5 155 695 175   
1    ;framed
;
7       ;DLG_EDIT
76      ;tlength
1       ;tpos
hello   
5 130 695 150   
1    ;framed
;
7       ;DLG_EDIT
76      ;tlength
1       ;tpos
hello   
5 105 695 125   
1    ;framed
;
7       ;DLG_EDIT
53      ;tlength
1       ;tpos
hello   
205 80 695 100   
1    ;framed
;
7       ;DLG_EDIT
16      ;tlength
1       ;tpos
hello   
255 55 355 75    
1    ;framed
;
7       ;DLG_EDIT
16      ;tlength
1       ;tpos
hello   
255 30 355 50    
1    ;framed
;
6	;DLG_TEXT
43	;length
0	;non-clickable
Please enter the following information:       
5 205 430 225
0
;#	bv_depth_menu
;
9	;DLG_MENU
16
0
Image type:8bit 
400 55 580 75  
0
;
6	;DLG_TEXT
4	;length
0	;non-clickable
Min:      
400 30 440 50
0
;
6	;DLG_TEXT
4	;length
0	;non-clickable
Max:      
550 30 590 50
0
;
;#	bv_min_edit
7       ;DLG_EDIT
1       ;tlength
0       ;tpos
0   
445 30 545 50    
1    ;framed
;
;#	bv_max_edit
7       ;DLG_EDIT
3       ;tlength
0       ;tpos
255
595 30 695 50    
1    ;framed
;
;#	bv_zswap_chk
10      ;DLG_CHKBOX
8       ;
0       ;
Invert Z
590 55 695 75
0       ;initially NOT selected
;
;#	bv_bswap_chk
10      ;DLG_CHKBOX
8       ;
0       ;
Byteswap
400 5 505 25
0       ;initially NOT selected
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
;
;	dialog box for the brainvox main menu   
;
1	;DLG_HEADER
0
0
----
0 0 700 235  ;rect
0   ;paint the box
;
2       ;DLG_COLOR
0
0  
;LR LG LB FR FG FB IR IG IB B1RB1GB1BB2RB2GB2BB3RB3GB3BTR TG TB bkRbkGbkB
;000000000128128128000033033000052052000110110075113113255255255084084084
000000000128128128000017017000051051000119119085119119255255255085085085
0 0 0 0  
0
;
6	;DLG_TEXT
43	;length
0	;non-clickable
Descriptive Patient and Scan Information:
5 180 430 200  
0
;
6 	;DLG_TEXT
19	;          
0	;              
Imagename Template:       
5 80 200 100 
0
;
12	;DLG_NOP 
25	;          
0	;              
Interpixel distance (mm): 
5 55 250 75  
0
;
12	;DLG_NOP 
25	;          
0	;              
Interslice distance (mm): 
5 30 250 50  
0
;
11	;DLG_BUTTON
4
0
Quit    
5  5  115 25  
0
;
11	;DLG_BUTTON
9
0
Edit info
120 5  230 25  
0
;
6       ;DLG_TEXT
76      ;tlength
0       ;tpos
hello   
5 155 695 175   
6    ;framed
;
6       ;DLG_TEXT
76      ;tlength
0       ;tpos
hello   
5 130 695 150   
6    ;framed
;
6       ;DLG_TEXT
76      ;tlength
0       ;tpos
hello   
5 105 695 125   
6    ;framed
;
6       ;DLG_TEXT
53      ;tlength
0       ;tpos
hello   
205 80 695 100   
6    ;framed
;
11	;DLG_BUTTON
25
0
Define and Cut Subvolumes
5 55 345 75  
0
;
11	;DLG_BUTTON
25
0
Create/Modify Region List
355 55 695 75  
0
;#	bv_svol_menu
;
9	;DLG_MENU
17
0
Select subvolume:
5 30 345 50  
0
;#	bv_svol_button
;
11	;DLG_BUTTON
35
0
Define Subvolume Regions to Analyze
355 30 695 50  
0
;
11	;DLG_BUTTON
18
0
Compute Statistics
355 5 695 25  
0
;
6	;DLG_TEXT
43	;length
0	;non-clickable
Brainvox main menu:                           
5 205 430 225
0
;
;#	bv_preferences
11	;DLG_BUTTON
11
0
Preferences
235 5 345 25  
0
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
;	dialog box for the brainvox new setup dialog
;
1	;DLG_HEADER
0
0
----
0 0 700 235  ;rect
0   ;paint the box
;
2       ;DLG_COLOR
0
0  
;LR LG LB FR FG FB IR IG IB B1RB1GB1BB2RB2GB2BB3RB3GB3BTR TG TB bkRbkGbkB
;000000000128128128000033033000052052000110110075113113255255255084084084
000000000128128128000017017000051051000119119085119119255255255085085085
0 0 0 0  
0
;
6       ;DLG_TEXT
51      ;tlength
0       ;tpos
Brainvox has not been run in this directory before.
5 155 695 175   
4    ;left just
;
6       ;DLG_TEXT
58      ;tlength
0       ;tpos
(An '_patient' file could not be found in this directory.)
5 130 695 150   
4    ;left just
;
6       ;DLG_TEXT
56      ;tlength
0       ;tpos
Would you like to setup a new dataset in this directory?
5 105 695 125   
4    ;left just
;
;#	bv_new_exit
11	;DLG_BUTTON
4
0
Quit    
5  5  170 25  
0
;
;#	bv_new_setup
11	;DLG_BUTTON
17
0
Setup New Dataset
180 5  345 25  
0
;
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
;	dialog box for brainvox Preferences
;
1	;DLG_HEADER
0
0
----
0 0 700 235  ;rect
0   ;paint the box
;
2       ;DLG_COLOR
0
0  
;LR LG LB FR FG FB IR IG IB B1RB1GB1BB2RB2GB2BB3RB3GB3BTR TG TB bkRbkGbkB
;000000000128128128000033033000052052000110110075113113255255255084084084
000000000128128128000017017000051051000119119085119119255255255085085085
0 0 0 0  
0
;
6       ;DLG_TEXT
38      ;tlength
0       ;tpos
Set Brainvox preferences for this user
5 205 695 225   
4    ;left just
;
6       ;DLG_TEXT
58      ;tlength
0       ;tpos
Preference changes may occur the next time Brainvox is run.
5 180 695 200   
4    ;left just
;
6       ;DLG_TEXT
24      ;tlength
0       ;tpos
Backup rate (points/sec)
5 155 240 175   
4    ;left just
;
;#	bv_backup_edit
7       ;DLG_EDIT
1       ;tlength
0       ;tpos
0   
240 155 340 175    
1    ;framed
;
;#	bv_mouse_check
10      ;DLG_CHKBOX
24      ;
0       ;
Use IrisGL mouse mapping
5 130 330 150
0       ;initially NOT selected
;
;#	bv_noreap_check
10      ;DLG_CHKBOX
31      ;
0       ;
Do not use the Brainvox reaper
5 105 330 125
0       ;initially NOT selected
;
;#	bv_hwinterp_check
10      ;DLG_CHKBOX
31      ;
0       ;
Use 3D HW volume interpolation
5 80 330 100
0       ;initially NOT selected
;
;#	bv_prefs_save
11	;DLG_BUTTON
14
0
Save and close
5  5  170 25  
0
;
;#	bv_prefs_close
11	;DLG_BUTTON
5
0
Close
180 5  345 25  
0
;
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
