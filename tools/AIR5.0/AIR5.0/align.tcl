#!/usr/local/bin/wish4.0
# Copyright 1995-2001 Roger P. Woods, M.D.
# Modified 12/19/01

#output file box color 
set color1 #efffaafff
#modality box color
set color2 #efffaafff
#standard file box color
set color3 #efffaafff
#cost function box color
set color4 #efffaafff
#reslice file box color
set color5 #efffaafff
#sampling box color
set color6 #efffaafff
#convergence box color
set color7 #efffaafff
#iterations box color
set color8 #efffaafff
#model box color
set color9 #efffaafff
#flags box color
set color10 #efffaafff

#Some sample colors to choose from
#green #cfc
#cyan #cff
#red #fbb
#green #dfd
#dark blue #ccf
#yellow #ffc
#light red #fdd
#light blue #eef
#purple #fdf
#light yellow #ffe


proc GetFileName {itemname filter mode} {
	global result

	#invoke the menubox
	toplevel $itemname 

	if {[string compare $itemname ".select_reslice_file"] == 0} {
		wm title $itemname "Reslice File(s)"
	} elseif {[string compare $itemname ".select_standard_file"] == 0} {
		wm title $itemname "Standard File"
	}

	#create a place to put the directory name
	label $itemname.lbl -text Directory:
	entry $itemname.entry
	$itemname.entry config -textvariable result
	pack $itemname.lbl $itemname.entry -side top -fill x
	focus $itemname.entry
	set result [pwd]

	#create a scrolling listbox to put the file in
	listbox $itemname.list -yscrollcommand [list $itemname.sy set]
	$itemname.list configure -selectmode $mode
	scrollbar $itemname.sy -orient vertical -command [list $itemname.list yview]

	#create a Done button if extended mode is supported
	if {[string compare $mode "extended"] == 0} {
		button $itemname.button -text "Done selecting" -command "ReturnFile $itemname $filter"
		pack $itemname.button -side right -expand true
	}
	pack $itemname.sy -side right -fill y
	pack $itemname.list -side left -fill both -expand true

	#Have the menu updated whenever a return is entered into the directory field
	bind $itemname.entry <Return> "MenuUpdate $itemname $filter"

	bind $itemname.list <Double-1> "ReturnFile $itemname $filter"

	MenuUpdate $itemname $filter

}

proc ComFile { } {

	global comfile
	global comdir
	global color1

	#invoke a menubox
	toplevel .com
	wm title .com "Command File"

	#create a place to put the directory name
	label .com.lbl -text Directory: -background $color1
	entry .com.entry
	.com.entry config -textvariable comdir
	
	#create a place to put the file name
	label .com.filelbl -text File: -background $color1
	entry .com.fileentry
	.com.fileentry config -textvariable comfile

	pack .com.lbl .com.entry -side top
	pack .com.entry -side top -fill x

	focus .com.fileentry
	set comdir [pwd]
	set comfile "command.com"

	#create a scrolling listbox of directories
	listbox .com.list -yscrollcommand [list .com.sy set]
	.com.list configure -selectmode browse
	scrollbar .com.sy -orient vertical -command [list .com.list yview]

	button .com.done -text "Done" -command "ComFileSelected"
	pack .com.done -side bottom -fill x

	pack .com.fileentry .com.filelbl -side right -fill x -anchor n

	pack .com.sy -side right -fill y
	pack .com.list -side left -fill both -expand true


	#Have the menu updated whenever a return is entered into the directory field
	bind .com.entry <Return> "DirMenuUpdate .com"
	bind .com.list <Double-1> "DirReturnFile .com"

	#Error checking and completion when a return is entered into the file field
	bind .com.fileentry <Return> "ComFileSelected"

	DirMenuUpdate .com
}

proc ComFileSelected { } {
	global comfile
	global comdir
	global outfile

	set outfile $comdir
	append outfile "/"
	append outfile $comfile
	if [file exists $outfile] {
		toplevel .delete_file
		wm title .delete_file "WARNING:"
		message .delete_file.msg -text "File $outfile already exists" -aspect 1000
		button .delete_file.delete -text "Delete It" -command {
			if [catch {exec /bin/rm $outfile} err] {
				toplevel .goofup
				message .goofup.msg -text "/bin/rm $outfile"
				button .goofup.ok -text "OK" -command {
					destroy .goofup}
				pack .goofup.msg .goofup.ok -side top
				tkwait visibility .goofup
				grab set .goofup}
			destroy .delete_file
			WriteComFile}
		button .delete_file.nodelete -text "Oops!" -command {
			destroy .delete_file}
		pack .delete_file.msg -side top
		pack .delete_file.delete .delete_file.nodelete -side left -fill both -expand true
		focus .delete_file.nodelete
		grab set .delete_file 
		bell
	} else { WriteComFile }
}

proc WriteComFile { } {

	global outfile
	global program
	global stdfile
	global airsuffix
	global threshold1
	global threshold2
	global xsmooth1
	global ysmooth1
	global zsmooth1
	global xsmooth2
	global ysmooth2
	global zsmooth2
	global costfxn
	global model
	global initial
	global final
	global decrement
	global iterations
	global unimproved
	global converge
	global flags
	global modality
	global partitions1
	global partitions2
	global intermodality
	global overwrite

	exec echo "" >$outfile
	set reslicefiles [.image.r.file.list get 0 end]
	foreach i $reslicefiles {
		set comlist "exec echo $program $stdfile $i $i$airsuffix \
			-m $model \
			-t1 $threshold1 \
			-t2 $threshold2 \
			-b1 $xsmooth1 $ysmooth1 $zsmooth1 \
			-b2 $xsmooth2 $ysmooth2 $zsmooth2 \
			-p1 $partitions1 \
			-p2 $partitions2 \
			-x $costfxn \
			-s $initial $final $decrement \
			-r $iterations \
			-h $unimproved \
			-c $converge \
			$flags \
			>>$outfile"
		join $comlist " "

		if [catch {eval $comlist} err] {
		toplevel .goofup
		message .goofup.msg -text $comlist
		button .goofup.ok -text "OK" -command {
			destroy .goofup}
		pack .goofup.msg .goofup.ok -side top
		tkwait visibility .goofup
		grab set .goofup

		}
	}

	destroy .com
}


proc Run { } {
	global program
	global stdfile
	global airsuffix
	global threshold1
	global threshold2
	global xsmooth1
	global ysmooth1
	global zsmooth1
	global xsmooth2
	global ysmooth2
	global zsmooth2
	global costfxn
	global model
	global initial
	global final
	global decrement
	global iterations
	global unimproved
	global converge
	global flags
	global modality
	global partitions1
	global partitions2
	global intermodality
	global overwrite
	set reslicefiles [.image.r.file.list get 0 end]
	foreach i $reslicefiles {
		set comlist "exec xterm -e $program $stdfile $i $i$airsuffix \
			-m $model \
			-t1 $threshold1 \
			-t2 $threshold2 \
			-b1 $xsmooth1 $ysmooth1 $zsmooth1 \
			-b2 $xsmooth2 $ysmooth2 $zsmooth2 \
			-p1 $partitions1 \
			-p2 $partitions2 \
			-x $costfxn \
			-s $initial $final $decrement \
			-r $iterations \
			-h $unimproved \
			-c $converge \
			$flags"
		join $comlist " "
		if [catch {eval $comlist} err] {
			if [winfo exists .goofup] {
				raise .goofup
				focus .goofup
			} else {
				toplevel .goofup
				text .goofup.msg
				button .goofup.ok -text "OK" -command {
					destroy .goofup}
				pack .goofup.msg .goofup.ok -side top
				.goofup.msg insert insert "The following commands resulted in error messages from the operating system:\n\n"

			}
			.goofup.msg insert insert $comlist
			.goofup.msg insert insert "\n\n"
			tkwait visibility .goofup
			grab set .goofup

		}
	}




}
proc Intramodality { } {
	global partitions1
	global partitions2
	global direction
	global intermodality
	global color2

	toplevel .direction -bg $color2
	wm title .direction "Direction Selection"

	#Reactivate standard deviation cost function
	.output.config.costfxn.choices.leastsq configure -state normal
	.output.config.costfxn.choices.leastsqrescale configure -state normal

	frame .direction.buttons -bg $color2
	radiobutton .direction.buttons.bid -variable direction -text "Bidirectional fit" -value 0 -bg $color2 -highlightthickness 0 -command {
		set partitions1 1
		set partitions2 1
		destroy .direction
	}
	radiobutton .direction.buttons.fwd -variable direction -text "Forward fit" -value 1 -bg $color2 -highlightthickness 0 -command {
		set partitions1 1
		set partitions2 0
		set intermodality 1
		destroy .direction
	}
	radiobutton .direction.buttons.rev -variable direction -text "Reverse fit" -value -1 -bg $color2 -highlightthickness 0 -command {
		set partitions1 0
		set partitions2 1
		set intermodality 2
		destroy .direction
	}
	message .direction.msg -text "Bidirectional fits are generally recommended.\n\n\
		However, if you have edited the standard file to remove areas of pathology that you think\
		will interfere with registration, choose \"Forward fit\".\n\n Likewise, if you have edited\
		the reslice files to remove pathology, choose \"Reverse fit\"." -aspect 500 -bg $color2

	pack .direction.buttons.bid .direction.buttons.fwd .direction.buttons.rev -side top -anchor w -fill y -expand true

	if {$direction == 0} {
		set partitions1 1
		set partitions2 1
	} elseif {$direction == 1} {
		set partitions1 1
		set partitions2 0
		set intermodality 1
	} else {
		set partitions1 0
		set partitions2 1
		set intermodality 2
	}

	pack .direction.msg .direction.buttons -side top -fill y -expand true
	tkwait visibility .direction

	if {$direction == 0} {
		focus .direction.buttons.bid
	} elseif {$direction == 1} {
		focus .direction.buttons.fwd
	} else {
		focus .direction.buttons.rev
	}

	grab set .direction


}	



proc Intermodality { } {
	global partitions1
	global partitions2
	global costfxn
	global intermodality
	global color2
	global color4
	toplevel .modality -bg $color2
	wm title .modality "Segmentation Selection"

	#Change to standard deviation cost function and make sure it stays that way until this button is off
	if {$costfxn != 1} {
		set costfxn 1
			bell
			message .modality.warning -text "Your cost function has been reset to \"Standard\
				Deviation of Ratio Image\" because intermodality registration\
				using other cost functions is not available." -aspect 500 -bg $color4 -borderwidth 10 -relief groove
			pack .modality.warning -side top -pady 10
	}
	.output.config.costfxn.choices.leastsq configure -state disabled
	.output.config.costfxn.choices.leastsqrescale configure -state disabled

	frame .modality.partitions -bg $color2
	frame .modality.partitions.standard -bg $color2
	label .modality.partitions.standard.label -bg $color2 -text "Standard file partitions:"
	entry .modality.partitions.standard.entry -textvariable partitions1 -width 6
	pack .modality.partitions.standard.label .modality.partitions.standard.entry -side left

	frame .modality.partitions.reslice -bg $color2
	label .modality.partitions.reslice.label -bg $color2 -text "Reslice file partitions:"
	entry .modality.partitions.reslice.entry -textvariable partitions2 -width 6
	pack .modality.partitions.reslice.label .modality.partitions.reslice.entry -side left

	pack .modality.partitions.standard .modality.partitions.reslice -side top -fill both -expand true

	frame .modality.buttons -bg $color2
	frame .modality.buttons.all -bg $color2
	radiobutton .modality.buttons.all.standard -variable intermodality -text "Segment standard file" -value 1 -bg $color2 -highlightthickness 0 -command {
		.modality.partitions.standard.entry config -state normal
		if {$partitions1 == 1} {set partitions1 256}
		if {$partitions1 == 0} {set partitions1 256}
		.modality.partitions.reslice.entry config -state disabled
		set partitions2 0
		focus .modality.partitions.standard.entry
	}
	radiobutton .modality.buttons.all.reslice -variable intermodality -text "Segment reslice file" -value 2 -bg $color2 -highlightthickness 0 -command {
		.modality.partitions.standard.entry config -state disabled
		set partitions1 0
		.modality.partitions.reslice.entry config -state normal
		if {$partitions2 == 1} {set partitions2 256}
		if {$partitions2 == 0} {set partitions2 256}
		focus .modality.partitions.reslice.entry
	}
	radiobutton .modality.buttons.all.both -variable intermodality -text "Segment both files" -value 3 -bg $color2 -highlightthickness 0 -command {
		.modality.partitions.standard.entry config -state normal
		if {$partitions1 == 1} {set partitions1 256}
		if {$partitions1 == 0} {set partitions1 256}
		.modality.partitions.reslice.entry config -state normal
		if {$partitions2 == 1} {set partitions2 256}
		if {$partitions2 == 0} {set partitions2 256}
		focus .modality.partitions.standard.entry
	}

	pack .modality.buttons.all.standard .modality.buttons.all.reslice .modality.buttons.all.both -side top -anchor w -fill y -expand true

	button .modality.buttons.done -text "Done" -command {
		destroy .modality
	}

	pack .modality.buttons.all .modality.buttons.done -side left -expand true


	message .modality.msg -text "Segmentation for intermodality registration is based\
		solely on image intensity partitioning.\n\n\
		MRI files are best suited for intensity based segmentation but may nonetheless require\
		prior editing to remove confounding structures such as scalp and skull.\n\n\
		Up to 256 partitions are allowed." -aspect 500 -bg $color2
	pack .modality.msg .modality.buttons -side top -anchor w -fill y -expand true -pady 5


	if {$intermodality == 1} {
		.modality.partitions.standard.entry config -state normal
		if {$partitions1 == 1} {set partitions1 256}
		if {$partitions1 == 0} {set partitions1 256}
		.modality.partitions.reslice.entry config -state disabled
		set partitions2 0
		focus .modality.partitions.standard.entry
	} elseif {$intermodality == 2} {
		.modality.partitions.standard.entry config -state disabled
		set partitions1 0
		.modality.partitions.reslice.entry config -state normal
		if {$partitions2 == 1} {set partitions2 256}
		if {$partitions2 == 0} {set partitions2 256}
		focus .modality.partitions.reslice.entry
	} else {
		.modality.partitions.standard.entry config -state normal
		if {$partitions1 == 1} {set partitions1 256}
		if {$partitions1 == 0} {set partitions1 256}
		.modality.partitions.reslice.entry config -state normal
		if {$partitions2 == 1} {set partitions2 256}
		if {$partitions2 == 0} {set partitions2 256}
		focus .modality.partitions.standard.entry
	}

	pack .modality.buttons .modality.partitions -side top -fill both -expand true
	tkwait visibility .modality
	grab set .modality

}


#Handle menu selection for input files
proc ReturnFile {itemname filter} {
	global answer
	global result

	set selection [$itemname.list get [$itemname.list index active]]
	set temp [lindex $selection 0]
	if {[string compare [string index $temp [expr [string length $temp] -1]] "/"] == 0} {
		#This is a directory
		if [catch {cd [$itemname.entry get]/$selection} err] {
			ShowError $err $itemname
			return
		} else {
			set result [pwd]
			MenuUpdate $itemname $filter
			return
		}
	} else {
		#This is a file, potentially one of many
		unset answer
		set answer { }
		set temp [pwd]
		append temp "/"
		foreach i [$itemname.list curselection] {
			set temp2 $temp
			append temp2 [$itemname.list get $i]
			#Make sure we haven't included a directory by accident
			if {[string compare [string index $temp2 [expr [string length $temp2] -1]] "/"] != 0} {
				#This is not a directory
				lappend answer $temp2
			}
		}
		destroy $itemname
	}
}

#Handle menu selection for input files
proc DirReturnFile {itemname} {
	global comdir

	set selection [$itemname.list get [$itemname.list index active]]
	set temp [lindex $selection 0]
	if [catch {cd [$itemname.entry get]/$selection} err] {
		ShowError $err $itemname
		return
	} else {
		set comdir [pwd]
		DirMenuUpdate $itemname
		return
	}
}


#update the listbox for selecting input files
proc MenuUpdate {itemname filter} {
	if [catch {cd [$itemname.entry get]} err] {
		ShowError $err $itemname
		return
	} else {
		$itemname.list delete 0 end
		foreach f [lsort [glob -nocomplain *$filter]] {
			regsub $filter $f "" joe2
			$itemname.list insert end $joe2
		}
		$itemname.list insert end "../"
		foreach f [lsort [glob -nocomplain *]] {
			if [file isdirectory $f] {
				append f "/"
				$itemname.list insert end $f
			}
		}
	}
}

#update the listbox for selecting input files
proc DirMenuUpdate {itemname} {
	if [catch {cd [$itemname.entry get]} err] {
		ShowError $err $itemname
		return
	} else {
		$itemname.list delete 0 end
		$itemname.list insert end "../"
		foreach f [lsort [glob -nocomplain *]] {
			if [file isdirectory $f] {
				append f "/"
				$itemname.list insert end $f
			}
		}
	}
}

#Show error messages
proc ShowError {err itemname} {
	toplevel $itemname.error
	focus $itemname.error
	grab set $itemname.error
	message $itemname.error.msg -text $err -justify center -aspect 1000
	button $itemname.error.button -text "If you say so" -command "destroy $itemname.error"
	pack $itemname.error.msg $itemname.error.button -side top
	
}
frame .image
frame .image.s -background $color3 -relief ridge -borderwidth 3

frame .image.s.program -background $color3
label .image.s.program.label -text "Alignment Program:" -background $color3
entry .image.s.program.entry -textvariable program
pack .image.s.program.label -side left
pack .image.s.program.entry -side right -padx 1 -pady 1 -fill x

frame .image.s.title -background $color3
label .image.s.title.label -text "Standard File" -background $color3

frame .image.s.title.file -background $color3
entry .image.s.title.file.entry -textvariable stdfile
button .image.s.title.file.button -text "Select..." -command {
	if [winfo exists .select_standard_file] {
		raise .select_standard_file
		focus .select_standard_file.entry
	} else {
		GetFileName .select_standard_file .hdr browse
		tkwait variable answer
		set stdfile [lindex $answer 0]
		.image.s.title.file.entry xview moveto 1
		focus .image.s.title.file.entry
	}
}
pack .image.s.title.file.button .image.s.title.file.entry -side left -padx 1
pack .image.s.title.label .image.s.title.file -side top


frame .image.s.config -background $color3
entry .image.s.config.threshentry -textvariable threshold1 -width 6
label .image.s.config.threshlabel -text "threshold:" -background $color3
entry .image.s.config.partentry -textvariable partitions1 -width 6
label .image.s.config.partlabel -text "partitions:" -background $color3
pack .image.s.config.threshlabel .image.s.config.threshentry .image.s.config.partlabel .image.s.config.partentry -side left

frame .image.s.smooth -background $color3
entry .image.s.smooth.xentry -textvariable xsmooth1 -width 6
entry .image.s.smooth.yentry -textvariable ysmooth1 -width 6
entry .image.s.smooth.zentry -textvariable zsmooth1 -width 6
label .image.s.smooth.xlabel -text "smoothing: x:" -background $color3
label .image.s.smooth.ylabel -text "y:" -background $color3
label .image.s.smooth.zlabel -text "z:" -background $color3
pack .image.s.smooth.zentry .image.s.smooth.zlabel .image.s.smooth.yentry .image.s.smooth.ylabel .image.s.smooth.xentry .image.s.smooth.xlabel -side right -padx 1

frame .image.r -background $color5 -relief ridge -borderwidth 3
frame .image.r.title -background $color5
label .image.r.title.label -text "File(s) to Align" -background $color5
pack .image.r.title.label

frame .image.r.file -background $color5

#create a scrolling listbox to put the file in
listbox .image.r.file.list -yscrollcommand [list .image.r.file.sy set] -selectmode extended
scrollbar .image.r.file.sy -orient vertical -command [list .image.r.file.list yview]

frame .image.r.file.buttons -background $color5
button .image.r.file.buttons.add -text "Add..." -command {
	if [winfo exists .select_reslice_file] {
		raise .select_reslice_file
		focus .select_reslice_file.entry
	} else {
		GetFileName .select_reslice_file .hdr extended
		tkwait variable answer
		foreach i $answer {
			.image.r.file.list insert end $i
		}
		.image.r.file.list xview moveto 1
		focus .image.r.file.buttons.add
	}
}
button .image.r.file.buttons.delete -text "Remove" -command {
	foreach i [lsort -integer -decreasing [.image.r.file.list curselection]] {
		.image.r.file.list delete $i
	}
}

pack .image.r.file.buttons.add  .image.r.file.buttons.delete -side top -padx 1 -pady 1 -fill x
pack .image.r.file.buttons -side left -padx 1
pack .image.r.file.sy -side right -fill both -expand true
pack .image.r.file.list -side left -fill both -expand true


frame .image.r.config -background $color5
entry .image.r.config.threshentry -textvariable threshold2 -width 6
label .image.r.config.threshlabel -text "threshold:" -background $color5
entry .image.r.config.partentry -textvariable partitions2 -width 6
label .image.r.config.partlabel -text "partitions:" -background $color5
pack .image.r.config.threshlabel .image.r.config.threshentry .image.r.config.partlabel .image.r.config.partentry -side left


frame .image.r.smooth -background $color5
entry .image.r.smooth.xentry -textvariable xsmooth2 -width 6
entry .image.r.smooth.yentry -textvariable ysmooth2 -width 6
entry .image.r.smooth.zentry -textvariable zsmooth2 -width 6
label .image.r.smooth.xlabel -text "smoothing: x:" -background $color5
label .image.r.smooth.ylabel -text "y:" -background $color5
label .image.r.smooth.zlabel -text "z:" -background $color5
pack .image.r.smooth.xlabel .image.r.smooth.xentry .image.r.smooth.ylabel .image.r.smooth.yentry .image.r.smooth.zlabel .image.r.smooth.zentry -side left -padx 1

frame .image.o -background $color1 -relief ridge -borderwidth 3
label .image.o.label -text "Output File(s)" -background $color1
frame .image.o.name -background $color1
entry .image.o.name.entry -textvariable airsuffix -width 10
label .image.o.name.label -text ".air file suffix:" -background $color1
pack .image.o.name.label .image.o.name.entry -side left -padx 1 -fill x -expand true

frame .image.o.overwrite -background $color1
radiobutton .image.o.overwrite.ok -variable overwrite -text "OK to overwrite existing files" -value 1 -background $color1 -highlightthickness 0 
radiobutton .image.o.overwrite.no -variable overwrite -text "Do not overwrite files" -value 0 -background $color1 -highlightthickness 0
pack .image.o.overwrite.ok .image.o.overwrite.no -side top -anchor w

#pack .image.o.label .image.o.name .image.o.overwrite -side top -fill x -pady 10
pack .image.o.label .image.o.name -side top -fill x -pady 10
frame .image.o.buttons -background $color1
button .image.o.buttons.go -text "Align Now" -command Run
button .image.o.buttons.com -text "Save Command File..." -command {
	if [winfo exists .com] {
		raise .com
		focus .com.fileentry
	} else {
		ComFile
	}
}
pack .image.o.buttons.go .image.o.buttons.com -side top -fill both -pady 1 -expand true
pack .image.o.buttons -side top -fill both -padx 20 -pady 20 -expand true

pack .image.s.program -side top -fill y
pack .image.s.title -expand true
pack .image.s.smooth .image.s.config -side bottom -pady 1 -fill y
pack .image.r.title .image.r.file -side top -pady 1 -fill y
pack .image.r.smooth .image.r.config -side bottom -pady 1 -fill y

pack .image.o .image.r .image.s  -side right -fill both -expand true

frame .output

frame .output.config

frame .output.config.modality -background $color2 -relief ridge -borderwidth 3
label .output.config.modality.label -text "Modality" -background $color2
frame .output.config.modality.buttons -background $color2
radiobutton .output.config.modality.buttons.intra -variable modality -text "Intramodality (e.g. PET-PET)" -value 1 -background $color2 -highlightthickness 0 -command Intramodality

radiobutton .output.config.modality.buttons.inter -variable modality -text "Intermodality (e.g. MRI-PET)" -value 2 -background $color2 -highlightthickness 0 -command Intermodality
pack .output.config.modality.buttons.intra .output.config.modality.buttons.inter -side top -anchor w
pack .output.config.modality.label .output.config.modality.buttons -side top -fill x -pady 10 -anchor n

frame .output.config.costfxn -background $color4 -relief ridge -borderwidth 3
label .output.config.costfxn.label -text "Cost Function" -background $color4
frame .output.config.costfxn.choices -background $color4
radiobutton .output.config.costfxn.choices.ratio -variable costfxn -text "Standard Deviation of Ratio Image" -value 1 -background $color4 -highlightthickness 0
radiobutton .output.config.costfxn.choices.leastsq -variable costfxn -text "Least Squares of Difference Image" -value 2 -background $color4 -highlightthickness 0
radiobutton .output.config.costfxn.choices.leastsqrescale -variable costfxn -text "Least Squares with Intensity Rescaling" -value 3 -background $color4 -highlightthickness 0

pack .output.config.costfxn.choices.ratio .output.config.costfxn.choices.leastsq .output.config.costfxn.choices.leastsqrescale -side top -anchor w
pack .output.config.costfxn.label .output.config.costfxn.choices -side top -fill x -pady 10 -anchor n

pack .output.config.modality .output.config.costfxn -side top -fill both -expand true

frame .output.behavior
frame .output.behavior.sampling -background $color6 -relief ridge -borderwidth 3
label .output.behavior.sampling.label -text "Sampling" -background $color6

frame .output.behavior.sampling.options -background $color6
frame .output.behavior.sampling.options.ends -background $color6

frame .output.behavior.sampling.options.ends.initial -background $color6
label .output.behavior.sampling.options.ends.initial.label -text "Initial:" -background $color6
entry .output.behavior.sampling.options.ends.initial.entry -textvariable initial -width 4
pack .output.behavior.sampling.options.ends.initial.label .output.behavior.sampling.options.ends.initial.entry -side left

frame .output.behavior.sampling.options.ends.final -background $color6
label .output.behavior.sampling.options.ends.final.label -text "Final:" -background $color6
entry .output.behavior.sampling.options.ends.final.entry -textvariable final -width 4
pack .output.behavior.sampling.options.ends.final.label .output.behavior.sampling.options.ends.final.entry -side left


pack .output.behavior.sampling.options.ends.initial .output.behavior.sampling.options.ends.final -side left -fill x -padx 8 -expand true

frame .output.behavior.sampling.options.middle -background $color6
label .output.behavior.sampling.options.middle.label -text "Decrement by factors of:" -background $color6
entry .output.behavior.sampling.options.middle.entry -textvariable decrement -width 4
pack .output.behavior.sampling.options.middle.label .output.behavior.sampling.options.middle.entry -side left

pack .output.behavior.sampling.options.ends .output.behavior.sampling.options.middle -side top -pady 1

pack .output.behavior.sampling.label .output.behavior.sampling.options -side top -pady 10 -fill x -anchor n

frame .output.behavior.converge 

frame .output.behavior.converge.threshold -background $color7 -relief ridge -borderwidth 3
label .output.behavior.converge.threshold.label -text "Convergence Threshold" -background $color7
entry .output.behavior.converge.threshold.entry -textvariable converge -width 12
pack .output.behavior.converge.threshold.label .output.behavior.converge.threshold.entry -side top -pady 10

frame .output.behavior.converge.iterations -background $color8 -relief ridge -borderwidth 3
label .output.behavior.converge.iterations.label -text "Iterations (per sampling)" -background $color8

frame .output.behavior.converge.iterations.total -background $color8
label .output.behavior.converge.iterations.total.label -text "Total:" -background $color8
entry .output.behavior.converge.iterations.total.entry -textvariable iterations -width 5
pack .output.behavior.converge.iterations.total.label .output.behavior.converge.iterations.total.entry -side left

frame .output.behavior.converge.iterations.nobetter -background $color8
label .output.behavior.converge.iterations.nobetter.label -text "Without improvement:" -background $color8
entry .output.behavior.converge.iterations.nobetter.entry -textvariable unimproved -width 5
pack .output.behavior.converge.iterations.nobetter.label .output.behavior.converge.iterations.nobetter.entry -side left

pack .output.behavior.converge.iterations.label .output.behavior.converge.iterations.total .output.behavior.converge.iterations.nobetter -side top -pady 10

pack .output.behavior.converge.threshold .output.behavior.converge.iterations -side left -fill both -expand true


pack .output.behavior.sampling .output.behavior.converge -side top -fill both -expand true

frame .output.model -background $color9 -relief ridge -borderwidth 3
label .output.model.label -text "Spatial Model" -background $color9
frame .output.model.twod -background $color9
label .output.model.twod.label -text "2D" -background $color9
frame .output.model.twod.buttons -background $color9
radiobutton .output.model.twod.buttons.rigid -variable model -text "Rigid Body" -value 23 -background $color9 -highlightthickness 0
radiobutton .output.model.twod.buttons.global -variable model -text "Global Rescaling" -value 24 -background $color9 -highlightthickness 0
radiobutton .output.model.twod.buttons.fixed -variable model -text "Fixed Area" -value 25 -background $color9 -highlightthickness 0
radiobutton .output.model.twod.buttons.affine -variable model -text "Affine" -value 26 -background $color9 -highlightthickness 0
radiobutton .output.model.twod.buttons.perspective -variable model -text "Perspective" -value 28 -background $color9 -highlightthickness 0
pack .output.model.twod.buttons.rigid .output.model.twod.buttons.global .output.model.twod.buttons.fixed .output.model.twod.buttons.affine .output.model.twod.buttons.perspective -side top -fill y -expand true -anchor w
pack .output.model.twod.label .output.model.twod.buttons -side left -fill both -expand true

frame .output.model.threed -background $color9
label .output.model.threed.label -text "3D" -background $color9
frame .output.model.threed.buttons -background $color9
radiobutton .output.model.threed.buttons.rigid -variable model -text "Rigid Body" -value 6 -background $color9 -highlightthickness 0
radiobutton .output.model.threed.buttons.global -variable model -text "Global Rescaling" -value 7 -background $color9 -highlightthickness 0
radiobutton .output.model.threed.buttons.tala -variable model -text "Traditional" -value 9 -background $color9 -highlightthickness 0
radiobutton .output.model.threed.buttons.affine -variable model -text "Affine" -value 12 -background $color9 -highlightthickness 0
radiobutton .output.model.threed.buttons.perspective -variable model -text "Perspective" -value 15 -background $color9 -highlightthickness 0

pack .output.model.threed.buttons.rigid .output.model.threed.buttons.global .output.model.threed.buttons.tala .output.model.threed.buttons.affine .output.model.threed.buttons.perspective -side top -expand true -anchor w
pack .output.model.threed.label .output.model.threed.buttons -side left -fill both -expand true

pack .output.model.label .output.model.twod .output.model.threed -side top -fill both -expand true -pady 10


pack .image .output.config .output.behavior .output.model -side left -fill both -expand true

frame .flags -background $color10
label .flags.label -text "Additional program flags:" -background $color10
entry .flags.entry -textvariable flags
pack .flags.label -side left
pack .flags.entry -side right -fill x -expand true

wm title . "AIR 5.0 Alignment Tool"


pack .image .output .flags -side top -fill both -expand true
#pack .image .output -side top -fill both -expand true

set threshold1 55
set threshold2 55
set xsmooth1 0.0
set ysmooth1 0.0
set zsmooth1 0.0
set xsmooth2 0.0
set ysmooth2 0.0
set zsmooth2 0.0
set airsuffix ".air"
set costfxn 1
set model 6
set modality 1
set partitions1 1
.image.s.config.partentry config -state disabled
set partitions2 1
.image.r.config.partentry config -state disabled
set intermodality 2
set direction 0
set initial 81
set final 1
set decrement 3
set iterations 25
set unimproved 5
set converge .00001
set flags ""
set overwrite 1
set program "alignlinear"
.image.s.program.entry xview moveto 1

set answer { }


