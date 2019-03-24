C
C	Defines for the various filer return destinations
C
C	IPC message format:
C	(IPC_F_RETURN)(Filenametype code)(zero terminated filename)
C
C	Palette editor, load and save palettes and opacity tables
C	
C	Filename type codes:
C
C	Special types  
C		Warning dialog (no return IPC)
C	dir=<btntext>
C
	Integer*4       FLR_WARNING
	Parameter       (FLR_WARNING = 0)
C
C		two button request
C	Base typecode is FLR_REQUEST
C	dir=<btn1>[|<btn2>][|<btn3>]
C	IPC filename is the button text
C
	Integer*4       FLR_REQUEST
	Parameter       (FLR_REQUEST = 10000)
	Integer*4	FLR_REQ_TEX_BASE 
	Parameter       (FLR_REQ_TEX_BASE = FLR_REQUEST)
C
C	Image LUT => paled
C
	Integer*4	FLR_PAL1_S_OPAC
	Integer*4	FLR_PAL1_L_OPAC
	Integer*4	FLR_PAL1_S_PAL
	Integer*4	FLR_PAL1_L_PAL
	Parameter	(FLR_PAL1_S_OPAC= 1)
	Parameter	(FLR_PAL1_L_OPAC= 2)
	Parameter	(FLR_PAL1_S_PAL = 3)
	Parameter	(FLR_PAL1_L_PAL = 4)
C
C	Volume LUT => paled
C
	Integer*4	FLR_PAL2_S_OPAC
	Integer*4	FLR_PAL2_L_OPAC
	Integer*4	FLR_PAL2_S_PAL
	Integer*4	FLR_PAL2_L_PAL
	Parameter	(FLR_PAL2_S_OPAC= 5)
	Parameter	(FLR_PAL2_L_OPAC= 6)
	Parameter	(FLR_PAL2_S_PAL = 7)
	Parameter	(FLR_PAL2_L_PAL = 8)
C
C	Texture LUT => paled
C
	Integer*4	FLR_PAL3_S_OPAC
	Integer*4	FLR_PAL3_L_OPAC
	Integer*4	FLR_PAL3_S_PAL
	Integer*4	FLR_PAL3_L_PAL
	Parameter	(FLR_PAL3_S_OPAC= 9)
	Parameter	(FLR_PAL3_L_OPAC= 10)
	Parameter	(FLR_PAL3_S_PAL = 11)
	Parameter	(FLR_PAL3_L_PAL = 12)
C
C	Histogram save => paled
C
	Integer*4	FLR_PAL_HISTO
	Parameter	(FLR_PAL_HISTO = 17)
C
C	Get an image for the texture volume
C
	Integer*4	FLR_TEX_VOLUME
	Parameter       (FLR_TEX_VOLUME = 13)
C
C	Textfile I/O => MAP3
C
	Integer*4	FLR_MAP3_LOAD_SET
	Integer*4	FLR_MAP3_SAVE_SET
	Integer*4	FLR_MAP3_LOAD_VALS
	Parameter       (FLR_MAP3_LOAD_SET = 14)
	Parameter       (FLR_MAP3_SAVE_SET = 15)
	Parameter       (FLR_MAP3_LOAD_VALS= 16)
C
C	Image to save tracing images
C
	Integer*4	FLR_TRACE_IMAGES
	Parameter	(FLR_TRACE_IMAGES=18)
C
C	Voxel to open a file of commands
C
	Integer*4	FLR_CMD_FILE
	Parameter	(FLR_CMD_FILE=19)
C
C	Thats all the defined filer services.
C
