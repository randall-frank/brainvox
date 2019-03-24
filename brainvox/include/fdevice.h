C *************************************************************************
C 									  *
C  		 Copyright (C) 1984, Silicon Graphics, Inc.		  *
C 									  *
C   These coded instructions, statements, and computer programs  contain  *
C   unpublished  proprietary  information of Silicon Graphics, Inc., and  *
C   are protected by Federal copyright law.  They  may  not be disclosed  *
C   to  third  parties  or copied or duplicated in any form, in whole or  *
C   in part, without the prior written consent of Silicon Graphics, Inc.  *
C 									  *
C ************************************************************************

C 
C  Device name space partitioning
C 
C   hex 0000 -> hex 0fff	devices defined by SGI
C     hex 0001 -> hex 00ff	    buttons
C     hex 0100 -> hex 01ff	    valuators
C     hex 0200 -> hex 02ff	    pseudo devices
C     hex 0300 -> hex 0eff	    reserved
C     hex 0f00 -> hex 0fff	    additional buttons
C 
C   hex 1000 -> hex 7fff	devices defined by users
C     hex 1000 -> hex 2fff	    buttons
C     hex 3000 -> hex 3fff	    valuators
C     hex 4000 -> hex 7fff	    pseudo devices
C 
C   hex 8000 -> hex ffff	can not be used
C 





       integer*4   NULLDE
       parameter ( NULLDE = 0 ) 	
       integer*4   BUTOFF
       parameter ( BUTOFF = 1 ) 	
       integer*4   VALOFF
       parameter ( VALOFF = 256 ) 	
       integer*4   PSEUDO
       parameter ( PSEUDO = 512 ) 	
       integer*4   BUT2OF
       parameter ( BUT2OF = 3840 ) 	

       integer*4   TIMOFF
       parameter ( TIMOFF = 515 ) 	
       integer*4   XKBDOF
       parameter ( XKBDOF = 143 ) 	

       integer*4   BUTCOU
       parameter ( BUTCOU = 255 ) 	
       integer*4   VALCOU
       parameter ( VALCOU = 256 ) 	

       integer*4   TIMCOU
       parameter ( TIMCOU = 4 ) 	
       integer*4   XKBDCO
       parameter ( XKBDCO = 28 ) 	

       integer*4   USERBU
       parameter ( USERBU = 4096 ) 	
       integer*4   USERVA
       parameter ( USERVA = 12288 ) 	
       integer*4   USERPS
       parameter ( USERPS = 16384 ) 	

C 
C  Button definitions for the base US keyboards
C 
       integer*4   BUT0
       parameter ( BUT0 = 1 ) 	
       integer*4   BUT1
       parameter ( BUT1 = 2 ) 	
       integer*4   BUT2
       parameter ( BUT2 = 3 ) 	
       integer*4   BUT3
       parameter ( BUT3 = 4 ) 	
       integer*4   BUT4
       parameter ( BUT4 = 5 ) 	
       integer*4   BUT5
       parameter ( BUT5 = 6 ) 	
       integer*4   BUT6
       parameter ( BUT6 = 7 ) 	
       integer*4   BUT7
       parameter ( BUT7 = 8 ) 	
       integer*4   BUT8
       parameter ( BUT8 = 9 ) 	
       integer*4   BUT9
       parameter ( BUT9 = 10 ) 	
       integer*4   BUT10
       parameter ( BUT10 = 11 ) 	
       integer*4   BUT11
       parameter ( BUT11 = 12 ) 	
       integer*4   BUT12
       parameter ( BUT12 = 13 ) 	
       integer*4   BUT13
       parameter ( BUT13 = 14 ) 	
       integer*4   BUT14
       parameter ( BUT14 = 15 ) 	
       integer*4   BUT15
       parameter ( BUT15 = 16 ) 	
       integer*4   BUT16
       parameter ( BUT16 = 17 ) 	
       integer*4   BUT17
       parameter ( BUT17 = 18 ) 	
       integer*4   BUT18
       parameter ( BUT18 = 19 ) 	
       integer*4   BUT19
       parameter ( BUT19 = 20 ) 	
       integer*4   BUT20
       parameter ( BUT20 = 21 ) 	
       integer*4   BUT21
       parameter ( BUT21 = 22 ) 	
       integer*4   BUT22
       parameter ( BUT22 = 23 ) 	
       integer*4   BUT23
       parameter ( BUT23 = 24 ) 	
       integer*4   BUT24
       parameter ( BUT24 = 25 ) 	
       integer*4   BUT25
       parameter ( BUT25 = 26 ) 	
       integer*4   BUT26
       parameter ( BUT26 = 27 ) 	
       integer*4   BUT27
       parameter ( BUT27 = 28 ) 	
       integer*4   BUT28
       parameter ( BUT28 = 29 ) 	
       integer*4   BUT29
       parameter ( BUT29 = 30 ) 	
       integer*4   BUT30
       parameter ( BUT30 = 31 ) 	
       integer*4   BUT31
       parameter ( BUT31 = 32 ) 	
       integer*4   BUT32
       parameter ( BUT32 = 33 ) 	
       integer*4   BUT33
       parameter ( BUT33 = 34 ) 	
       integer*4   BUT34
       parameter ( BUT34 = 35 ) 	
       integer*4   BUT35
       parameter ( BUT35 = 36 ) 	
       integer*4   BUT36
       parameter ( BUT36 = 37 ) 	
       integer*4   BUT37
       parameter ( BUT37 = 38 ) 	
       integer*4   BUT38
       parameter ( BUT38 = 39 ) 	
       integer*4   BUT39
       parameter ( BUT39 = 40 ) 	
       integer*4   BUT40
       parameter ( BUT40 = 41 ) 	
       integer*4   BUT41
       parameter ( BUT41 = 42 ) 	
       integer*4   BUT42
       parameter ( BUT42 = 43 ) 	
       integer*4   BUT43
       parameter ( BUT43 = 44 ) 	
       integer*4   BUT44
       parameter ( BUT44 = 45 ) 	
       integer*4   BUT45
       parameter ( BUT45 = 46 ) 	
       integer*4   BUT46
       parameter ( BUT46 = 47 ) 	
       integer*4   BUT47
       parameter ( BUT47 = 48 ) 	
       integer*4   BUT48
       parameter ( BUT48 = 49 ) 	
       integer*4   BUT49
       parameter ( BUT49 = 50 ) 	
       integer*4   BUT50
       parameter ( BUT50 = 51 ) 	
       integer*4   BUT51
       parameter ( BUT51 = 52 ) 	
       integer*4   BUT52
       parameter ( BUT52 = 53 ) 	
       integer*4   BUT53
       parameter ( BUT53 = 54 ) 	
       integer*4   BUT54
       parameter ( BUT54 = 55 ) 	
       integer*4   BUT55
       parameter ( BUT55 = 56 ) 	
       integer*4   BUT56
       parameter ( BUT56 = 57 ) 	
       integer*4   BUT57
       parameter ( BUT57 = 58 ) 	
       integer*4   BUT58
       parameter ( BUT58 = 59 ) 	
       integer*4   BUT59
       parameter ( BUT59 = 60 ) 	
       integer*4   BUT60
       parameter ( BUT60 = 61 ) 	
       integer*4   BUT61
       parameter ( BUT61 = 62 ) 	
       integer*4   BUT62
       parameter ( BUT62 = 63 ) 	
       integer*4   BUT63
       parameter ( BUT63 = 64 ) 	
       integer*4   BUT64
       parameter ( BUT64 = 65 ) 	
       integer*4   BUT65
       parameter ( BUT65 = 66 ) 	
       integer*4   BUT66
       parameter ( BUT66 = 67 ) 	
       integer*4   BUT67
       parameter ( BUT67 = 68 ) 	
       integer*4   BUT68
       parameter ( BUT68 = 69 ) 	
       integer*4   BUT69
       parameter ( BUT69 = 70 ) 	
       integer*4   BUT70
       parameter ( BUT70 = 71 ) 	
       integer*4   BUT71
       parameter ( BUT71 = 72 ) 	
       integer*4   BUT72
       parameter ( BUT72 = 73 ) 	
       integer*4   BUT73
       parameter ( BUT73 = 74 ) 	
       integer*4   BUT74
       parameter ( BUT74 = 75 ) 	
       integer*4   BUT75
       parameter ( BUT75 = 76 ) 	
       integer*4   BUT76
       parameter ( BUT76 = 77 ) 	
       integer*4   BUT77
       parameter ( BUT77 = 78 ) 	
       integer*4   BUT78
       parameter ( BUT78 = 79 ) 	
       integer*4   BUT79
       parameter ( BUT79 = 80 ) 	
       integer*4   BUT80
       parameter ( BUT80 = 81 ) 	
       integer*4   BUT81
       parameter ( BUT81 = 82 ) 	
       integer*4   BUT82
       parameter ( BUT82 = 83 ) 	
       integer*4   MAXKBD
       parameter ( MAXKBD = 83 ) 	

C  Mouse buttons, etc. 
       integer*4   BUT100
       parameter ( BUT100 = 101 ) 	
       integer*4   BUT101
       parameter ( BUT101 = 102 ) 	
       integer*4   BUT102
       parameter ( BUT102 = 103 ) 	
       integer*4   BUT103
       parameter ( BUT103 = 104 ) 	
       integer*4   BUT104
       parameter ( BUT104 = 105 ) 	
       integer*4   BUT105
       parameter ( BUT105 = 106 ) 	
       integer*4   BUT106
       parameter ( BUT106 = 107 ) 	
       integer*4   BUT107
       parameter ( BUT107 = 108 ) 	
       integer*4   BUT108
       parameter ( BUT108 = 109 ) 	
       integer*4   BUT109
       parameter ( BUT109 = 110 ) 	

C  Button box definitions 
       integer*4   BUT110
       parameter ( BUT110 = 111 ) 	
       integer*4   BUT111
       parameter ( BUT111 = 112 ) 	
       integer*4   BUT112
       parameter ( BUT112 = 113 ) 	
       integer*4   BUT113
       parameter ( BUT113 = 114 ) 	
       integer*4   BUT114
       parameter ( BUT114 = 115 ) 	
       integer*4   BUT115
       parameter ( BUT115 = 116 ) 	
       integer*4   BUT116
       parameter ( BUT116 = 117 ) 	
       integer*4   BUT117
       parameter ( BUT117 = 118 ) 	
       integer*4   BUT118
       parameter ( BUT118 = 119 ) 	
       integer*4   BUT119
       parameter ( BUT119 = 120 ) 	
       integer*4   BUT120
       parameter ( BUT120 = 121 ) 	
       integer*4   BUT121
       parameter ( BUT121 = 122 ) 	
       integer*4   BUT122
       parameter ( BUT122 = 123 ) 	
       integer*4   BUT123
       parameter ( BUT123 = 124 ) 	
       integer*4   BUT124
       parameter ( BUT124 = 125 ) 	
       integer*4   BUT125
       parameter ( BUT125 = 126 ) 	
       integer*4   BUT126
       parameter ( BUT126 = 127 ) 	
       integer*4   BUT127
       parameter ( BUT127 = 128 ) 	
       integer*4   BUT128
       parameter ( BUT128 = 129 ) 	
       integer*4   BUT129
       parameter ( BUT129 = 130 ) 	
       integer*4   BUT130
       parameter ( BUT130 = 131 ) 	
       integer*4   BUT131
       parameter ( BUT131 = 132 ) 	
       integer*4   BUT132
       parameter ( BUT132 = 133 ) 	
       integer*4   BUT133
       parameter ( BUT133 = 134 ) 	
       integer*4   BUT134
       parameter ( BUT134 = 135 ) 	
       integer*4   BUT135
       parameter ( BUT135 = 136 ) 	
       integer*4   BUT136
       parameter ( BUT136 = 137 ) 	
       integer*4   BUT137
       parameter ( BUT137 = 138 ) 	
       integer*4   BUT138
       parameter ( BUT138 = 139 ) 	
       integer*4   BUT139
       parameter ( BUT139 = 140 ) 	
       integer*4   BUT140
       parameter ( BUT140 = 141 ) 	
       integer*4   BUT141
       parameter ( BUT141 = 142 ) 	

C  Button definitions for the extended keyboard.  Although current keyboards
C  are 101 or 102 keys, there are 112 positions and so that many values are
C  reserved.
C 
       integer*4   BUT142
       parameter ( BUT142 = 143 ) 	
       integer*4   BUT143
       parameter ( BUT143 = 144 ) 	
       integer*4   BUT144
       parameter ( BUT144 = 145 ) 	
       integer*4   BUT145
       parameter ( BUT145 = 146 ) 	
       integer*4   BUT146
       parameter ( BUT146 = 147 ) 	
       integer*4   BUT147
       parameter ( BUT147 = 148 ) 	
       integer*4   BUT148
       parameter ( BUT148 = 149 ) 	
       integer*4   BUT149
       parameter ( BUT149 = 150 ) 	
       integer*4   BUT150
       parameter ( BUT150 = 151 ) 	
       integer*4   BUT151
       parameter ( BUT151 = 152 ) 	
       integer*4   BUT152
       parameter ( BUT152 = 153 ) 	
       integer*4   BUT153
       parameter ( BUT153 = 154 ) 	
       integer*4   BUT154
       parameter ( BUT154 = 155 ) 	
       integer*4   BUT155
       parameter ( BUT155 = 156 ) 	
       integer*4   BUT156
       parameter ( BUT156 = 157 ) 	
       integer*4   BUT157
       parameter ( BUT157 = 158 ) 	
       integer*4   BUT158
       parameter ( BUT158 = 159 ) 	
       integer*4   BUT159
       parameter ( BUT159 = 160 ) 	
       integer*4   BUT160
       parameter ( BUT160 = 161 ) 	
       integer*4   BUT161
       parameter ( BUT161 = 162 ) 	
       integer*4   BUT162
       parameter ( BUT162 = 163 ) 	
       integer*4   BUT163
       parameter ( BUT163 = 164 ) 	
       integer*4   BUT164
       parameter ( BUT164 = 165 ) 	
       integer*4   BUT165
       parameter ( BUT165 = 166 ) 	
       integer*4   BUT166
       parameter ( BUT166 = 167 ) 	
       integer*4   BUT167
       parameter ( BUT167 = 168 ) 	
       integer*4   BUT168
       parameter ( BUT168 = 169 ) 	

C 
C  BUT169 (=170) through BUT179 (=180) are reserved for the remainder of
C  the 112 key positions.
C 

C 
C  BUT181 through BUT189 are used for the nine buttons of the Space Ball.
C 
C  Codes through 255 inclusive are reserved for future use by SGI.
C 
       integer*4   BUT181
       parameter ( BUT181 = 182 ) 	
       integer*4   BUT182
       parameter ( BUT182 = 183 ) 	
       integer*4   BUT183
       parameter ( BUT183 = 184 ) 	
       integer*4   BUT184
       parameter ( BUT184 = 185 ) 	
       integer*4   BUT185
       parameter ( BUT185 = 186 ) 	
       integer*4   BUT186
       parameter ( BUT186 = 187 ) 	
       integer*4   BUT187
       parameter ( BUT187 = 188 ) 	
       integer*4   BUT188
       parameter ( BUT188 = 189 ) 	
       integer*4   BUT189
       parameter ( BUT189 = 190 ) 	

C  other buttons 

       integer*4   MOUSE1
       parameter ( MOUSE1 = 101 ) 	
       integer*4   MOUSE2
       parameter ( MOUSE2 = 102 ) 	
       integer*4   MOUSE3
       parameter ( MOUSE3 = 103 ) 	
C       integer*4   LEFTMO
C       parameter ( LEFTMO = 103 ) 	
C       integer*4   MIDDLE
C       parameter ( MIDDLE = 102 ) 	
C       integer*4   RIGHTM
C       parameter ( RIGHTM = 101 ) 	
       integer*4   LPENBU
       parameter ( LPENBU = 104 ) 	
       integer*4   BPAD0
       parameter ( BPAD0 = 105 ) 	
       integer*4   BPAD1
       parameter ( BPAD1 = 106 ) 	
       integer*4   BPAD2
       parameter ( BPAD2 = 107 ) 	
       integer*4   BPAD3
       parameter ( BPAD3 = 108 ) 	
       integer*4   LPENVA
       parameter ( LPENVA = 109 ) 	

C  button box 

       integer*4   SWBASE
       parameter ( SWBASE = 111 ) 	
       integer*4   SW0
       parameter ( SW0 	= 111 ) 	
       integer*4   SW1
       parameter ( SW1 	= 112 ) 	
       integer*4   SW2
       parameter ( SW2 	= 113 ) 	
       integer*4   SW3
       parameter ( SW3 	= 114 ) 	
       integer*4   SW4
       parameter ( SW4 	= 115 ) 	
       integer*4   SW5
       parameter ( SW5 	= 116 ) 	
       integer*4   SW6
       parameter ( SW6 	= 117 ) 	
       integer*4   SW7
       parameter ( SW7 	= 118 ) 	
       integer*4   SW8
       parameter ( SW8 	= 119 ) 	
       integer*4   SW9
       parameter ( SW9 	= 120 ) 	
       integer*4   SW10
       parameter ( SW10 = 121 ) 	
       integer*4   SW11
       parameter ( SW11 = 122 ) 	
       integer*4   SW12
       parameter ( SW12 = 123 ) 	
       integer*4   SW13
       parameter ( SW13 = 124 ) 	
       integer*4   SW14
       parameter ( SW14 = 125 ) 	
       integer*4   SW15
       parameter ( SW15 = 126 ) 	
       integer*4   SW16
       parameter ( SW16 = 127 ) 	
       integer*4   SW17
       parameter ( SW17 = 128 ) 	
       integer*4   SW18
       parameter ( SW18 = 129 ) 	
       integer*4   SW19
       parameter ( SW19 = 130 ) 	
       integer*4   SW20
       parameter ( SW20 = 131 ) 	
       integer*4   SW21
       parameter ( SW21 = 132 ) 	
       integer*4   SW22
       parameter ( SW22 = 133 ) 	
       integer*4   SW23
       parameter ( SW23 = 134 ) 	
       integer*4   SW24
       parameter ( SW24 = 135 ) 	
       integer*4   SW25
       parameter ( SW25 = 136 ) 	
       integer*4   SW26
       parameter ( SW26 = 137 ) 	
       integer*4   SW27
       parameter ( SW27 = 138 ) 	
       integer*4   SW28
       parameter ( SW28 = 139 ) 	
       integer*4   SW29
       parameter ( SW29 = 140 ) 	
       integer*4   SW30
       parameter ( SW30 = 141 ) 	
       integer*4   SW31
       parameter ( SW31 = 142 ) 	

C  space ball buttons 

       integer*4   SBBASE
       parameter ( SBBASE = 182 ) 	
       integer*4   SBPICK
       parameter ( SBPICK = 182 ) 	
       integer*4   SBBUT1
       parameter ( SBBUT1 = 183 ) 	
       integer*4   SBBUT2
       parameter ( SBBUT2 = 184 ) 	
       integer*4   SBBUT3
       parameter ( SBBUT3 = 185 ) 	
       integer*4   SBBUT4
       parameter ( SBBUT4 = 186 ) 	
       integer*4   SBBUT5
       parameter ( SBBUT5 = 187 ) 	
       integer*4   SBBUT6
       parameter ( SBBUT6 = 188 ) 	
       integer*4   SBBUT7
       parameter ( SBBUT7 = 189 ) 	
       integer*4   SBBUT8
       parameter ( SBBUT8 = 190 ) 	

C  standard keyboard 

       integer*4   AKEY
       parameter ( AKEY = 11 ) 	
       integer*4   BKEY
       parameter ( BKEY = 36 ) 	
       integer*4   CKEY
       parameter ( CKEY = 28 ) 	
       integer*4   DKEY
       parameter ( DKEY = 18 ) 	
       integer*4   EKEY
       parameter ( EKEY = 17 ) 	
       integer*4   FKEY
       parameter ( FKEY = 19 ) 	
       integer*4   GKEY
       parameter ( GKEY = 26 ) 	
       integer*4   HKEY
       parameter ( HKEY = 27 ) 	
       integer*4   IKEY
       parameter ( IKEY = 40 ) 	
       integer*4   JKEY
       parameter ( JKEY = 34 ) 	
       integer*4   KKEY
       parameter ( KKEY = 35 ) 	
       integer*4   LKEY
       parameter ( LKEY = 42 ) 	
       integer*4   MKEY
       parameter ( MKEY = 44 ) 	
       integer*4   NKEY
       parameter ( NKEY = 37 ) 	
       integer*4   OKEY
       parameter ( OKEY = 41 ) 	
       integer*4   PKEY
       parameter ( PKEY = 48 ) 	
       integer*4   QKEY
       parameter ( QKEY = 10 ) 	
       integer*4   RKEY
       parameter ( RKEY = 24 ) 	
       integer*4   SKEY
       parameter ( SKEY = 12 ) 	
       integer*4   TKEY
       parameter ( TKEY = 25 ) 	
       integer*4   UKEY
       parameter ( UKEY = 33 ) 	
       integer*4   VKEY
       parameter ( VKEY = 29 ) 	
       integer*4   WKEY
       parameter ( WKEY = 16 ) 	
       integer*4   XKEY
       parameter ( XKEY = 21 ) 	
       integer*4   YKEY
       parameter ( YKEY = 32 ) 	
       integer*4   ZKEY
       parameter ( ZKEY = 20 ) 	
       integer*4   ZEROKE
       parameter ( ZEROKE = 46 ) 	
       integer*4   ONEKEY
       parameter ( ONEKEY = 8 ) 	
       integer*4   TWOKEY
       parameter ( TWOKEY = 14 ) 	
       integer*4   THREEK
       parameter ( THREEK = 15 ) 	
       integer*4   FOURKE
       parameter ( FOURKE = 22 ) 	
       integer*4   FIVEKE
       parameter ( FIVEKE = 23 ) 	
       integer*4   SIXKEY
       parameter ( SIXKEY = 30 ) 	
       integer*4   SEVENK
       parameter ( SEVENK = 31 ) 	
       integer*4   EIGHTK
       parameter ( EIGHTK = 38 ) 	
       integer*4   NINEKE
       parameter ( NINEKE = 39 ) 	
       integer*4   BREAKK
       parameter ( BREAKK = 1 ) 	
       integer*4   SETUPK
       parameter ( SETUPK = 2 ) 	
       integer*4   CTRLKE
       parameter ( CTRLKE = 3 ) 	
       integer*4   LEFTCT
       parameter ( LEFTCT = CTRLKE ) 	
       integer*4   CAPSLO
       parameter ( CAPSLO = 4 ) 	
       integer*4   RIGHTS
       parameter ( RIGHTS = 5 ) 	
       integer*4   LEFTSH
       parameter ( LEFTSH = 6 ) 	
       integer*4   NOSCRL
       parameter ( NOSCRL = 13 ) 	
       integer*4   ESCKEY
       parameter ( ESCKEY = 7 ) 	
       integer*4   TABKEY
       parameter ( TABKEY = 9 ) 	
       integer*4   RETKEY
       parameter ( RETKEY = 51 ) 	
       integer*4   SPACEK
       parameter ( SPACEK = 83 ) 	
       integer*4   LINEFE
       parameter ( LINEFE = 60 ) 	
       integer*4   BACKSP
       parameter ( BACKSP = 61 ) 	
       integer*4   DELKEY
       parameter ( DELKEY = 62 ) 	
       integer*4   SEMICO
       parameter ( SEMICO = 43 ) 	
       integer*4   PERIOD
       parameter ( PERIOD = 52 ) 	
       integer*4   COMMAK
       parameter ( COMMAK = 45 ) 	
       integer*4   QUOTEK
       parameter ( QUOTEK = 50 ) 	
       integer*4   ACCENT
       parameter ( ACCENT = 55 ) 	
       integer*4   MINUSK
       parameter ( MINUSK = 47 ) 	
       integer*4   VIRGUL
       parameter ( VIRGUL = 53 ) 	
       integer*4   BACKSL
       parameter ( BACKSL = 57 ) 	
       integer*4   EQUALK
       parameter ( EQUALK = 54 ) 	
       integer*4   LEFTBR
       parameter ( LEFTBR = 49 ) 	
       integer*4   RIGHTB
       parameter ( RIGHTB = 56 ) 	
       integer*4   LEFTAR
       parameter ( LEFTAR = 73 ) 	
       integer*4   DOWNAR
       parameter ( DOWNAR = 74 ) 	
       integer*4   RIGHTA
       parameter ( RIGHTA = 80 ) 	
       integer*4   UPARRO
       parameter ( UPARRO = 81 ) 	
       integer*4   PAD0
       parameter ( PAD0 = 59 ) 	
       integer*4   PAD1
       parameter ( PAD1 = 58 ) 	
       integer*4   PAD2
       parameter ( PAD2 = 64 ) 	
       integer*4   PAD3
       parameter ( PAD3 = 65 ) 	
       integer*4   PAD4
       parameter ( PAD4 = 63 ) 	
       integer*4   PAD5
       parameter ( PAD5 = 69 ) 	
       integer*4   PAD6
       parameter ( PAD6 = 70 ) 	
       integer*4   PAD7
       parameter ( PAD7 = 67 ) 	
       integer*4   PAD8
       parameter ( PAD8 = 68 ) 	
       integer*4   PAD9
       parameter ( PAD9 = 75 ) 	
       integer*4   PADPF1
       parameter ( PADPF1 = 72 ) 	
       integer*4   PADPF2
       parameter ( PADPF2 = 71 ) 	
       integer*4   PADPF3
       parameter ( PADPF3 = 79 ) 	
       integer*4   PADPF4
       parameter ( PADPF4 = 78 ) 	
       integer*4   PADPER
       parameter ( PADPER = 66 ) 	
       integer*4   PADMIN
       parameter ( PADMIN = 76 ) 	
       integer*4   PADCOM
       parameter ( PADCOM = 77 ) 	
       integer*4   PADENT
       parameter ( PADENT = 82 ) 	

C  the extended keyboard 

       integer*4   LEFTAL
       parameter ( LEFTAL = 143 ) 	
       integer*4   RGHTAL
       parameter ( RGHTAL = 144 ) 	
       integer*4   RIGHTC
       parameter ( RIGHTC = 145 ) 	
       integer*4   F1KEY
       parameter ( F1KEY = 146 ) 	
       integer*4   F2KEY
       parameter ( F2KEY = 147 ) 	
       integer*4   F3KEY
       parameter ( F3KEY = 148 ) 	
       integer*4   F4KEY
       parameter ( F4KEY = 149 ) 	
       integer*4   F5KEY
       parameter ( F5KEY = 150 ) 	
       integer*4   F6KEY
       parameter ( F6KEY = 151 ) 	
       integer*4   F7KEY
       parameter ( F7KEY = 152 ) 	
       integer*4   F8KEY
       parameter ( F8KEY = 153 ) 	
       integer*4   F9KEY
       parameter ( F9KEY = 154 ) 	
       integer*4   F10KEY
       parameter ( F10KEY = 155 ) 	
       integer*4   F11KEY
       parameter ( F11KEY = 156 ) 	
       integer*4   F12KEY
       parameter ( F12KEY = 157 ) 	
       integer*4   PRINTS
       parameter ( PRINTS = 158 ) 	
       integer*4   SCROLL
       parameter ( SCROLL = 159 ) 	
       integer*4   PAUSEK
       parameter ( PAUSEK = 160 ) 	
       integer*4   INSERT
       parameter ( INSERT = 161 ) 	
       integer*4   HOMEKE
       parameter ( HOMEKE = 162 ) 	
       integer*4   PAGEUP
       parameter ( PAGEUP = 163 ) 	
       integer*4   ENDKEY
       parameter ( ENDKEY = 164 ) 	
       integer*4   PAGEDO
       parameter ( PAGEDO = 165 ) 	
       integer*4   NUMLOC
       parameter ( NUMLOC = 166 ) 	
       integer*4   PADVIR
       parameter ( PADVIR = 167 ) 	
       integer*4   PADAST
       parameter ( PADAST = 168 ) 	
       integer*4   PADPLU
       parameter ( PADPLU = 169 ) 	

C  
C  By rights, we should define symbolic entries here for all of the new
C  characters brought to us by ISO 8859-1.  In fact, since there is no
C  current convention to avoid making new symbols that are unique, the
C  danger of collison with existing user symbols is too high. 
C 


C  valuators 

       integer*4   SGIRES
       parameter ( SGIRES = 256 ) 	
       integer*4   DIAL0
       parameter ( DIAL0 = 257 ) 	
       integer*4   DIAL1
       parameter ( DIAL1 = 258 ) 	
       integer*4   DIAL2
       parameter ( DIAL2 = 259 ) 	
       integer*4   DIAL3
       parameter ( DIAL3 = 260 ) 	
       integer*4   DIAL4
       parameter ( DIAL4 = 261 ) 	
       integer*4   DIAL5
       parameter ( DIAL5 = 262 ) 	
       integer*4   DIAL6
       parameter ( DIAL6 = 263 ) 	
       integer*4   DIAL7
       parameter ( DIAL7 = 264 ) 	
       integer*4   DIAL8
       parameter ( DIAL8 = 265 ) 	
       integer*4   MOUSEX
       parameter ( MOUSEX = 266 ) 	
       integer*4   MOUSEY
       parameter ( MOUSEY = 267 ) 	
       integer*4   LPENX
       parameter ( LPENX = 268 ) 	
       integer*4   LPENY
       parameter ( LPENY = 269 ) 	
       integer*4   BPADX
       parameter ( BPADX = 270 ) 	
       integer*4   BPADY
       parameter ( BPADY = 271 ) 	
       integer*4   CURSRX
       parameter ( CURSRX = 272 ) 	
       integer*4   CURSRY
       parameter ( CURSRY = 273 ) 	
       integer*4   GHOSTX
       parameter ( GHOSTX = 274 ) 	
       integer*4   GHOSTY
       parameter ( GHOSTY = 275 ) 	

C  Space Ball valuators 

       integer*4   SBTX
       parameter ( SBTX = 276 ) 	
       integer*4   SBTY
       parameter ( SBTY = 277 ) 	
       integer*4   SBTZ
       parameter ( SBTZ = 278 ) 	
       integer*4   SBRX
       parameter ( SBRX = 279 ) 	
       integer*4   SBRY
       parameter ( SBRY = 280 ) 	
       integer*4   SBRZ
       parameter ( SBRZ = 281 ) 	
       integer*4   SBPERI
       parameter ( SBPERI = 282 ) 	

C  timers 

       integer*4   TIMER0
       parameter ( TIMER0 = 515 ) 	
       integer*4   TIMER1
       parameter ( TIMER1 = 516 ) 	
       integer*4   TIMER2
       parameter ( TIMER2 = 517 ) 	
       integer*4   TIMER3
       parameter ( TIMER3 = 518 ) 	

C  misc devices 

       integer*4   KEYBD
       parameter ( KEYBD = 513 ) 	
       integer*4   RAWKBD
       parameter ( RAWKBD = 514 ) 	
       integer*4   VALMAR
       parameter ( VALMAR = 523 ) 	
       integer*4   REDRAW
       parameter ( REDRAW = 528 ) 	
       integer*4   INPTCH
       parameter ( INPTCH = 534 ) 	
       integer*4   QFULL
       parameter ( QFULL = 535 ) 	
       integer*4   QREADE
       parameter ( QREADE = 538 ) 	
       integer*4   WINFRE
       parameter ( WINFRE = 539 ) 	
       integer*4   WINTHA
       parameter ( WINTHA = 540 ) 	
       integer*4   REDRWI
       parameter ( REDRWI = 541 ) 	
       integer*4   WINQUI
       parameter ( WINQUI = 542 ) 	
       integer*4   DPTHCH
       parameter ( DPTHCH = 543 ) 	
       integer*4   WINSHU
       parameter ( WINSHU = 546 ) 	
       integer*4   DRAWOV
       parameter ( DRAWOV = 547 ) 	
       integer*4   VIDEO
       parameter ( VIDEO = 548 ) 	

C       integer*4   MENUBU
C       parameter ( MENUBU = RIGHTM ) 	


C  
C  obsolete symbols 
C 


       integer*4   WINCLS
       parameter ( WINCLS = 537 ) 	
       integer*4   KBDFNA
       parameter ( KBDFNA = 544 ) 	
       integer*4   KBDFST
       parameter ( KBDFST = 545 ) 	
       integer*4   MAXSGI
       parameter ( MAXSGI = 20000 ) 	

C  these events only occur when using the mex window manager (3K Series) 
       integer*4   GERROR
       parameter ( GERROR = 524 ) 	
       integer*4   WMSEND
       parameter ( WMSEND = 529 ) 	
       integer*4   WMREPL
       parameter ( WMREPL = 530 ) 	
       integer*4   WMGFCL
       parameter ( WMGFCL = 531 ) 	
       integer*4   WMTXCL
       parameter ( WMTXCL = 532 ) 	
       integer*4   MODECH
       parameter ( MODECH = 533 ) 	
       integer*4   PIECHN
       parameter ( PIECHN = 536 ) 	


