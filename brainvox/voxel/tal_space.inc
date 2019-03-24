C
C	Structure for the Talairach Space conversion
C
	Structure	/tal_conv/
		Integer*4	valid
		Real*4		points(3,3)  !CA,CP,PLANE
		Real*4		CA_front,CP_back,CA_CP  !distances
		Real*4		CA_top,CA_bottom
		Real*4		CA_right,CA_left
		Real*4		xaxis(3)  ! vectors
		Real*4		yaxis(3)
		Real*4		zaxis(3)
	End Structure
C
	Record	/tal_conv/	tspace
C
	Common /talairach/tspace
C
C       Defines for the size of the atlas brain (ISBN 3-13-711701-1)
C       Talairach/Tournoux: Co-Planar Stereotaxic Atlas of the Human Brain
C       Georg Thieme Verlag Stuttgart, New York 1988
C
	Real*4	TAL_FRONT,TAL_MIDDLE,TAL_BACK,TAL_RIGHT,TAL_LEFT
	Real*4	TAL_BOTTOM,TAL_TOP
	Parameter (TAL_FRONT = 70.0)
	Parameter (TAL_MIDDLE = 23.5)
	Parameter (TAL_BACK = 80.0)
C       0 to Back distance is really 103.5  (80+23.5=103.5)
	Parameter (TAL_RIGHT = 68.5)
	Parameter (TAL_LEFT = 68.5)
	Parameter (TAL_TOP = 74.5)
	Parameter (TAL_BOTTOM = 44.0)
C
C       useful defines
C
	Real*4	TOLERANCE
	Parameter (TOLERANCE = 5.0)
	Integer*4 PT_CA,PT_CP,PT_PLANE
	Parameter (PT_CA = 1)
	Parameter (PT_CP = 2)
	Parameter (PT_PLANE = 3)
C
