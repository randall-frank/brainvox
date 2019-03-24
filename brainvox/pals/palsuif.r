;
;	dialog box for the brainvox regions program
;	uses default coloring ...
;
1	;DLG_HEADER
0
0
----
0 0 480 300  ;rect
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
9       ;DLG_MENU
7
0
Palette
5 230 105 250
0
;
;#	pal_m_rgb
9	;DLG_MENU
4	;          
0	;              
Mode
115 230 215 250
0	;initially selected
;
11	;DLG_BUTTON
5
0
Reset
5 205 105 225  
0
;
11	;DLG_BUTTON
4 
0
Save
5 155 105 175  
0
;
11	;DLG_BUTTON
6
0
Revert
5 180 105 200
0
;
6	;DLG_TEXT
7 	;length
0	;non-clickable
Display 
5 105 105 125
4	;left just
;
6	;DLG_TEXT
4 	;length
0	;non-clickable
Edit 
115 105 215 125
4	;left just
;
10      ;DLG_CHKBOX
5       ;tlength
0       ;tpos
Red     
5 80 100 100    
1    	;aux
;
10      ;DLG_CHKBOX
5       ;tlength
0       ;tpos
Green   
5 55 100 75     
1    	;aux
;
10      ;DLG_CHKBOX
5       ;tlength
0       ;tpos
Blue    
5 30 100 50     
1    	;aux
;
10      ;DLG_CHKBOX
5       ;tlength
0       ;tpos
Alpha   
5 5 100 25    
1    	;aux
;
10      ;DLG_CHKBOX
5       ;tlength
0       ;tpos
Red     
115 80 210 100    
1    	;aux
;
10      ;DLG_CHKBOX
5       ;tlength
0       ;tpos
Green   
115 55 210 75     
1    	;aux
;
10      ;DLG_CHKBOX
5       ;tlength
0       ;tpos
Blue    
115 30 210 50     
1    	;aux
;
10      ;DLG_CHKBOX
5       ;tlength
0       ;tpos
Alpha   
115 5 210 25    
1    	;aux
;
;#	pal_t_pin1
6	;DLG_TEXT
23	;length
0	;non-clickable
000:000R 000G 000B 000A
5 280 215 300
4	;left just
;
;#	pal_t_pin2
6	;DLG_TEXT
23	;length
0	;non-clickable
000:000R 000G 000B 000A
5 255 215 275
4	;left just
;
;#	pal_m_tools
9	;DLG_MENU
8	;          
0	;              
Freehand
115 205 215 225
0	;initially selected
;
;#	pal_m_options
9	;DLG_MENU
7	;          
0	;              
Options
115 180 215 200
0	;initially selected
;
;#	pal_b_copy
11	;DLG_BUTTON
4
0
Copy
115 155 215 175
0
;
;#	pal_b_paste
11	;DLG_BUTTON
5
0
Paste
115 130 215 150
0
;
;#	pal_b_mpaste
11	;DLG_BUTTON
7
0
M-paste
5 130 105 150
0
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
