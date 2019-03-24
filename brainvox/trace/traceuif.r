;	This is the resource file for the program brainvox_trace
;	it contains two 768x512 dialogs 
;	It uses the default coloring scheme.
;
;	dialog 1 (this is for state 1)
;
1	;DLG_HEADER
0
0
------
0 0 512 250   ;bounding box
0 	;do paint the box
;
2	;DLG_COLOR
0
0  
;LR LG LB FR FG FB IR IG IB B1RB1GB1BB2RB2GB2BB3RB3GB3BTR TG TB bkRbkGbkB
;000000000128128128000033033000052052000110110075113113255255255084084084
000000000128128128000017017000051051000119119085119119255255255085085085
0 0 0 0  
0
;
12	;DLG_NOP
511	;tlength (0-511)
65600	;tpos   (page by 64 pixels, line by 1)
placeholder
0 0 511 19   ;  horiz
255	;framed
;
12	;DLG_NOP
511	;tlength (0-511)
65600	;tpos   (page by 64 pixels, line by 1)
placeholder
513 531 533 20   ;  vert 
255	;framed
;
8	;DLG_sbar
40	;tlength (0-40)
65539	;tpos   (page by 4 pixels, line by 1)
placeholder
487 29 507 191   ;  vert 
0	;framed
;
11	;Button T2 DLG_BUTTON
7	; 2chars
0
Texture
261 5 336 25  
0
;
11	;Button PD DLG_BUTTON
5	; 2chars
0
Image
426 5 501 25
0
;
8	;DLG_sbar
40	;tlength (0-40)
65540	;tpos   (page by 4 pixels, line by 1)
placeholder
231 4 251 165   ;  select region bar 
0	;framed
;
8	;DLG_sbar
40	;tlength (0-40)
65540	;tpos   (page by 4 pixels, line by 1)
placeholder
5 190 251 210   ;  select image bar
0	;framed
;
6	;DLG_text
21
0
Select region to edit
5 166 250 186
4
;
6	;DLG_text
18
0
Background regions   
261 191 506 211
4
;
4	;DLG_Line
0
0
---
260 29 486 29
0
;
4	;DLG_Line
0
0
---
260 191 486 191
0
;
4	;DLG_Line
0
0
---
256 0 256 512 
0
;
4	;DLG_Line
0
0
---
260 29 260 190
0
;
6	;DLG_text
8
0
Patient:          
5 235 251 250
4
;
6	;DLG_text
16
0
Slice number:001  
5 215 251 230
4
;
4	;DLG_Line
0
0
---
4 4 230 4
0
;
4	;DLG_Line
0
0
---
4 165 230 165
0
;
4	;DLG_Line
0
0
---
4 4 4 165 
0
;
6	;DLG_text
1	;length=2
1	;clickable=true
1
261 170 486 190
0
;
6	;DLG_text
1	;length=2
1	;clickable=true
2
261 150 486 170
0
;
6	;DLG_text
1	;length=2
1	;clickable=true
3
261 130 486 150
0
;
6	;DLG_text
1	;length=2
1	;clickable=true
4
261 110 486 130
0
;
6	;DLG_text
1	;length=2
1	;clickable=true
5
261 90 486 110
0
;
6	;DLG_text
1	;length=2
1	;clickable=true
6
261 70 486 90
0
;
6	;DLG_text
1	;length=2
1	;clickable=true
7
261 50 486 70
0
;
6	;DLG_text
1	;length=2
1	;clickable=true
8
261 30 486 50
0
;
;	ROI select name set...
;
6	;DLG_text
1	;length=2
1	;clickable=true
1
5  145 230 165
0
;
6	;DLG_text
1	;length=2
1	;clickable=true
2
5  125 230 145
0
;
6	;DLG_text
1	;length=2
1	;clickable=true
3
5  105 230 125
0
;
6	;DLG_text
1	;length=2
1	;clickable=true
4
5  85 230 105
0
;
6	;DLG_text
1	;length=2
1	;clickable=true
5
5  65 230 85
0
;
6	;DLG_text
1	;length=2
1	;clickable=true
6
5  45 230 65
0
;
6	;DLG_text
1	;length=2
1	;clickable=true
7
5  25 230 45
0
;
6	;DLG_text
1	;length=2
1	;clickable=true
8
5  5 230 25
0
;
9 	;DLG_button
5	;tlength (0-40)
0	;tpos   (page by 4 pixels, line by 1)
Color      
341 25 421 5      ;  vert 
0	;framed
;
9       ;DLG_MENU  
7       ;tlength 
0       ;tpos   
Options    
261 225 361 245     ;  
0       ;framed
;
11      ;DLG_BUTTON                               
4       ;tlength 
0       ;tpos   
Quit       
370 225 470 245     ;  
0       ;framed
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
;	start of dialog number 2
;	used as state2
;
;
1	;DLG_HEADER
0
0
------
0 0 512 250   ;bounding box
0 	;do paint the box
;
2	;DLG_COLOR
0
0  
;LR LG LB FR FG FB IR IG IB B1RB1GB1BB2RB2GB2BB3RB3GB3BTR TG TB bkRbkGbkB
;000000000128128128000033033000052052000110110075113113255255255084084084
000000000128128128000017017000051051000119119085119119255255255085085085
0 0 0 0  
0
;
12	;DLG_NOP
511	;tlength (0-511)
65600	;tpos   (page by 64 pixels, line by 1)
placeholder
0 0 511 19   ;  horiz
255	;framed
;
12	;DLG_NOP
511	;tlength (0-511)
65600	;tpos   (page by 64 pixels, line by 1)
placeholder
1 531 20 20   ;  vert 
255	;framed
;
8	;DLG_sbar
40	;tlength (0-40)
65539	;tpos   (page by 4 pixels, line by 1)
placeholder
487 29 507 191   ;  vert 
0	;framed
;
11	;Button DLG_BUTTON
7	; 2chars
0
Texture
261 5 336 25  
0
;
11	;Button DLG_BUTTON
5	; 2chars
0
Image 	
426 5 501 25
0
;
11	;DLG_button
4	;tlength (0-40)
0	;tpos   (page by 4 pixels, line by 1)
Edit       
5 105 105 125   ;  vert 
0	;framed
;
11	;DLG_button
5	;tlength (0-40)
0	;tpos   (page by 4 pixels, line by 1)
Close      
5  155 105 175   ;  vert 
0	;framed
;
11	;DLG_button
6	;tlength (0-40)
0	;tpos   (page by 4 pixels, line by 1)
Backup     
120  130 220 150   ;  vert 
0	;framed
;
11	;DLG_button
6 	;tlength (0-40) was 19
0	;tpos   (page by 4 pixels, line by 1)
Revert to saved ROI
120  105 220 125   ;  vert 
0	;framed
;
6	;DLG_text  
0 	;tlength (0-40)
0	;tpos   
Hello  
5  30 225 50   ;  vert 
2	;framed
;
9 	;DLG_button
5	;tlength (0-40)
0	;tpos   (page by 4 pixels, line by 1)
Color      
120 180 220 200   ;  vert 
0	;framed
;
11	;DLG_button
3	;tlength (0-40)
0	;tpos   (page by 4 pixels, line by 1)
New        
5 180 105 200   ;  vert 
0	;framed
;
11	;DLG_button
4	;tlength (0-40)
0	;tpos   (page by 4 pixels, line by 1)
Save       
5 130 105 150   ;  vert 
0	;framed
;
6	;DLG_text
8
0
Patient:          
5 235 251 250
4
;
6	;DLG_text
16
0
Slice number:001  
5 215 251 230
4
;
6	;DLG_text
18
0
Background regions   
261 191 506 211
4
;
12	;DLG_NOP
0
0
---
8 200 228 200
0
;
12	;DLG_NOP
0
0
---
8  40 228 40
0
;
6	;DLG_text
1	;length=2
1	;clickable=true
1
261 170 486 190
0
;
6	;DLG_text
1	;length=2
1	;clickable=true
2
261 150 486 170
0
;
6	;DLG_text
1	;length=2
1	;clickable=true
3
261 130 486 150
0
;
6	;DLG_text
1	;length=2
1	;clickable=true
4
261 110 486 130
0
;
6	;DLG_text
1	;length=2
1	;clickable=true
5
261 90 486 110
0
;
6	;DLG_text
1	;length=2
1	;clickable=true
6
261 70 486 90
0
;
6	;DLG_text
1	;length=2
1	;clickable=true
7
261 50 486 70
0
;
6	;DLG_text
1	;length=2
1	;clickable=true
8
261 30 486 50
0
;
12	;DLG_NOP
0
0
---
8  200 8  40 
0
;
4	;DLG_Line
0
0
---
260 29 486 29
0
;
4	;DLG_Line
0
0
---
260 191 486 191
0
;
4	;DLG_Line
0
0
---
256 0 256 512 
0
;
4	;DLG_Line
0
0
---
260 29 260 190
0
;
9 	;DLG_MENU
4	; Copy a trace popup
0
Copy
120 155 220 175
0
;
11	;DLG_BUTTON
9
0
Autotrace
5 80 105 100     ;  Old QUIT button place...
0
;
;
8 	;DLG_SBAR
255	; 0-255
65552   ; page by 16 line by 1
---
5 55 220 75 
0
;
9 	;DLG_button
5	;tlength (0-40)
0	;tpos   (page by 4 pixels, line by 1)
Color      
341 25 421 5      ;  vert 
0	;framed
;
9       ;DLG_MENU  
7       ;tlength 
0       ;tpos   
Options    
261 225 361 245     ;  
0       ;framed
;
9       ;DLG_MENU                              
8       ;tlength 
0       ;tpos   
Close To   
120  80 220 100   ;  vert 
0	;framed
;
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
;	dialog 3 (this is for arb image control)
;
1	;DLG_HEADER
0
0
------
0 0 320 155   ;bounding box
0 	;do paint the box
;
2	;DLG_COLOR
0
0  
;LR LG LB FR FG FB IR IG IB B1RB1GB1BB2RB2GB2BB3RB3GB3BTR TG TB bkRbkGbkB
;000000000128128128000033033000052052000110110075113113255255255084084084
000000000128128128000017017000051051000119119085119119255255255085085085
0 0 0 0  
0
;
;# tr_arb_close
11	;DLG_BUTTON
5
0
Close
235 5 315 25
0
;# tr_arb_grab
11	;DLG_BUTTON
8
0
Grab rot
75 5 155 25
0
;# tr_arb_revert
11	;DLG_BUTTON
6
0
Revert
5 5 70 25
0
;# tr_arb_remove
11	;DLG_BUTTON
6
0
Remove
160 5 230 25
0
;
;# tr_arb_text1
6	;DLG_text
5	;length
0	;clickable
Mode:
5 130 55 150
4       ;left justify
;
;# tr_arb_text2
6	;DLG_text
31	;length
0	;clickable
Rx= 000.0 Ry= 000.0 Rz= 000.0
5 105 315 125
4       ;left justify
;
;# tr_arb_text3
6	;DLG_text
15	;length
0	;clickable
Base slice: 000
5 80 155 100
4       ;left justify
;
;# tr_arb_text4
6	;DLG_text
17	;length
0	;clickable
Number of slices:
5 55 175 75
4       ;left justify
;
;# tr_arb_text5
6	;DLG_text
19	;length
0	;clickable
Interslice spacing:
5 30 195 50
4       ;left justify
;
;# tr_arb_text6
6	;DLG_text
2	;length
0	;clickable
mm
275 30 295 50
4       ;left justify
;
;# tr_arb_text7
6	;DLG_text
8	;length
0	;clickable
Slice #:
165 80 245 100
4       ;left justify
;
;# tr_arb_islice
7       ;DLG_EDIT
3       ;length
0       ;non-clickable
1.0
200 30 270 50
1                      
;
;# tr_arb_nslices
7       ;DLG_EDIT
3       ;length
0       ;non-clickable
100
180 55 250 75
1                      
;
;# tr_arb_matchslice
7       ;DLG_EDIT
3       ;length
0       ;non-clickable
100
245 80 315 100
1                      
;
;# tr_arb_mode
9       ;DLG_MENU
6 
0
Slices
60 130 170 150
0
;
;# tr_arb_submode
9       ;DLG_MENU
6 
0
Global
175 130 315 150
0
;
3	;DLG_END
0
0
---
0 0 0 0
0
