C
C	palette editor globals
C
	Integer*4	pins(2),scale_pins
	Integer*4	color_model  ! 1=rgb 2=hsv
	Integer*4	tool ! 1=freehand 2=fiddle 3=rotate
	Real*4		ed_pal(256,4)

	common/pals_gbl/pins,color_model,tool,ed_pal,scale_pins

        Integer*4       cbuffer(256,4),ccount
        
        common/pal_buffer/cbuffer,ccount

