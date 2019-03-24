;
;	dialog box for the brainvox regions program
;	uses default coloring ...
;
1	;DLG_HEADER
0
0
----
0 0 700 230 ;rect
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
12	;length
0	;non-clickable
Region name:
355 130 475 150  
4	;left just
;
12	;DLG_RADIO NOP
9	;          
0	;              
Left Hemi       
5 205 170 225 
1	;initially selected
;
12	;DLG_RADIO NOP
10	;          
0	;              
Right Hemi       
180 205 345 225 
0	;initially NOT selected
;
7 	;DLG_EDIT
5	;          
0	;              
hello                     
355 105 695 125
1
;
12	;DLG_NOP
18	;          
0	;              
----
355 105 555 125 
1
;
12	;DLG_NOP
16	;          
0	;              
----
355 80 555 100 
0	
;
10	;DLG_CHECKBOX
13	;          
0	;              
Lesion region
355 80 520 100 
0	;initially not selected
;
11	;DLG_BUTTON
10
0
Add region
355  55  520 75  
0
;
11	;DLG_BUTTON
13
0
Remove region
355  30  520 50  
0
;
11	;DLG_BUTTON
4
0
Quit
355 5 520 25  
0
;
8       ;DLG_SBAR
9       ;tlength max=9,min=0
65541       ;tpos page=5,line=1
---   
325 4 350 151   
0    ;default position
;
4       ;DLG_LINE
0       ;tlength
0       ;tpos
---     
5 4 325 4       
0    	;aux
;
4       ;DLG_LINE
0       ;tlength
0       ;tpos
---     
5 4 5 151       
0    	;aux
;
4       ;DLG_LINE
0       ;tlength
0       ;tpos
---     
5 151 325 151     
0    	;aux
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
6 130 324 150   
4    	;aux=left just
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
6 105 324 129   
4    	;aux=left just
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
6 80 324 104   
4    	;aux=left just
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
6 55 324 79    
4    	;aux=left just
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
6 30 324 54    
4    	;aux=left just
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
6 5 324 29    
4    	;aux=left just
;
6	;DLG_TEXT
35	;length
0	;non-clickable
Brainvox region definition utility: 
5 205 360 225
4	;left just
;
4       ;DLG_LINE
0       ;tlength
0       ;tpos
---     
0 153 700 153     
0    	;aux
;
;#	bv_remove_svol
11	;DLG_BUTTON
16
0
Remove Subvolume
5 155  170 175  
0
;
;#	bv_edit_svol
7 	;DLG_EDIT
5	;          
0	;              
hello                     
425 180  695 200  
1
;
;#	bv_add_svol
11	;DLG_BUTTON
13
0
Add Subvolume
175 155  340 175  
0
;
;#	bv_rename_svol
11	;DLG_BUTTON
16
0
Rename Subvolume
345 155  510 175  
0
;
;#	bv_svol_menu
9       ;DLG_MENU
17
0
Select Subvolume:
5 180 250 200
0
;
;#	bv_svol_text
6	;DLG_TEXT
18	;length
0	;non-clickable
Current Subvolume: 
255 180 435 200
4	;left just
;
;#	bv_rgn_sort
11	;DLG_BUTTON
12
0
Sort regions
525  55  690 75  
0
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
