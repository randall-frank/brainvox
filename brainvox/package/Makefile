#    ======================================================================
#       Copyright (C) 1989 Image Analysis Facility, University of Iowa
#    ======================================================================
#
#	makefile for binary packages
#
include ../config

ifeq ($(OS),irix)
all:
	rm -f ../bvox_bin.tgz
	cd ../bin; tar cvf ../bvox_bin.tar *
	cd ..; gzip bvox_bin.tar
	cd ..; mv bvox_bin.tar.gz bvox_bin.tgz
else
ifeq ($(OS),linux)
all:
	rm -f ../bvox_bin.tgz
	cd ../bin; tar cvf ../bvox_bin.tar *
	cd ..; gzip bvox_bin.tar
	cd ..; mv bvox_bin.tar.gz bvox_bin.tgz
else
ifeq ($(OS),WIN32)
all:
	makensis brainvox_mui.nsi
else
ifeq ($(OS),osx)
PMD=/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/
PM=$(PMD)/PackageMaker
all:
	-mkdir -p ./Brainvox_dist/Package_Root/Applications/Brainvox.app/bin
	-mkdir -p ./Brainvox_dist/Package_Root/Applications/Brainvox.app/relnotes
	-install -m 755 ../bin/* \
		Brainvox_dist/Package_Root/Applications/Brainvox.app/bin
	-install -m 755 ../docs/ReleaseNotes.txt \
		Brainvox_dist/Package_Root/Applications/Brainvox.app/relnotes
	$(PM) -build -p ./brainvox.pkg -f ./Brainvox_dist/Package_Root \
		-i Info.plist -d Description.plist -ds -v
	-install -m 644 ReadMe.rtf  ./brainvox.pkg/Contents/Resources
	-install -m 644 License.rtf ./brainvox.pkg/Contents/Resources
	-install -m 644 Welcome.rtf ./brainvox.pkg/Contents/Resources
	hdiutil create -srcfolder brainvox.pkg brainvox -volname Brainvox -ov
	gzip brainvox.dmg
	mv brainvox.dmg.gz ..
#	open /Developer/Applications/Utilities/PackageMaker.app brainvox.pmproj
else
all:
	echo "Unknown platform"
endif
endif
endif
endif

clean:
	-rm -rf Brainvox_dist
	-rm -rf brainvox.pkg

image:
	hdiutil create -srcfolder brainvox.pkg brainvox -volname Brainvox -ov
	mv brainvox.dmg ..
	make clean
