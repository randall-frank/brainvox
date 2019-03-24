;	This is the resource file for the program hello
;	it contains a 512x512 dialog for entering a text
;	string with ok and cancel buttons.  It uses the
;	default coloring scheme.
;
;#	Headitem
1	;DLG_HEADER
0
0
------
0 0 511 511   ;bounding box
0 	;paint the box
;
7	;DLG_EDIT
25	;tlength
5	;tpos
hello	
10 160 -1 -1 ;250 128   ;  text at 10,128
1	;framed
;
;#	OKbutton 
11	;Button OK DLG_BUTTON
2	; 2chars
0
Ok	
50 400 150 440
0
;# cancel1
;
11	;Button cancel DLG_BUTTON
6	; 6chars
0	;# cancelbut
Cancel	
350 400 450 440
0
;
8
255 
65546
fjahdsfhkj
30 210 190 240 ;ddd
128
;
8
255 
65546
fjahdsfhkj
450 300 480 10 ;ddd
128
;
9  ;DLG_MENU
10 
0
fjahdsfhkj
30 50 -1 -1   ;ddd
1
;
10
10 
0
fjahdsfhkj
30 300 -1 -1   ;ddd
1
;
10
10 
0
fjahdsfhkj
30 340 -1 -1   ;ddd
1
;
10
10 
0
fjahdsfhkj
30 380 -1 -1   ;ddd
1
;
13
10 
0
fjahdsfhkj
230 300 -1 -1   ;ddd
0
;
13
10 
0
fjahdsfhkj
230 340 -1 -1   ;ddd
0
;
13
10 
0
fjahdsfhkj
230 380 -1 -1   ;ddd
0
;
7	;DLG_EDIT
25	;tlength
5	;tpos
hello	
10 110 -1 -1 ;250 120  ;  text at 10,128
2561	;framed
;
6	;DLG_TEXT
1	;tlength
1	;tpos
A
240 240 -1 -1
1283
;
6	;DLG_TEXT
10	;tlength
1	;tpos
ABCDEFGHIJ
240 100 -1 -1
1536
;
6	;DLG_TEXT
10	;tlength
1	;tpos
ABCDEFGHIJ
240 160 420 130
1034	;1026+8 right justify
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
