;	This is the resource file for the program Resedit
;	Copyright (C) 1991 rjf
;
1	;DLG_HEADER
0
0
------
0 0 560 255   ;bounding box
0 	;paint the box
;
2       ;DLG_COLOR
0
0  
000000000128128128000033033000052052000110110075113113255255255084084084
0 0 0 0  
0
;
;#	newfile 
11	;DLG_BUTTON
8	;
0
New file
5 205 155 225 
0
;
;#	openfile
11	;DLG_BUTTON
9	;
0	;
Open file
5 180 155 200   
0
;
;#	savefile
11	;DLG_BUTTON
9	;
0	;
Save file
5 155 155 175   
0
;
;#	appendfile
11	;DLG_BUTTON
11	;
0	;
Append file
5 130 155 150   
0
;
;#	quit
11	;DLG_BUTTON
4	;
0	;
Quit
5 105 155 125   
0
;
6	;DLG_TEXT
7	;tlength
0	;tpos
Window:
5 80 155 100 
4
;
;#	windowtext
6	;DLG_TEXT
11	;tlength
0	;tpos
(xxxx,xxxx)
5 55 155 75 
4
;
;#	itemtext2
6	;DLG_TEXT
15	;tlength
0	;tpos
xxx,xxx-xxx,xxx
5 5 155 25 
4
;
;#	itemtext
6	;DLG_TEXT
15	;tlength
0	;tpos
Item:xxx of xxx
5 30 155 50
4
;
4	;DLG_LINE
0
0
---
159 0 159 227
0
;
4	;DLG_LINE
0
0
---
0 227 560 227
0
;
4	;DLG_LINE
0
0
---
159 103 560 103
0
;
;#	typemenu
9  ;DLG_MENU
9  
0
Item type
165 205 290 225
0
;
;#	gridmenu
9  ;DLG_MENU
7   
0
Snap:01
295 205 420 225
0
;
;#	objnummenu
9  ;DLG_MENU
11  
0
Object #:01
425 180 550 200
0
;
;#	popbutton
11	;DLG_BUTTON
8	;
0	;
Pop item
165 180 290 200 
0
;
;#	pushbutton
11	;DLG_BUTTON
9	;
0	;
Push item
295 180 420 200 
0
;
;#	shownumchk
10	;DLG_CHECK
11
0
Show number
165 155 290 175
1
;
;#	xitemchk
10	;DLG_CHECK
10
0
X cur item
295 155 420 175
1
;
;#	newitem
11	;DLG_BUTTON
8	;
0	;
New item
165 130 290 150 
0
;
;#	deleteitem
11	;DLG_BUTTON
11	;
0	;
Delete item
295 130 420 150 
0
;
6	;DLG_TEXT
6	;tlength
0	;tpos
Label:
165 105 240 125
8
;
;#	labeledit
7	;DLG_EDIT
7	;tlength
7	;tpos
unknown
245 105 555 125
1	;framed
;
6	;DLG_TEXT
31	;tlength
0	;tpos
Resedit ver 1.0 By:Randy Frank
5 230 555 250
1
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
;	Dialog number 2  (Line item)
;
1	;DLG_HEADER
0
0
------
160 0 560 105   ;bounding box
1 	; do NOT paint the box as dialog 1 did it for us
;
6	;DLG_TEXT
9
0
Line Item
165 80 300 100
4
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
;	Dialog number 3  (box item)
;
1	;DLG_HEADER
0
0
------
160 0 560 105   ;bounding box
1 	; do NOT paint the box as dialog 1 did it for us
;
6	;DLG_TEXT
8
0
Box Item
165 80 300 100
4
;
;#	boxframed
10	;DLG_CHECK
6 
0
Framed
165 55 290 75
1
;
;#	boxfilled
10	;DLG_CHECK
6
0
Filled
295 55 420 75
1
;
;#	boxcolor
9  ;DLG_MENU
10  
0
Fill color
425 55 550 75
0
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
;	Dialog number 4  (text item)
;
1	;DLG_HEADER
0
0
------
160 0 560 105   ;bounding box
1 	; do NOT paint the box as dialog 1 did it for us
;
6	;DLG_TEXT
9
0
Text Item
165 80 300 100
4
;
6	;DLG_TEXT
5
0
Text:
165 55 210 75 
8
;
;#	textitem
7	;DLG_EDIT
3
3
---
215 55 555 75 
1
;
;#	textclickable
10	;DLG_CHECK
9
0
Clickable
165 5 290 25
0
;
;#	textframed
10	;DLG_CHECK
6
0
Framed
295 5 420 25
1
;
;#	texthighlighted
10	;DLG_CHECK
11
0
Highlighted
425 5 550 25
1
;
;#	textfont
9  ;DLG_MENU
4  
0
Font
425 80 550 100
0
;
;#	textjustify
13	;DLG_RADIO
5
0
Left
165 30 290 50
1
;
13	;DLG_RADIO
6
0
Center
295 30 420 50
0
;
13	;DLG_RADIO
5
0
Right
425 30 550 50
0
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
;	Dialog number 5  (Edit item)
;
1	;DLG_HEADER
0
0
------
160 0 560 105   ;bounding box
1 	; do NOT paint the box as dialog 1 did it for us
;
6	;DLG_TEXT
9
0
Edit Item
165 80 300 100
4
;
6	;DLG_TEXT
5
0
Text:
165 55 210 75 
8
;
7	;DLG_EDIT
3
3
---
215 55 555 75 
1
;
;#	editframed
10	;DLG_CHECK
6 
0
Framed
165 30 290 50
1
;
;#	editfont
9  ;DLG_MENU
4  
0
Font
295 30 420 50 
0
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
;	Dialog number 6  (Scrollbar item)
;
1	;DLG_HEADER
0
0
------
160 0 560 105   ;bounding box
1 	; do NOT paint the box as dialog 1 did it for us
;
6	;DLG_TEXT
14
0
Scrollbar Item
165 80 300 100
4
;
6	;DLG_TEXT
10
0
Max Value:
165 55 275 75
8
;
6	;DLG_TEXT
10
0
Min Value:
165 30 275 50
8
;
6	;DLG_TEXT
11
0
Page Value:
365 55 475 75
8
;
6	;DLG_TEXT
11
0
Line Value:
365 30 475 50
8
;
;#	sbaredits
7	;DLG_EDIT
1
1	
-
280 55 350 75
1
;
7	;DLG_EDIT
1
1	
-
280 30 350 50
1
;
7	;DLG_EDIT
1
1	
-
480 55 550 75
1
;
7	;DLG_EDIT
1
1	
-
480 30 550 50
1
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
;	Dialog number 7  (Menu item)
;
1	;DLG_HEADER
0
0
------
160 0 560 105   ;bounding box
1 	; do NOT paint the box as dialog 1 did it for us
;
6	;DLG_TEXT
15
0
Popup Menu Item
165 80 300 100
4
;
6	;DLG_TEXT
5
0
Text:
165 55 210 75 
8
;
7	;DLG_EDIT
3
3
---
215 55 555 75 
1
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
;	Dialog number 8  (Checkbox item)
;
1	;DLG_HEADER
0
0
------
160 0 560 105   ;bounding box
1 	; do NOT paint the box as dialog 1 did it for us
;
6	;DLG_TEXT
13
0
Checkbox Item
165 80 300 100
4
;
6	;DLG_TEXT
5
0
Text:
165 55 210 75 
8
;
7	;DLG_EDIT
3
3
---
215 55 555 75 
1
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
;	Dialog number 9  (Button item)
;
1	;DLG_HEADER
0
0
------
160 0 560 105   ;bounding box
1 	; do NOT paint the box as dialog 1 did it for us
;
6	;DLG_TEXT
11
0
Button Item
165 80 300 100
4
;
6	;DLG_TEXT
5
0
Text:
165 55 210 75 
8
;
7	;DLG_EDIT
3
3
---
215 55 555 75 
1
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
;	Dialog number 10  (NOP item)
;
1	;DLG_HEADER
0
0
------
160 0 560 105   ;bounding box
1 	; do NOT paint the box as dialog 1 did it for us
;
6	;DLG_TEXT
17
0
No Operation Item
165 80 300 100
4
;
3	;DLG_END
0
0
---
0 0 0 0
0
;
;	Dialog number 10 (Radio item)
;
1	;DLG_HEADER
0
0
------
160 0 560 105   ;bounding box
1 	; do NOT paint the box as dialog 1 did it for us
;
6	;DLG_TEXT
17
0
Radio Button Item
165 80 300 100
4
;
6	;DLG_TEXT
5
0
Text:
165 55 210 75 
8
;
7	;DLG_EDIT
3
3
---
215 55 555 75 
1
;
3	;DLG_END
0
0
---
0 0 0 0
0
