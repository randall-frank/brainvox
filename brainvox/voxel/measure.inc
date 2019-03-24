
	Integer*4	max_linedata
	Parameter	(max_linedata=14)

	Structure /measuredata/
		Real*4		data(max_linedata)
		Integer*4	Color
	End Structure

	Record /measuredata/ linedata

	Integer*4	pointtype !1=BV 2=tal 3=disk
	Integer*4	data_layer

	Common /measurementdata/linedata,pointtype,data_layer

