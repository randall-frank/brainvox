***************************************************
*    Resource file converted to FORTRAN by rmaker *
*    Source file:hello.res                        *
*    File contained:     1 dialog(s).             *
*    Compiled at:Sun Jun 04 11:14:29 2006         *
**interface****************************************
       Subroutine get_dlg_comp(file,num,arrlen,arr,stat)
       Implicit None
**includes**************
       Include "sgiuif.h"
**declarations**********
       Character*200 file   !ignored by this routine
       Integer*4     num    !dlg # to read
       Integer*4     arrlen !size of arr
       Record/ditem/ arr(*) !place to store dlg
       Integer*4     stat   !returned status
**code******************
       stat=0           
       If (num .gt.     1) then 
            stat=-1
            Return
       Endif
       If ((num .eq.     1).and.(arrlen .ge.    18)) then
**Dialog     1 with    18 items*****
**DlgItem     1 of    18 type=DLG_HEADER*****
       arr(    1).dtype  =    1
       arr(    1).tlength=    0
       arr(    1).tpos   =    0
       arr(    1).text   ="------                                  "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "
       arr(    1).rect(1)=    0
       arr(    1).rect(2)=  511
       arr(    1).rect(3)=  511
       arr(    1).rect(4)=    0
       arr(    1).aux    =    0
**DlgItem     2 of    18 type=DLG_EDIT  *****
       arr(    2).dtype  =    7
       arr(    2).tlength=   25
       arr(    2).tpos   =    5
       arr(    2).text   ="hello	                                  "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "
       arr(    2).rect(1)=   10
       arr(    2).rect(2)=  160
       arr(    2).rect(3)=   -1
       arr(    2).rect(4)=   -1
       arr(    2).aux    =    1
**DlgItem     3 of    18 type=DLG_BUTTON*****
       arr(    3).dtype  =   11
       arr(    3).tlength=    2
       arr(    3).tpos   =    0
       arr(    3).text   ="Ok	                                     "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "
       arr(    3).rect(1)=   50
       arr(    3).rect(2)=  440
       arr(    3).rect(3)=  150
       arr(    3).rect(4)=  400
       arr(    3).aux    =    0
**DlgItem     4 of    18 type=DLG_BUTTON*****
       arr(    4).dtype  =   11
       arr(    4).tlength=    6
       arr(    4).tpos   =    0
       arr(    4).text   ="Cancel	                                 "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "
       arr(    4).rect(1)=  350
       arr(    4).rect(2)=  440
       arr(    4).rect(3)=  450
       arr(    4).rect(4)=  400
       arr(    4).aux    =    0
**DlgItem     5 of    18 type=DLG_SBAR  *****
       arr(    5).dtype  =    8
       arr(    5).tlength=  255
       arr(    5).tpos   =65546
       arr(    5).text   ="fjahdsfhkj                              "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "
       arr(    5).rect(1)=   30
       arr(    5).rect(2)=  240
       arr(    5).rect(3)=  190
       arr(    5).rect(4)=  210
       arr(    5).aux    =  128
**DlgItem     6 of    18 type=DLG_SBAR  *****
       arr(    6).dtype  =    8
       arr(    6).tlength=  255
       arr(    6).tpos   =65546
       arr(    6).text   ="fjahdsfhkj                              "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "
       arr(    6).rect(1)=  450
       arr(    6).rect(2)=  300
       arr(    6).rect(3)=  480
       arr(    6).rect(4)=   10
       arr(    6).aux    =  128
**DlgItem     7 of    18 type=DLG_MENU  *****
       arr(    7).dtype  =    9
       arr(    7).tlength=   10
       arr(    7).tpos   =    0
       arr(    7).text   ="fjahdsfhkj                              "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "
       arr(    7).rect(1)=   30
       arr(    7).rect(2)=   50
       arr(    7).rect(3)=   -1
       arr(    7).rect(4)=   -1
       arr(    7).aux    =    1
**DlgItem     8 of    18 type=DLG_CHECK *****
       arr(    8).dtype  =   10
       arr(    8).tlength=   10
       arr(    8).tpos   =    0
       arr(    8).text   ="fjahdsfhkj                              "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "
       arr(    8).rect(1)=   30
       arr(    8).rect(2)=  300
       arr(    8).rect(3)=   -1
       arr(    8).rect(4)=   -1
       arr(    8).aux    =    1
**DlgItem     9 of    18 type=DLG_CHECK *****
       arr(    9).dtype  =   10
       arr(    9).tlength=   10
       arr(    9).tpos   =    0
       arr(    9).text   ="fjahdsfhkj                              "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "
       arr(    9).rect(1)=   30
       arr(    9).rect(2)=  340
       arr(    9).rect(3)=   -1
       arr(    9).rect(4)=   -1
       arr(    9).aux    =    1
**DlgItem    10 of    18 type=DLG_CHECK *****
       arr(   10).dtype  =   10
       arr(   10).tlength=   10
       arr(   10).tpos   =    0
       arr(   10).text   ="fjahdsfhkj                              "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "
       arr(   10).rect(1)=   30
       arr(   10).rect(2)=  380
       arr(   10).rect(3)=   -1
       arr(   10).rect(4)=   -1
       arr(   10).aux    =    1
**DlgItem    11 of    18 type=DLG_RADIO *****
       arr(   11).dtype  =   13
       arr(   11).tlength=   10
       arr(   11).tpos   =    0
       arr(   11).text   ="fjahdsfhkj                              "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "
       arr(   11).rect(1)=  230
       arr(   11).rect(2)=  300
       arr(   11).rect(3)=   -1
       arr(   11).rect(4)=   -1
       arr(   11).aux    =    0
**DlgItem    12 of    18 type=DLG_RADIO *****
       arr(   12).dtype  =   13
       arr(   12).tlength=   10
       arr(   12).tpos   =    0
       arr(   12).text   ="fjahdsfhkj                              "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "
       arr(   12).rect(1)=  230
       arr(   12).rect(2)=  340
       arr(   12).rect(3)=   -1
       arr(   12).rect(4)=   -1
       arr(   12).aux    =    0
**DlgItem    13 of    18 type=DLG_RADIO *****
       arr(   13).dtype  =   13
       arr(   13).tlength=   10
       arr(   13).tpos   =    0
       arr(   13).text   ="fjahdsfhkj                              "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "
       arr(   13).rect(1)=  230
       arr(   13).rect(2)=  380
       arr(   13).rect(3)=   -1
       arr(   13).rect(4)=   -1
       arr(   13).aux    =    0
**DlgItem    14 of    18 type=DLG_EDIT  *****
       arr(   14).dtype  =    7
       arr(   14).tlength=   25
       arr(   14).tpos   =    5
       arr(   14).text   ="hello	                                  "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "
       arr(   14).rect(1)=   10
       arr(   14).rect(2)=  110
       arr(   14).rect(3)=   -1
       arr(   14).rect(4)=   -1
       arr(   14).aux    = 2561
**DlgItem    15 of    18 type=DLG_TEXT  *****
       arr(   15).dtype  =    6
       arr(   15).tlength=    1
       arr(   15).tpos   =    1
       arr(   15).text   ="A                                       "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "
       arr(   15).rect(1)=  240
       arr(   15).rect(2)=  240
       arr(   15).rect(3)=   -1
       arr(   15).rect(4)=   -1
       arr(   15).aux    = 1283
**DlgItem    16 of    18 type=DLG_TEXT  *****
       arr(   16).dtype  =    6
       arr(   16).tlength=   10
       arr(   16).tpos   =    1
       arr(   16).text   ="ABCDEFGHIJ                              "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "
       arr(   16).rect(1)=  240
       arr(   16).rect(2)=  100
       arr(   16).rect(3)=   -1
       arr(   16).rect(4)=   -1
       arr(   16).aux    = 1536
**DlgItem    17 of    18 type=DLG_TEXT  *****
       arr(   17).dtype  =    6
       arr(   17).tlength=   10
       arr(   17).tpos   =    1
       arr(   17).text   ="ABCDEFGHIJ                              "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "
       arr(   17).rect(1)=  240
       arr(   17).rect(2)=  160
       arr(   17).rect(3)=  420
       arr(   17).rect(4)=  130
       arr(   17).aux    = 1034
**DlgItem    18 of    18 type=DLG_END   *****
       arr(   18).dtype  =    3
       arr(   18).tlength=    0
       arr(   18).tpos   =    0
       arr(   18).text   ="---                                     "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "//
     +           "                                        "
       arr(   18).rect(1)=    0
       arr(   18).rect(2)=    0
       arr(   18).rect(3)=    0
       arr(   18).rect(4)=    0
       arr(   18).aux    =    0
             Return
       Endif
       stat=-1
       Return
       End
** End of file :Sun Jun 04 11:14:29 2006*****

