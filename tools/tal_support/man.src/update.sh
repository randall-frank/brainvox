#!/bin/csh
set mans=/usr/local/man/man1
foreach i ($argv)
	echo "Working on:" $i
	nroff -man $i > $i:r
	cp $i:r $i:r.man
	pack -f $i:r
	cp $i:r.z $mans
	chmod +r $mans/$i:r.z
	rm -f $i:r.z
end
