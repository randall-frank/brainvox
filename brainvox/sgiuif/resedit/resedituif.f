***************************************************
*    Resource file converted to FORTRAN by rmaker *
*    Source file:resedituif.r                     *
*    File contained:    11 dialog(s).             *
*    Compiled at:Fri Jun 21 14:48:39 1991         *
**interface****************************************
       Subroutine get_dlg_comp(file,num,arrlen,arr,stat)
       Implicit None
**includes**************
       Include "sgiuif.h"
**declarations**********
       Character*80  file   !ignored by this routine
       Integer*4     num    !dlg # to read
       Integer*4     arrlen !size of arr
       Record/ditem/ arr(*) !place to store dlg
       Integer*4     stat   !returned status
**code******************
       stat=0           
       If (num .gt.    11) then 
            stat=-1
            Return
       Endif
       If ((num .eq.     1).and.(arrlen .ge.    27)) then
**Dialog     1 with    27 items*****
**DlgItem     1 of    27 type=DLG_HEADER*****
             arr(    1).dtype  =    1
             arr(    1).tlength=    0
             arr(    1).tpos   =    0
             arr(    1).text   ="------              "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    1).rect(1)=    0
             arr(    1).rect(2)=  255
             arr(    1).rect(3)=  560
             arr(    1).rect(4)=    0
             arr(    1).aux    =    0
**DlgItem     2 of    27 type=DLG_COLOR *****
             arr(    2).dtype  =    2
             arr(    2).tlength=    0
             arr(    2).tpos   =    0
             arr(    2).text   ="00000000012812812800"//
     +                 "00330330000520520001"//
     +                 "10110075113113255255"//
     +                 "255084084084        "
             arr(    2).rect(1)=    0
             arr(    2).rect(2)=    0
             arr(    2).rect(3)=    0
             arr(    2).rect(4)=    0
             arr(    2).aux    =    0
**DlgItem     3 of    27 type=DLG_BUTTON*****
             arr(    3).dtype  =   11
             arr(    3).tlength=    8
             arr(    3).tpos   =    0
             arr(    3).text   ="New file            "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    3).rect(1)=    5
             arr(    3).rect(2)=  225
             arr(    3).rect(3)=  155
             arr(    3).rect(4)=  205
             arr(    3).aux    =    0
**DlgItem     4 of    27 type=DLG_BUTTON*****
             arr(    4).dtype  =   11
             arr(    4).tlength=    9
             arr(    4).tpos   =    0
             arr(    4).text   ="Open file           "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    4).rect(1)=    5
             arr(    4).rect(2)=  200
             arr(    4).rect(3)=  155
             arr(    4).rect(4)=  180
             arr(    4).aux    =    0
**DlgItem     5 of    27 type=DLG_BUTTON*****
             arr(    5).dtype  =   11
             arr(    5).tlength=    9
             arr(    5).tpos   =    0
             arr(    5).text   ="Save file           "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    5).rect(1)=    5
             arr(    5).rect(2)=  175
             arr(    5).rect(3)=  155
             arr(    5).rect(4)=  155
             arr(    5).aux    =    0
**DlgItem     6 of    27 type=DLG_BUTTON*****
             arr(    6).dtype  =   11
             arr(    6).tlength=   11
             arr(    6).tpos   =    0
             arr(    6).text   ="Append file         "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    6).rect(1)=    5
             arr(    6).rect(2)=  150
             arr(    6).rect(3)=  155
             arr(    6).rect(4)=  130
             arr(    6).aux    =    0
**DlgItem     7 of    27 type=DLG_BUTTON*****
             arr(    7).dtype  =   11
             arr(    7).tlength=    4
             arr(    7).tpos   =    0
             arr(    7).text   ="Quit                "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    7).rect(1)=    5
             arr(    7).rect(2)=  125
             arr(    7).rect(3)=  155
             arr(    7).rect(4)=  105
             arr(    7).aux    =    0
**DlgItem     8 of    27 type=DLG_TEXT  *****
             arr(    8).dtype  =    6
             arr(    8).tlength=    7
             arr(    8).tpos   =    0
             arr(    8).text   ="Window:             "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    8).rect(1)=    5
             arr(    8).rect(2)=  100
             arr(    8).rect(3)=  155
             arr(    8).rect(4)=   80
             arr(    8).aux    =    4
**DlgItem     9 of    27 type=DLG_TEXT  *****
             arr(    9).dtype  =    6
             arr(    9).tlength=   11
             arr(    9).tpos   =    0
             arr(    9).text   ="(xxxx,xxxx)         "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    9).rect(1)=    5
             arr(    9).rect(2)=   75
             arr(    9).rect(3)=  155
             arr(    9).rect(4)=   55
             arr(    9).aux    =    4
**DlgItem    10 of    27 type=DLG_TEXT  *****
             arr(   10).dtype  =    6
             arr(   10).tlength=   15
             arr(   10).tpos   =    0
             arr(   10).text   ="xxx,xxx-xxx,xxx     "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(   10).rect(1)=    5
             arr(   10).rect(2)=   25
             arr(   10).rect(3)=  155
             arr(   10).rect(4)=    5
             arr(   10).aux    =    4
**DlgItem    11 of    27 type=DLG_TEXT  *****
             arr(   11).dtype  =    6
             arr(   11).tlength=   15
             arr(   11).tpos   =    0
             arr(   11).text   ="Item:xxx of xxx     "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(   11).rect(1)=    5
             arr(   11).rect(2)=   50
             arr(   11).rect(3)=  155
             arr(   11).rect(4)=   30
             arr(   11).aux    =    4
**DlgItem    12 of    27 type=DLG_LINE  *****
             arr(   12).dtype  =    4
             arr(   12).tlength=    0
             arr(   12).tpos   =    0
             arr(   12).text   ="---                 "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(   12).rect(1)=  159
             arr(   12).rect(2)=  227
             arr(   12).rect(3)=  159
             arr(   12).rect(4)=    0
             arr(   12).aux    =    0
**DlgItem    13 of    27 type=DLG_LINE  *****
             arr(   13).dtype  =    4
             arr(   13).tlength=    0
             arr(   13).tpos   =    0
             arr(   13).text   ="---                 "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(   13).rect(1)=    0
             arr(   13).rect(2)=  227
             arr(   13).rect(3)=  560
             arr(   13).rect(4)=  227
             arr(   13).aux    =    0
**DlgItem    14 of    27 type=DLG_LINE  *****
             arr(   14).dtype  =    4
             arr(   14).tlength=    0
             arr(   14).tpos   =    0
             arr(   14).text   ="---                 "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(   14).rect(1)=  159
             arr(   14).rect(2)=  103
             arr(   14).rect(3)=  560
             arr(   14).rect(4)=  103
             arr(   14).aux    =    0
**DlgItem    15 of    27 type=DLG_MENU  *****
             arr(   15).dtype  =    9
             arr(   15).tlength=    9
             arr(   15).tpos   =    0
             arr(   15).text   ="Item type           "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(   15).rect(1)=  165
             arr(   15).rect(2)=  225
             arr(   15).rect(3)=  290
             arr(   15).rect(4)=  205
             arr(   15).aux    =    0
**DlgItem    16 of    27 type=DLG_MENU  *****
             arr(   16).dtype  =    9
             arr(   16).tlength=    7
             arr(   16).tpos   =    0
             arr(   16).text   ="Snap:01             "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(   16).rect(1)=  295
             arr(   16).rect(2)=  225
             arr(   16).rect(3)=  420
             arr(   16).rect(4)=  205
             arr(   16).aux    =    0
**DlgItem    17 of    27 type=DLG_MENU  *****
             arr(   17).dtype  =    9
             arr(   17).tlength=   11
             arr(   17).tpos   =    0
             arr(   17).text   ="Object #:01         "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(   17).rect(1)=  425
             arr(   17).rect(2)=  200
             arr(   17).rect(3)=  550
             arr(   17).rect(4)=  180
             arr(   17).aux    =    0
**DlgItem    18 of    27 type=DLG_BUTTON*****
             arr(   18).dtype  =   11
             arr(   18).tlength=    8
             arr(   18).tpos   =    0
             arr(   18).text   ="Pop item            "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(   18).rect(1)=  165
             arr(   18).rect(2)=  200
             arr(   18).rect(3)=  290
             arr(   18).rect(4)=  180
             arr(   18).aux    =    0
**DlgItem    19 of    27 type=DLG_BUTTON*****
             arr(   19).dtype  =   11
             arr(   19).tlength=    9
             arr(   19).tpos   =    0
             arr(   19).text   ="Push item           "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(   19).rect(1)=  295
             arr(   19).rect(2)=  200
             arr(   19).rect(3)=  420
             arr(   19).rect(4)=  180
             arr(   19).aux    =    0
**DlgItem    20 of    27 type=DLG_CHECK *****
             arr(   20).dtype  =   10
             arr(   20).tlength=   11
             arr(   20).tpos   =    0
             arr(   20).text   ="Show number         "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(   20).rect(1)=  165
             arr(   20).rect(2)=  175
             arr(   20).rect(3)=  290
             arr(   20).rect(4)=  155
             arr(   20).aux    =    1
**DlgItem    21 of    27 type=DLG_CHECK *****
             arr(   21).dtype  =   10
             arr(   21).tlength=   10
             arr(   21).tpos   =    0
             arr(   21).text   ="X cur item          "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(   21).rect(1)=  295
             arr(   21).rect(2)=  175
             arr(   21).rect(3)=  420
             arr(   21).rect(4)=  155
             arr(   21).aux    =    1
**DlgItem    22 of    27 type=DLG_BUTTON*****
             arr(   22).dtype  =   11
             arr(   22).tlength=    8
             arr(   22).tpos   =    0
             arr(   22).text   ="New item            "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(   22).rect(1)=  165
             arr(   22).rect(2)=  150
             arr(   22).rect(3)=  290
             arr(   22).rect(4)=  130
             arr(   22).aux    =    0
**DlgItem    23 of    27 type=DLG_BUTTON*****
             arr(   23).dtype  =   11
             arr(   23).tlength=   11
             arr(   23).tpos   =    0
             arr(   23).text   ="Delete item         "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(   23).rect(1)=  295
             arr(   23).rect(2)=  150
             arr(   23).rect(3)=  420
             arr(   23).rect(4)=  130
             arr(   23).aux    =    0
**DlgItem    24 of    27 type=DLG_TEXT  *****
             arr(   24).dtype  =    6
             arr(   24).tlength=    6
             arr(   24).tpos   =    0
             arr(   24).text   ="Label:              "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(   24).rect(1)=  165
             arr(   24).rect(2)=  125
             arr(   24).rect(3)=  240
             arr(   24).rect(4)=  105
             arr(   24).aux    =    8
**DlgItem    25 of    27 type=DLG_EDIT  *****
             arr(   25).dtype  =    7
             arr(   25).tlength=    7
             arr(   25).tpos   =    7
             arr(   25).text   ="unknown             "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(   25).rect(1)=  245
             arr(   25).rect(2)=  125
             arr(   25).rect(3)=  555
             arr(   25).rect(4)=  105
             arr(   25).aux    =    1
**DlgItem    26 of    27 type=DLG_TEXT  *****
             arr(   26).dtype  =    6
             arr(   26).tlength=   31
             arr(   26).tpos   =    0
             arr(   26).text   ="Resedit ver 1.0 By:R"//
     +                 "andy Frank          "//
     +                 "                    "//
     +                 "                    "
             arr(   26).rect(1)=    5
             arr(   26).rect(2)=  250
             arr(   26).rect(3)=  555
             arr(   26).rect(4)=  230
             arr(   26).aux    =    1
**DlgItem    27 of    27 type=DLG_END   *****
             arr(   27).dtype  =    3
             arr(   27).tlength=    0
             arr(   27).tpos   =    0
             arr(   27).text   ="---                 "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(   27).rect(1)=    0
             arr(   27).rect(2)=    0
             arr(   27).rect(3)=    0
             arr(   27).rect(4)=    0
             arr(   27).aux    =    0
             Return
       elseif ((num .eq.     2).and.(arrlen .ge.     3)) then
**Dialog     2 with     3 items*****
**DlgItem     1 of     3 type=DLG_HEADER*****
             arr(    1).dtype  =    1
             arr(    1).tlength=    0
             arr(    1).tpos   =    0
             arr(    1).text   ="------              "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    1).rect(1)=  160
             arr(    1).rect(2)=  105
             arr(    1).rect(3)=  560
             arr(    1).rect(4)=    0
             arr(    1).aux    =    1
**DlgItem     2 of     3 type=DLG_TEXT  *****
             arr(    2).dtype  =    6
             arr(    2).tlength=    9
             arr(    2).tpos   =    0
             arr(    2).text   ="Line Item           "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    2).rect(1)=  165
             arr(    2).rect(2)=  100
             arr(    2).rect(3)=  300
             arr(    2).rect(4)=   80
             arr(    2).aux    =    4
**DlgItem     3 of     3 type=DLG_END   *****
             arr(    3).dtype  =    3
             arr(    3).tlength=    0
             arr(    3).tpos   =    0
             arr(    3).text   ="---                 "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    3).rect(1)=    0
             arr(    3).rect(2)=    0
             arr(    3).rect(3)=    0
             arr(    3).rect(4)=    0
             arr(    3).aux    =    0
             Return
       elseif ((num .eq.     3).and.(arrlen .ge.     6)) then
**Dialog     3 with     6 items*****
**DlgItem     1 of     6 type=DLG_HEADER*****
             arr(    1).dtype  =    1
             arr(    1).tlength=    0
             arr(    1).tpos   =    0
             arr(    1).text   ="------              "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    1).rect(1)=  160
             arr(    1).rect(2)=  105
             arr(    1).rect(3)=  560
             arr(    1).rect(4)=    0
             arr(    1).aux    =    1
**DlgItem     2 of     6 type=DLG_TEXT  *****
             arr(    2).dtype  =    6
             arr(    2).tlength=    8
             arr(    2).tpos   =    0
             arr(    2).text   ="Box Item            "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    2).rect(1)=  165
             arr(    2).rect(2)=  100
             arr(    2).rect(3)=  300
             arr(    2).rect(4)=   80
             arr(    2).aux    =    4
**DlgItem     3 of     6 type=DLG_CHECK *****
             arr(    3).dtype  =   10
             arr(    3).tlength=    6
             arr(    3).tpos   =    0
             arr(    3).text   ="Framed              "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    3).rect(1)=  165
             arr(    3).rect(2)=   75
             arr(    3).rect(3)=  290
             arr(    3).rect(4)=   55
             arr(    3).aux    =    1
**DlgItem     4 of     6 type=DLG_CHECK *****
             arr(    4).dtype  =   10
             arr(    4).tlength=    6
             arr(    4).tpos   =    0
             arr(    4).text   ="Filled              "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    4).rect(1)=  295
             arr(    4).rect(2)=   75
             arr(    4).rect(3)=  420
             arr(    4).rect(4)=   55
             arr(    4).aux    =    1
**DlgItem     5 of     6 type=DLG_MENU  *****
             arr(    5).dtype  =    9
             arr(    5).tlength=   10
             arr(    5).tpos   =    0
             arr(    5).text   ="Fill color          "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    5).rect(1)=  425
             arr(    5).rect(2)=   75
             arr(    5).rect(3)=  550
             arr(    5).rect(4)=   55
             arr(    5).aux    =    0
**DlgItem     6 of     6 type=DLG_END   *****
             arr(    6).dtype  =    3
             arr(    6).tlength=    0
             arr(    6).tpos   =    0
             arr(    6).text   ="---                 "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    6).rect(1)=    0
             arr(    6).rect(2)=    0
             arr(    6).rect(3)=    0
             arr(    6).rect(4)=    0
             arr(    6).aux    =    0
             Return
       elseif ((num .eq.     4).and.(arrlen .ge.    12)) then
**Dialog     4 with    12 items*****
**DlgItem     1 of    12 type=DLG_HEADER*****
             arr(    1).dtype  =    1
             arr(    1).tlength=    0
             arr(    1).tpos   =    0
             arr(    1).text   ="------              "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    1).rect(1)=  160
             arr(    1).rect(2)=  105
             arr(    1).rect(3)=  560
             arr(    1).rect(4)=    0
             arr(    1).aux    =    1
**DlgItem     2 of    12 type=DLG_TEXT  *****
             arr(    2).dtype  =    6
             arr(    2).tlength=    9
             arr(    2).tpos   =    0
             arr(    2).text   ="Text Item           "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    2).rect(1)=  165
             arr(    2).rect(2)=  100
             arr(    2).rect(3)=  300
             arr(    2).rect(4)=   80
             arr(    2).aux    =    4
**DlgItem     3 of    12 type=DLG_TEXT  *****
             arr(    3).dtype  =    6
             arr(    3).tlength=    5
             arr(    3).tpos   =    0
             arr(    3).text   ="Text:               "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    3).rect(1)=  165
             arr(    3).rect(2)=   75
             arr(    3).rect(3)=  210
             arr(    3).rect(4)=   55
             arr(    3).aux    =    8
**DlgItem     4 of    12 type=DLG_EDIT  *****
             arr(    4).dtype  =    7
             arr(    4).tlength=    3
             arr(    4).tpos   =    3
             arr(    4).text   ="---                 "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    4).rect(1)=  215
             arr(    4).rect(2)=   75
             arr(    4).rect(3)=  555
             arr(    4).rect(4)=   55
             arr(    4).aux    =    1
**DlgItem     5 of    12 type=DLG_CHECK *****
             arr(    5).dtype  =   10
             arr(    5).tlength=    9
             arr(    5).tpos   =    0
             arr(    5).text   ="Clickable           "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    5).rect(1)=  165
             arr(    5).rect(2)=   25
             arr(    5).rect(3)=  290
             arr(    5).rect(4)=    5
             arr(    5).aux    =    0
**DlgItem     6 of    12 type=DLG_CHECK *****
             arr(    6).dtype  =   10
             arr(    6).tlength=    6
             arr(    6).tpos   =    0
             arr(    6).text   ="Framed              "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    6).rect(1)=  295
             arr(    6).rect(2)=   25
             arr(    6).rect(3)=  420
             arr(    6).rect(4)=    5
             arr(    6).aux    =    1
**DlgItem     7 of    12 type=DLG_CHECK *****
             arr(    7).dtype  =   10
             arr(    7).tlength=   11
             arr(    7).tpos   =    0
             arr(    7).text   ="Highlighted         "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    7).rect(1)=  425
             arr(    7).rect(2)=   25
             arr(    7).rect(3)=  550
             arr(    7).rect(4)=    5
             arr(    7).aux    =    1
**DlgItem     8 of    12 type=DLG_MENU  *****
             arr(    8).dtype  =    9
             arr(    8).tlength=    4
             arr(    8).tpos   =    0
             arr(    8).text   ="Font                "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    8).rect(1)=  425
             arr(    8).rect(2)=  100
             arr(    8).rect(3)=  550
             arr(    8).rect(4)=   80
             arr(    8).aux    =    0
**DlgItem     9 of    12 type=DLG_RADIO *****
             arr(    9).dtype  =   13
             arr(    9).tlength=    5
             arr(    9).tpos   =    0
             arr(    9).text   ="Left                "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    9).rect(1)=  165
             arr(    9).rect(2)=   50
             arr(    9).rect(3)=  290
             arr(    9).rect(4)=   30
             arr(    9).aux    =    1
**DlgItem    10 of    12 type=DLG_RADIO *****
             arr(   10).dtype  =   13
             arr(   10).tlength=    6
             arr(   10).tpos   =    0
             arr(   10).text   ="Center              "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(   10).rect(1)=  295
             arr(   10).rect(2)=   50
             arr(   10).rect(3)=  420
             arr(   10).rect(4)=   30
             arr(   10).aux    =    0
**DlgItem    11 of    12 type=DLG_RADIO *****
             arr(   11).dtype  =   13
             arr(   11).tlength=    5
             arr(   11).tpos   =    0
             arr(   11).text   ="Right               "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(   11).rect(1)=  425
             arr(   11).rect(2)=   50
             arr(   11).rect(3)=  550
             arr(   11).rect(4)=   30
             arr(   11).aux    =    0
**DlgItem    12 of    12 type=DLG_END   *****
             arr(   12).dtype  =    3
             arr(   12).tlength=    0
             arr(   12).tpos   =    0
             arr(   12).text   ="---                 "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(   12).rect(1)=    0
             arr(   12).rect(2)=    0
             arr(   12).rect(3)=    0
             arr(   12).rect(4)=    0
             arr(   12).aux    =    0
             Return
       elseif ((num .eq.     5).and.(arrlen .ge.     7)) then
**Dialog     5 with     7 items*****
**DlgItem     1 of     7 type=DLG_HEADER*****
             arr(    1).dtype  =    1
             arr(    1).tlength=    0
             arr(    1).tpos   =    0
             arr(    1).text   ="------              "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    1).rect(1)=  160
             arr(    1).rect(2)=  105
             arr(    1).rect(3)=  560
             arr(    1).rect(4)=    0
             arr(    1).aux    =    1
**DlgItem     2 of     7 type=DLG_TEXT  *****
             arr(    2).dtype  =    6
             arr(    2).tlength=    9
             arr(    2).tpos   =    0
             arr(    2).text   ="Edit Item           "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    2).rect(1)=  165
             arr(    2).rect(2)=  100
             arr(    2).rect(3)=  300
             arr(    2).rect(4)=   80
             arr(    2).aux    =    4
**DlgItem     3 of     7 type=DLG_TEXT  *****
             arr(    3).dtype  =    6
             arr(    3).tlength=    5
             arr(    3).tpos   =    0
             arr(    3).text   ="Text:               "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    3).rect(1)=  165
             arr(    3).rect(2)=   75
             arr(    3).rect(3)=  210
             arr(    3).rect(4)=   55
             arr(    3).aux    =    8
**DlgItem     4 of     7 type=DLG_EDIT  *****
             arr(    4).dtype  =    7
             arr(    4).tlength=    3
             arr(    4).tpos   =    3
             arr(    4).text   ="---                 "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    4).rect(1)=  215
             arr(    4).rect(2)=   75
             arr(    4).rect(3)=  555
             arr(    4).rect(4)=   55
             arr(    4).aux    =    1
**DlgItem     5 of     7 type=DLG_CHECK *****
             arr(    5).dtype  =   10
             arr(    5).tlength=    6
             arr(    5).tpos   =    0
             arr(    5).text   ="Framed              "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    5).rect(1)=  165
             arr(    5).rect(2)=   50
             arr(    5).rect(3)=  290
             arr(    5).rect(4)=   30
             arr(    5).aux    =    1
**DlgItem     6 of     7 type=DLG_MENU  *****
             arr(    6).dtype  =    9
             arr(    6).tlength=    4
             arr(    6).tpos   =    0
             arr(    6).text   ="Font                "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    6).rect(1)=  295
             arr(    6).rect(2)=   50
             arr(    6).rect(3)=  420
             arr(    6).rect(4)=   30
             arr(    6).aux    =    0
**DlgItem     7 of     7 type=DLG_END   *****
             arr(    7).dtype  =    3
             arr(    7).tlength=    0
             arr(    7).tpos   =    0
             arr(    7).text   ="---                 "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    7).rect(1)=    0
             arr(    7).rect(2)=    0
             arr(    7).rect(3)=    0
             arr(    7).rect(4)=    0
             arr(    7).aux    =    0
             Return
       elseif ((num .eq.     6).and.(arrlen .ge.    11)) then
**Dialog     6 with    11 items*****
**DlgItem     1 of    11 type=DLG_HEADER*****
             arr(    1).dtype  =    1
             arr(    1).tlength=    0
             arr(    1).tpos   =    0
             arr(    1).text   ="------              "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    1).rect(1)=  160
             arr(    1).rect(2)=  105
             arr(    1).rect(3)=  560
             arr(    1).rect(4)=    0
             arr(    1).aux    =    1
**DlgItem     2 of    11 type=DLG_TEXT  *****
             arr(    2).dtype  =    6
             arr(    2).tlength=   14
             arr(    2).tpos   =    0
             arr(    2).text   ="Scrollbar Item      "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    2).rect(1)=  165
             arr(    2).rect(2)=  100
             arr(    2).rect(3)=  300
             arr(    2).rect(4)=   80
             arr(    2).aux    =    4
**DlgItem     3 of    11 type=DLG_TEXT  *****
             arr(    3).dtype  =    6
             arr(    3).tlength=   10
             arr(    3).tpos   =    0
             arr(    3).text   ="Max Value:          "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    3).rect(1)=  165
             arr(    3).rect(2)=   75
             arr(    3).rect(3)=  275
             arr(    3).rect(4)=   55
             arr(    3).aux    =    8
**DlgItem     4 of    11 type=DLG_TEXT  *****
             arr(    4).dtype  =    6
             arr(    4).tlength=   10
             arr(    4).tpos   =    0
             arr(    4).text   ="Min Value:          "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    4).rect(1)=  165
             arr(    4).rect(2)=   50
             arr(    4).rect(3)=  275
             arr(    4).rect(4)=   30
             arr(    4).aux    =    8
**DlgItem     5 of    11 type=DLG_TEXT  *****
             arr(    5).dtype  =    6
             arr(    5).tlength=   11
             arr(    5).tpos   =    0
             arr(    5).text   ="Page Value:         "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    5).rect(1)=  365
             arr(    5).rect(2)=   75
             arr(    5).rect(3)=  475
             arr(    5).rect(4)=   55
             arr(    5).aux    =    8
**DlgItem     6 of    11 type=DLG_TEXT  *****
             arr(    6).dtype  =    6
             arr(    6).tlength=   11
             arr(    6).tpos   =    0
             arr(    6).text   ="Line Value:         "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    6).rect(1)=  365
             arr(    6).rect(2)=   50
             arr(    6).rect(3)=  475
             arr(    6).rect(4)=   30
             arr(    6).aux    =    8
**DlgItem     7 of    11 type=DLG_EDIT  *****
             arr(    7).dtype  =    7
             arr(    7).tlength=    1
             arr(    7).tpos   =    1
             arr(    7).text   ="-                   "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    7).rect(1)=  280
             arr(    7).rect(2)=   75
             arr(    7).rect(3)=  350
             arr(    7).rect(4)=   55
             arr(    7).aux    =    1
**DlgItem     8 of    11 type=DLG_EDIT  *****
             arr(    8).dtype  =    7
             arr(    8).tlength=    1
             arr(    8).tpos   =    1
             arr(    8).text   ="-                   "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    8).rect(1)=  280
             arr(    8).rect(2)=   50
             arr(    8).rect(3)=  350
             arr(    8).rect(4)=   30
             arr(    8).aux    =    1
**DlgItem     9 of    11 type=DLG_EDIT  *****
             arr(    9).dtype  =    7
             arr(    9).tlength=    1
             arr(    9).tpos   =    1
             arr(    9).text   ="-                   "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    9).rect(1)=  480
             arr(    9).rect(2)=   75
             arr(    9).rect(3)=  550
             arr(    9).rect(4)=   55
             arr(    9).aux    =    1
**DlgItem    10 of    11 type=DLG_EDIT  *****
             arr(   10).dtype  =    7
             arr(   10).tlength=    1
             arr(   10).tpos   =    1
             arr(   10).text   ="-                   "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(   10).rect(1)=  480
             arr(   10).rect(2)=   50
             arr(   10).rect(3)=  550
             arr(   10).rect(4)=   30
             arr(   10).aux    =    1
**DlgItem    11 of    11 type=DLG_END   *****
             arr(   11).dtype  =    3
             arr(   11).tlength=    0
             arr(   11).tpos   =    0
             arr(   11).text   ="---                 "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(   11).rect(1)=    0
             arr(   11).rect(2)=    0
             arr(   11).rect(3)=    0
             arr(   11).rect(4)=    0
             arr(   11).aux    =    0
             Return
       elseif ((num .eq.     7).and.(arrlen .ge.     5)) then
**Dialog     7 with     5 items*****
**DlgItem     1 of     5 type=DLG_HEADER*****
             arr(    1).dtype  =    1
             arr(    1).tlength=    0
             arr(    1).tpos   =    0
             arr(    1).text   ="------              "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    1).rect(1)=  160
             arr(    1).rect(2)=  105
             arr(    1).rect(3)=  560
             arr(    1).rect(4)=    0
             arr(    1).aux    =    1
**DlgItem     2 of     5 type=DLG_TEXT  *****
             arr(    2).dtype  =    6
             arr(    2).tlength=   15
             arr(    2).tpos   =    0
             arr(    2).text   ="Popup Menu Item     "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    2).rect(1)=  165
             arr(    2).rect(2)=  100
             arr(    2).rect(3)=  300
             arr(    2).rect(4)=   80
             arr(    2).aux    =    4
**DlgItem     3 of     5 type=DLG_TEXT  *****
             arr(    3).dtype  =    6
             arr(    3).tlength=    5
             arr(    3).tpos   =    0
             arr(    3).text   ="Text:               "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    3).rect(1)=  165
             arr(    3).rect(2)=   75
             arr(    3).rect(3)=  210
             arr(    3).rect(4)=   55
             arr(    3).aux    =    8
**DlgItem     4 of     5 type=DLG_EDIT  *****
             arr(    4).dtype  =    7
             arr(    4).tlength=    3
             arr(    4).tpos   =    3
             arr(    4).text   ="---                 "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    4).rect(1)=  215
             arr(    4).rect(2)=   75
             arr(    4).rect(3)=  555
             arr(    4).rect(4)=   55
             arr(    4).aux    =    1
**DlgItem     5 of     5 type=DLG_END   *****
             arr(    5).dtype  =    3
             arr(    5).tlength=    0
             arr(    5).tpos   =    0
             arr(    5).text   ="---                 "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    5).rect(1)=    0
             arr(    5).rect(2)=    0
             arr(    5).rect(3)=    0
             arr(    5).rect(4)=    0
             arr(    5).aux    =    0
             Return
       elseif ((num .eq.     8).and.(arrlen .ge.     5)) then
**Dialog     8 with     5 items*****
**DlgItem     1 of     5 type=DLG_HEADER*****
             arr(    1).dtype  =    1
             arr(    1).tlength=    0
             arr(    1).tpos   =    0
             arr(    1).text   ="------              "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    1).rect(1)=  160
             arr(    1).rect(2)=  105
             arr(    1).rect(3)=  560
             arr(    1).rect(4)=    0
             arr(    1).aux    =    1
**DlgItem     2 of     5 type=DLG_TEXT  *****
             arr(    2).dtype  =    6
             arr(    2).tlength=   13
             arr(    2).tpos   =    0
             arr(    2).text   ="Checkbox Item       "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    2).rect(1)=  165
             arr(    2).rect(2)=  100
             arr(    2).rect(3)=  300
             arr(    2).rect(4)=   80
             arr(    2).aux    =    4
**DlgItem     3 of     5 type=DLG_TEXT  *****
             arr(    3).dtype  =    6
             arr(    3).tlength=    5
             arr(    3).tpos   =    0
             arr(    3).text   ="Text:               "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    3).rect(1)=  165
             arr(    3).rect(2)=   75
             arr(    3).rect(3)=  210
             arr(    3).rect(4)=   55
             arr(    3).aux    =    8
**DlgItem     4 of     5 type=DLG_EDIT  *****
             arr(    4).dtype  =    7
             arr(    4).tlength=    3
             arr(    4).tpos   =    3
             arr(    4).text   ="---                 "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    4).rect(1)=  215
             arr(    4).rect(2)=   75
             arr(    4).rect(3)=  555
             arr(    4).rect(4)=   55
             arr(    4).aux    =    1
**DlgItem     5 of     5 type=DLG_END   *****
             arr(    5).dtype  =    3
             arr(    5).tlength=    0
             arr(    5).tpos   =    0
             arr(    5).text   ="---                 "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    5).rect(1)=    0
             arr(    5).rect(2)=    0
             arr(    5).rect(3)=    0
             arr(    5).rect(4)=    0
             arr(    5).aux    =    0
             Return
       elseif ((num .eq.     9).and.(arrlen .ge.     5)) then
**Dialog     9 with     5 items*****
**DlgItem     1 of     5 type=DLG_HEADER*****
             arr(    1).dtype  =    1
             arr(    1).tlength=    0
             arr(    1).tpos   =    0
             arr(    1).text   ="------              "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    1).rect(1)=  160
             arr(    1).rect(2)=  105
             arr(    1).rect(3)=  560
             arr(    1).rect(4)=    0
             arr(    1).aux    =    1
**DlgItem     2 of     5 type=DLG_TEXT  *****
             arr(    2).dtype  =    6
             arr(    2).tlength=   11
             arr(    2).tpos   =    0
             arr(    2).text   ="Button Item         "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    2).rect(1)=  165
             arr(    2).rect(2)=  100
             arr(    2).rect(3)=  300
             arr(    2).rect(4)=   80
             arr(    2).aux    =    4
**DlgItem     3 of     5 type=DLG_TEXT  *****
             arr(    3).dtype  =    6
             arr(    3).tlength=    5
             arr(    3).tpos   =    0
             arr(    3).text   ="Text:               "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    3).rect(1)=  165
             arr(    3).rect(2)=   75
             arr(    3).rect(3)=  210
             arr(    3).rect(4)=   55
             arr(    3).aux    =    8
**DlgItem     4 of     5 type=DLG_EDIT  *****
             arr(    4).dtype  =    7
             arr(    4).tlength=    3
             arr(    4).tpos   =    3
             arr(    4).text   ="---                 "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    4).rect(1)=  215
             arr(    4).rect(2)=   75
             arr(    4).rect(3)=  555
             arr(    4).rect(4)=   55
             arr(    4).aux    =    1
**DlgItem     5 of     5 type=DLG_END   *****
             arr(    5).dtype  =    3
             arr(    5).tlength=    0
             arr(    5).tpos   =    0
             arr(    5).text   ="---                 "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    5).rect(1)=    0
             arr(    5).rect(2)=    0
             arr(    5).rect(3)=    0
             arr(    5).rect(4)=    0
             arr(    5).aux    =    0
             Return
       elseif ((num .eq.    10).and.(arrlen .ge.     3)) then
**Dialog    10 with     3 items*****
**DlgItem     1 of     3 type=DLG_HEADER*****
             arr(    1).dtype  =    1
             arr(    1).tlength=    0
             arr(    1).tpos   =    0
             arr(    1).text   ="------              "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    1).rect(1)=  160
             arr(    1).rect(2)=  105
             arr(    1).rect(3)=  560
             arr(    1).rect(4)=    0
             arr(    1).aux    =    1
**DlgItem     2 of     3 type=DLG_TEXT  *****
             arr(    2).dtype  =    6
             arr(    2).tlength=   17
             arr(    2).tpos   =    0
             arr(    2).text   ="No Operation Item   "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    2).rect(1)=  165
             arr(    2).rect(2)=  100
             arr(    2).rect(3)=  300
             arr(    2).rect(4)=   80
             arr(    2).aux    =    4
**DlgItem     3 of     3 type=DLG_END   *****
             arr(    3).dtype  =    3
             arr(    3).tlength=    0
             arr(    3).tpos   =    0
             arr(    3).text   ="---                 "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    3).rect(1)=    0
             arr(    3).rect(2)=    0
             arr(    3).rect(3)=    0
             arr(    3).rect(4)=    0
             arr(    3).aux    =    0
             Return
       elseif ((num .eq.    11).and.(arrlen .ge.     5)) then
**Dialog    11 with     5 items*****
**DlgItem     1 of     5 type=DLG_HEADER*****
             arr(    1).dtype  =    1
             arr(    1).tlength=    0
             arr(    1).tpos   =    0
             arr(    1).text   ="------              "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    1).rect(1)=  160
             arr(    1).rect(2)=  105
             arr(    1).rect(3)=  560
             arr(    1).rect(4)=    0
             arr(    1).aux    =    1
**DlgItem     2 of     5 type=DLG_TEXT  *****
             arr(    2).dtype  =    6
             arr(    2).tlength=   17
             arr(    2).tpos   =    0
             arr(    2).text   ="Radio Button Item   "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    2).rect(1)=  165
             arr(    2).rect(2)=  100
             arr(    2).rect(3)=  300
             arr(    2).rect(4)=   80
             arr(    2).aux    =    4
**DlgItem     3 of     5 type=DLG_TEXT  *****
             arr(    3).dtype  =    6
             arr(    3).tlength=    5
             arr(    3).tpos   =    0
             arr(    3).text   ="Text:               "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    3).rect(1)=  165
             arr(    3).rect(2)=   75
             arr(    3).rect(3)=  210
             arr(    3).rect(4)=   55
             arr(    3).aux    =    8
**DlgItem     4 of     5 type=DLG_EDIT  *****
             arr(    4).dtype  =    7
             arr(    4).tlength=    3
             arr(    4).tpos   =    3
             arr(    4).text   ="---                 "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    4).rect(1)=  215
             arr(    4).rect(2)=   75
             arr(    4).rect(3)=  555
             arr(    4).rect(4)=   55
             arr(    4).aux    =    1
**DlgItem     5 of     5 type=DLG_END   *****
             arr(    5).dtype  =    3
             arr(    5).tlength=    0
             arr(    5).tpos   =    0
             arr(    5).text   ="---                 "//
     +                 "                    "//
     +                 "                    "//
     +                 "                    "
             arr(    5).rect(1)=    0
             arr(    5).rect(2)=    0
             arr(    5).rect(3)=    0
             arr(    5).rect(4)=    0
             arr(    5).aux    =    0
             Return
       Endif
       Stat=-1
       Return
       End
** End of file :Fri Jun 21 14:48:40 1991*****
