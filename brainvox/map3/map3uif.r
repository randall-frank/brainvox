;
;	dialog box for the brainvox regions program
;	uses default coloring ...
;
1	;DLG_HEADER
0
0
----
0 0 600 285 ;rect
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
;# map3_header
6	;DLG_TEXT
5	;length
0	;non-clickable
MAP-3
5 260 55 280  
4	;left just
;
;# map3_files
6	;DLG_TEXT
7	;length
0	;non-clickable
Files::
60 260 600 280  
4	;left just
;
;# map3_edit_value
7 	;DLG_EDIT
3	;          
0	;              
1.0                     
415 30 470 50
1
;
6	;DLG_TEXT
6	;          
0	;              
Value:
350 30 410 50
0
;
;# map3_exp
7 	;DLG_EDIT
3	;          
0	;              
1.0                     
45 30 105 50
1
;
6	;DLG_TEXT
4	;          
0	;              
Exp:
5 30 40 50 
0	
;
;# map3_offset
7 	;DLG_EDIT
3	;          
0	;              
0.0                     
180 30 240 50
1
;
6	;DLG_TEXT
7	;          
0	;              
Offset:
110 30 180 50 
0	
;
;# map3_n_chk
10	;DLG_CHECKBOX
7	;          
0	;              
N-Image
250 30 345 50 
0	;initially not selected
;
;# map3_add
11	;DLG_BUTTON
7
0
> Add >
260 235 340 255 
0
;
;# map3_remove
11	;DLG_BUTTON
6
0
Remove
260 210 340 230 
0
;
;# map3_clear
11	;DLG_BUTTON
5
0
Clear
260 185 340 205  
0
;
;# map3_up
11	;DLG_BUTTON
2
0
Up
260 160 340 180  
0
;
;# map3_down
11	;DLG_BUTTON
4
0
Down
260 135 340 155  
0
;
;# map3_home
11	;DLG_BUTTON
4
0
Home
260 110 340 130  
0
;
;# map3_plot
11	;DLG_BUTTON
4
0
Plot
260 85 340 105  
0
;
;# map3_opts
9	;DLG_MENU
4
0
Opts
260 60 340 80  
0
;
;# map3_rscroll
8       ;DLG_SBAR
9       ;tlength max=9,min=0
65545       ;tpos page=9,line=1
---   
576 54 596 256  
0    ;default position
;
;# map3_lscroll
8       ;DLG_SBAR
9       ;tlength max=9,min=0
65545       ;tpos page=9,line=1
---   
231 54 251 256  
0    ;default position
;
4       ;DLG_LINE
0       ;tlength
0       ;tpos
---     
4 54 230 54       
0    	;aux
;
4       ;DLG_LINE
0       ;tlength
0       ;tpos
---     
349 54 575 54   
0    	;aux
;
4       ;DLG_LINE
0       ;tlength
0       ;tpos
---     
4 54 4 256     
0    	;aux
;
4       ;DLG_LINE
0       ;tlength
0       ;tpos
---     
349 54 349 256     
0    	;aux
;
4       ;DLG_LINE
0       ;tlength
0       ;tpos
---     
4 256 230 256       
0    	;aux
;
4       ;DLG_LINE
0       ;tlength
0       ;tpos
---     
349 256 575 256   
0    	;aux
;
;# map3_ltext
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
5 235 230 255   
4    	;aux=left just
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
5 215 230 235   
4    	;aux=left just
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
5 195 230 215   
4    	;aux=left just
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
5 175 230 195   
4    	;aux=left just
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
5 155 230 175   
4    	;aux=left just
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
5 135 230 155   
4    	;aux=left just
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
5 115 230 135   
4    	;aux=left just
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
5 95 230 115   
4    	;aux=left just
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
5 75 230 95   
4    	;aux=left just
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
5 55 230 75   
4    	;aux=left just
;
;# map3_rtext
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
350 235 575 255   
4    	;aux=left just
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
350 215 575 235   
4    	;aux=left just
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
350 195 575 215   
4    	;aux=left just
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
350 175 575 195   
4    	;aux=left just
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
350 155 575 175   
4    	;aux=left just
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
350 135 575 155   
4    	;aux=left just
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
350 115 575 135   
4    	;aux=left just
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
350 95 575 115   
4    	;aux=left just
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
350 75 575 95   
4    	;aux=left just
;
6       ;DLG_TEXT
5       ;tlength
1       ;tpos=clickable
hello   
350 55 575 75   
4    	;aux=left just
;
;#	map3_done
11	;DLG_BUTTON
4
0
Done
5 5 125 25  
0
;
;#	map3_compute
11	;DLG_BUTTON
10
0
Calc MAP-3
130 5 250 25  
0
;
;#	map3_load
11	;DLG_BUTTON
10 
0
Load setup
350 5 470 25
0
;
;#	map3_save
11	;DLG_BUTTON
10
0
Save setup
475 5 595 25  
0
;
;#	map3_file_values
11	;DLG_BUTTON
12
0
Values<-file
475 30 595 50
0
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
