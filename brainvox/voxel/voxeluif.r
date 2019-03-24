;
;	dialog box for the brainvox voxel program
;	uses default coloring ...
;
1	;DLG_HEADER
0
0
----
0 0 485 385  ;rect
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
6	;length
0	;non-clickable
Rx=000         
180 180 245 200
4	;left just
;
6	;DLG_TEXT
6	;length
0	;non-clickable
Ry=000         
180 155 245 175
4	;left just
;
6	;DLG_TEXT
6	;length
0	;non-clickable
Rz=000         
180 130 245 150
4	;left just
;
8	;DLG_SBAR 
359	;tlength (0 - 359)
65546	;tpos (page = 10 line = 1)
---             
5 180 170 200
0	;aux (initial = 0)
;
8	;DLG_SBAR 
359	;tlength (0 - 359)
65546	;tpos (page = 10 line = 1)
---             
5 155 170 175
0	;aux (initial = 0)
;
8	;DLG_SBAR 
359	;tlength (0 - 359)
65546	;tpos (page = 10 line = 1)
---             
5 130 170 150
0	;aux (initial = 0)
;
5	; DLG_BOX 
0	; 
0	; no fill
----
4 4 106 106
0	; framed
;
9	;DLG_MENU
7
0
Options
120 105 240 125
0
;
11	;DLG_BUTTON
6 
0
Render
120 80 240 100 
0
;
9	;DLG_MENU
9 
0
Rendering
120 55 240 75
0
;
9	;DLG_MENU
5
0
Color
120 30 240 50  
0
;
9 	;DLG_MENU
5
0
Clear
120 5 240 25   
0
;
8	;DLG_SBAR 
65546	;tlength (1 - 10)
65541	;tpos (page = 5 line = 1)
---             
5 205 170 225
1	;aux (initial = 1)
;
6	;DLG_TEXT
6	;length
0	;non-clickable
Im=000         
180 205 245 225
4	;left just
;
;  New cutting plane stuff starts here!!!
;
;#	cutbars
8	;DLG_SBAR 
359	;tlength (0 - 359)
65546	;tpos (page = 5 line = 1)
---             
250 205 415 225
256	;aux (initial = 1)
;
8	;DLG_SBAR 
359	;tlength (0 - 359)
65546	;tpos (page = 10 line = 1)
---             
250 180 415 200
0	;aux (initial = 0)
;
8	;DLG_SBAR 
359	;tlength (0 - 359)
65546	;tpos (page = 10 line = 1)
---             
250 155 415 175
0	;aux (initial = 0)
;
8	;DLG_SBAR 
511	;tlength (0 - 511)
65546	;tpos (page = 10 line = 1)
---             
250 130 415 150
256	;aux (initial = 0)
;
;#	cuttext
6	;DLG_TEXT
6	;length
0	;non-clickable
D=   0         
420 205 480 225
4	;left just
;
6	;DLG_TEXT
6	;length
0	;non-clickable
Rx=  0         
420 180 480 200
4	;left just
;
6	;DLG_TEXT
6	;length
0	;non-clickable
Ry=  0         
420 155 480 175
4	;left just
;
6	;DLG_TEXT
6	;length
0	;non-clickable
Rz=  0         
420 130 480 150
4	;left just
;
;#	statemenu
9	;DLG_MENU
8
0
Mode:Sli
250 105 360 125
0
;
;#	samplebutton
11	;DLG_BUTTON
6 
0
Sample
370 105 480 125 
2
;
4	;DLG_LINE
0 
0
---
247 0 247 230 
0
;
4	;DLG_LINE
0 
0
---
247 102 485 102 
0
;
;#	measurecolor
9 	;DLG_MENU
10
0
Co:Red
360 2 480 22   
0
;#	measuretext
6	;DLG_TEXT
22	;length
1	;clickable
Pos: 000.0 000.0 000.0   
250 79 480 99
4	;left just
;
6	;DLG_TEXT
11	;length
1	;clickable
Last:   0.0
250 5 360 20
4	;left just
;
6	;DLG_TEXT
22	;length
1	;clickable
Total:          0.0 mm   
250 23 480 38
4	;left just
;
6	;DLG_TEXT
23	;length
1	;clickable
Area:           0.0 mm2   
250 41 480 56
4	;left just
;
6	;DLG_TEXT
25	;length
1	;clickable
Tal:  000.0  000.0  000.0   
250 59 480 74
4	;left just
;
6	;DLG_TEXT
10	;length
0	;non-clickable
Voxelimage 
5 107 105 127
4	;left just
;
4	;DLG_LINE
0	;length
0	;non-clickable
----
0 230 485 230
0	;left just
;# MPF_6
6	;DLG_TEXT
7	;length
0	;non-clickable
Output:
95 285 165 305
4	;left just
;# MPF_7
6	;DLG_TEXT
8	;length
0	;non-clickable
N Slices
95 235 170 255
4	;left just
;# MPF_8
6	;DLG_TEXT
8	;length
0	;non-clickable
Cal slew
95 260 170 280
4	;left just
;# MPF_9
6	;DLG_TEXT
4	;length
0	;non-clickable
Xoff
235 235 275 255
4	;left just
;# MPF_10
6	;DLG_TEXT
4	;length
0	;non-clickable
tilt
235 260 275 280
4	;left just
;# MPF_1
6	;DLG_TEXT
11	;length
0	;non-clickable
Interslice:
135 335 245 355
4	;left just
;# MPF_2
6	;DLG_TEXT
11	;length
0	;non-clickable
Interpixel:
95 310 205 330
4	;left just
;# MPF_3
6	;DLG_TEXT
7	;length
0	;non-clickable
Xshift:
340 335 410 355
4	;left just
;
;# pointtags
6	;DLG_TEXT
15	;length
0	;non-clickable
Tot Tilt:   0.0
340 310 480 330
4	;left just
;
6	;DLG_TEXT
15	;length
0	;non-clickable
Tot Slew:   0.0
340 285 480 305
4	;left just
;
6	;DLG_TEXT
15	;length
0	;non-clickable
Tab Z:   0.0 mm
340 260 480 280
4	;left just
;
6	;DLG_TEXT
15	;length
0	;non-clickable
Tab Y:   0.0 mm
340 235 480 255
4	;left just
;
;# pointmenus
9	;DLG_MENU
6	;length
0	;non-clickable
RF Rgn
5 310 90 330
0                      
;
9	;DLG_MENU
6	;length
0	;non-clickable
LF Rgn
5 285 90 305
0
;
9	;DLG_MENU
6	;length
0	;non-clickable
RR Rgn
5 260 90 280
0
;
9	;DLG_MENU
6	;length
0	;non-clickable
LR Rgn
5 235 90 255
0
;
;# mpfitmenu
9	;DLG_MENU
11	;length
0	;non-clickable
MP_FIT opts
5 335 130 355
0                      
;# MPF_4
6	;DLG_TEXT
2	;length
0	;non-clickable
mm
305 335 325 355
4                      
;# MPF_5
6	;DLG_TEXT
2	;length
0	;non-clickable
mm
295 310 315 330
4                      
;
;# thickedit
7	;DLG_EDIT
4	;length
0	;non-clickable
6.64
250 335 305 355
1                      
;
;# ipixeledit
7	;DLG_EDIT
3	;length
0	;non-clickable
1.0
210 310 295 330
1                      
;
;# tempedit
7	;DLG_EDIT
15	;length
0	;non-clickable
sampled_%%%.pic
175 285 335 305
1                      
;
;# cslewedit
7	;DLG_EDIT
1	;length
0	;non-clickable
0
175 260 230 280
1                      
;
;# ctiltedit
7	;DLG_EDIT
1	;length
0	;non-clickable
0
280 260 335 280
1                      
;
;# countedit
7	;DLG_EDIT
2	;length
0	;non-clickable
15
175 235 230 255
1                      
;
;# offedit
7	;DLG_EDIT
4	;length
0	;non-clickable
0.00
280 235 335 255
1                      
;
;# xshiftedit
7	;DLG_EDIT
3	;length
0	;non-clickable
0.0
415 335 480 355
1
;
4	;DLG_LINE
0 
0
---
0 358 485 358 
0
;
;# taltext
6	;DLG_TEXT
12	;length
0	;non-clickable
X:
0 360 20 380
4	;left just
;
;# talxedit
7	;DLG_EDIT
3	;length
0	;non-clickable
0.0
25 360 90 380
1                      
;
6	;DLG_TEXT
2	;length
0	;non-clickable
Y:
95 360 115 380
4	;left just
;
;# talyedit
7	;DLG_EDIT
3	;length
0	;non-clickable
0.0
120 360 185 380
1                      
;
6	;DLG_TEXT
2	;length
0	;non-clickable
Z:
190 360 210 380
4	;left just
;
;# talzedit
7	;DLG_EDIT
3	;length
0	;non-clickable
0.0
215 360 280 380
1                      
;
;# pnttypemenu
9	;DLG_MENU
7 
0
Pnt:Tal
285 360 380 380
0
;
;# pntadd
11	;DLG_BUTTON
9
0
Add Point
385 360 480 380
0
;# CMD_LBL
6	;DLG_TEXT
8	;length
0	;non-clickable
Command:
5 260 90 280
0
;# CMD_PLAY
11	;DLG_BUTTON
12	;length
0	;non-clickable
Play file...
95 260 215 280
0
;# CMD_EDIT
7	;DLG_EDIT
0	;length
0	;non-clickable
H
5 235 480 255
1
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
;	dialog box for the brainvox voxel/pointsets program
;	uses default coloring ...
;
1	;DLG_HEADER
0
0
----
0 0 300 130  ;rect
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
;# pset_label1
6	;DLG_TEXT
17	;length
0	;non-clickable
Pointset renaming
5 105 295 125
4	;left just
;
;# pset_label2
6	;DLG_TEXT
9	;length
0	;non-clickable
New name:
5 55 105 75
8	;right just
;
;# pset_menu
9	;DLG_MENU
7
0
Pointset:
5 80 295 100
0
;
;# pset_doit
11	;DLG_BUTTON
5 
0
Apply
5 5 105 25 
0
;
;# pset_done
11	;DLG_BUTTON
4 
0
Done
110 5 210 25 
0
;
;# pset_edit
7	;DLG_EDIT
12	;length
0	;non-clickable
Point set 00
110 55 295 75
1                      
;
;# pset_locked
10      ;DLG_CHKBOX
6      ;          
0       ;              
Locked       
110 30 210 50 
0       ;initially NOT selected
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
;	dialog box for the brainvox voxel/MP_FIT I/O program
;	uses default coloring ...
;
1	;DLG_HEADER
0
0
----
0 0 300 130  ;rect
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
;# mpio_label1
6	;DLG_TEXT
21	;length
0	;non-clickable
MP_FIT settings files
5 105 295 125
4	;left just
;
;# mpio_label2
6	;DLG_TEXT
9	;length
0	;non-clickable
Filename:
5 55 105 75
8	;right just
;
;# mpio_menu
9	;DLG_MENU
6
0
Files:
5 80 295 100
0
;
;# mpio_done
11	;DLG_BUTTON
4 
0
Done
5 5 105 25 
0
;
;# mpio_load
11	;DLG_BUTTON
4 
0
Load
5 30 105 50 
0
;
;# mpio_save
11	;DLG_BUTTON
4 
0
Save
110 30 210 50 
0
;
;# mpio_edit
7	;DLG_EDIT
6	;length
0	;non-clickable
Sample
110 55 295 75
1                      
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
;	dialog box for the brainvox voxel/lighting model program
;	uses default coloring ...
;
1	;DLG_HEADER
0
0
----
0 0 300 130  ;rect
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
;# lmod_label1
6	;DLG_TEXT
1 	;length
0	;non-clickable
Lighting model
5 105 150 125
4	;left just
;
;# lmod_intrinsic
10	;DLG_CHECK
9	;length
0
Intrinsic
175 30 295 50
0	;default to off
;
;# lmod_LI_label
6	;DLG_TEXT
20	;length
0	;non-clickable
Light intensity:0.00
5 105 205 125
4	;left just
;
;# lmod_VI_label
6	;DLG_TEXT
20	;length
0	;non-clickable
Voxel intensity:0.00
5 55 205 75
4	;left just
;
;# lmod_menu
9	;DLG_MENU
7
0
Filters
205 5 295 25
0
;
;# lmod_done
11	;DLG_BUTTON
4 
0
Done
5 5 100 25 
0
;
;# lmod_postlight
11	;DLG_BUTTON
9 
0
Postlight
105 5 200 25 
0
;
;# lmod_VI_bar
8	;DLG_SBAR 
200	;tlength (0 - 200)
65546	;tpos (page = 10 line = 1)
---             
5 30 170 50
100	;aux (initial = 0)
;
;# lmod_LI_bar
8	;DLG_SBAR 
200	;tlength (0 - 200)
65546	;tpos (page = 10 line = 1)
---             
5 80 170 100
100	;aux (initial = 0)
;
5	; DLG_BOX 
0	; 
0	; no fill
----
225 55 295 125
0	; framed
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
;	dialog box for the brainvox voxel/texture program
;	uses default coloring ...
;
1	;DLG_HEADER
0
0
----
0 0 450 385  ;rect
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
;# tex_label1
6	;DLG_TEXT
10 	;length
0	;non-clickable
Volume I/O
5 155 115 175
4	;left just
;
;# tex_label2
6	;DLG_TEXT
9 	;length
0	;non-clickable
Directory
5 130 105 150
4	;left just
;
;# tex_label3
6	;DLG_TEXT
13 	;length
0	;non-clickable
Template name
5 80 145 100
4	;left just
;
;# tex_interp
10	;DLG_CHECK
20	;length
0
Interpolated loading
5 55 220 75
1	;default to on
;
;# tex_loadoptions
9	;DLG_MENU
18
0
Volume I/O options
225 55 445 75
0
;
;# tex_directory
7	;DLG_EDIT
1	;length
0	;non-clickable
.
5 105 445 125
1                      
;
;# tex_template
7	;DLG_EDIT
11	;length
0	;non-clickable
vol_%%%.pic
150 80 445 100
1                      
;
;# tex_select
11	;DLG_BUTTON
13
0
Select Volume
5 30 145 50 
0
;
;# tex_loadvol
11	;DLG_BUTTON
11
0
Load Volume
155 30 295 50 
0
;
;# tex_savevol
11	;DLG_BUTTON
11
0
Save Volume
305 30 445 50
0
;
;# tex_done
11	;DLG_BUTTON
4 
0
Done
5 5 100 25 
0
;
;# tex_label4
6	;DLG_TEXT
15 	;length
0	;non-clickable
2D Image Display Functions
5 285 205 305
4	;left just
;
;# tex_label5
6	;DLG_TEXT
25 	;length
0	;non-clickable
Image Texture Function 1
5 255 95 275
4	;left just
;
;# tex_label6
6	;DLG_TEXT
25 	;length
0	;non-clickable
Image Texture Function 2
5 205 95 225
4	;left just
;
;# tex_map1
9	;DLG_MENU
7
0
Options
5 230 445 250
0
;
;# tex_map2
9	;DLG_EDIT
7	;length
0	;non-clickable
Options
5 180 445 200
0                      
;
4	;DLG_LINE
0 
0
---
0 27 450 27 
0
;
4	;DLG_LINE
0 
0
---
0 178 450 178
0
;
4	;DLG_LINE
0 
0
---
0 307 450 307
0
;#      bv_depth_menu
;
9       ;DLG_MENU
10
0
Type:8bit 
110 155 230 175
0
;
6       ;DLG_TEXT
4       ;length
0       ;non-clickable
Min:
150 130 190 150
0
;
6       ;DLG_TEXT
4       ;length
0       ;non-clickable
Max:
300 130 340 150
0
;
;#      bv_min_edit
7       ;DLG_EDIT
1       ;tlength
0       ;tpos
0
195 130 295 150
1    ;framed
;
;#      bv_max_edit
7       ;DLG_EDIT
3       ;tlength
0       ;tpos
255
345 130 445 150
1    ;framed
;
;#      bv_zswap_chk
10      ;DLG_CHKBOX
8       ;
0       ;
Invert Z
340 155 445 175
0       ;initially NOT selected
;
;#      bv_bswap_chk
10      ;DLG_CHKBOX
8       ;
0       ;
Byteswap
235 155 335 175
0       ;initially NOT selected
;
6	;DLG_TEXT
24 	;length
0	;non-clickable
Texture Volume Clipping
5 360 295 380
4	;left just
;
;# tex_cl_fill
9	;DLG_MENU
11
0
Fill volume
5 335 145 355
0
;
;# tex_cl_clip
9	;DLG_MENU
11
0
Clip region
155 335 295 355 
0
;
6	;DLG_TEXT
7 	;length
0	;non-clickable
Value1:
302 335 365 355
4	;left just
;
6	;DLG_TEXT
7 	;length
0	;non-clickable
Value2:
302 310 365 330
4	;left just
;
;# tex_cl_val1
7	;DLG_EDIT
3
0
255
370 335 445 355
1	; boxed
;
;# tex_cl_val2
7	;DLG_EDIT
3
0
255
370 310 445 330
1	; boxed
;
;# tex_cl_options
9	;DLG_MENU
10
0
Volume ops
5 310 145 330
0
;
;# tex_cl_trace_opts
9	;DLG_MENU
12
0
2D Trace ops
155 310 295 330
0
;
;
3	;DLG_END
0
0
---
0 0 0 0
0
