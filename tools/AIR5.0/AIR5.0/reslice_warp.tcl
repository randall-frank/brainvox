#!/usr/local/bin/wish4.0
# Copyright 1995-2001 Roger P. Woods, M.D.
# Modified 12/19/01

# Sinc width box color
set color1 #fffefffaa
# Reslice file box color
set color2 #fffefffaa
# Output file box color
set color3 #fffefffaa
# Interpolation box color
set color4 #fffefffaa
# Flag box
set color5 #fffefffaa

proc GetFileName {itemname filter mode} {
	global result

	#invoke the menubox
	toplevel $itemname 

	if {[string compare $itemname ".select_reslice_file"] == 0} {
		wm title $itemname "AIR File(s)"
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

	#Have the menu updated whenever a return is ente#fbb into the directory field
	bind $itemname.entry <Return> "MenuUpdate $itemname $filter"

	bind $itemname.list <Double-1> "ReturnFile $itemname $filter"

	MenuUpdate $itemname $filter

}

proc ComFile { } {

	global comfile
	global comdir
	global color3

	#invoke a menubox
	toplevel .com
	wm title .com "Command File"

	#create a place to put the directory name
	label .com.lbl -text Directory: -background $color3
	entry .com.entry
	.com.entry config -textvariable comdir
	
	#create a place to put the file name
	label .com.filelbl -text File: -background $color3
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

	global program
	global prefix
	global xwindow
	global ywindow
	global zwindow
	global flags
	global modality
	global overwrite
	global cubic
	global outfile
	exec echo "" >$outfile
	set reslicefiles [.image.r.file.list get 0 end]
	foreach i $reslicefiles {
		set dir [file dirname $i]
		set tail [file tail $i]
		set root [file root $tail]
		set newfile $dir
		append newfile "/"
		append newfile $prefix
		append newfile $root
		set comlist "exec echo $program $i $newfile -n $modality $xwindow $ywindow $zwindow"
		if {$overwrite==1} {
			append comlist "-o "
		}
		if {$cubic!=1} {
			append comlist "-k "
		}
		append comlist $flags
		append comlist ">>"
		append comlist $outfile
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


	destroy .com
}


proc Run { } {
	global program
	global prefix
	global xwindow
	global ywindow
	global zwindow
	global flags
	global modality
	global overwrite
	global cubic
	set reslicefiles [.image.r.file.list get 0 end]
	foreach i $reslicefiles {
		set dir [file dirname $i]
		set tail [file tail $i]
		set root [file root $tail]
		set newfile $dir
		append newfile "/"
		append newfile $prefix
		append newfile $root
		set comlist "exec xterm -e $program $i $newfile -n $modality $xwindow $ywindow $zwindow"
		if {$overwrite==1} {
			append comlist "-o "
		}
		if {$cubic!=1} {
			append comlist "-k "
		}
		append comlist $flags
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
proc Nearest { } {
	global xwindow
	global ywindow
	global zwindow
	global xwindowchoice
	global ywindowchoice
	global zwindowchoice

	if {[string compare $xwindow ""] !=0} {
		set xwindowchoice $xwindow
	}
	if {[string compare $ywindow ""] !=0} {
		set ywindowchoice $ywindow
	}
	if {[string compare $zwindow ""] !=0} {
		set zwindowchoice $zwindow
	}
	.output.behavior.windowing.xyz.x.entry configure -state disabled
	.output.behavior.windowing.xyz.y.entry configure -state disabled
	.output.behavior.windowing.xyz.z.entry configure -state disabled
	set xwindow ""
	set ywindow ""
	set zwindow ""
}
proc Trilinear { } {
	global xwindow
	global ywindow
	global zwindow
	global xwindowchoice
	global ywindowchoice
	global zwindowchoice

	if {[string compare $xwindow ""] !=0} {
		set xwindowchoice $xwindow
	}
	if {[string compare $ywindow ""] !=0} {
		set ywindowchoice $ywindow
	}
	if {[string compare $zwindow ""] !=0} {
		set zwindowchoice $zwindow
	}
	.output.behavior.windowing.xyz.x.entry configure -state disabled
	.output.behavior.windowing.xyz.y.entry configure -state disabled
	.output.behavior.windowing.xyz.z.entry configure -state disabled
	set xwindow ""
	set ywindow ""
	set zwindow ""
}
proc Sincxy { } {
	global xwindow
	global ywindow
	global zwindow
	global xwindowchoice
	global ywindowchoice
	global zwindowchoice

	if {[string compare $zwindow ""] !=0} {
		set zwindowchoice $zwindow
	}
	.output.behavior.windowing.xyz.x.entry configure -state normal
	.output.behavior.windowing.xyz.y.entry configure -state normal
	.output.behavior.windowing.xyz.z.entry configure -state disabled
	set xwindow $xwindowchoice
	set ywindow $ywindowchoice
	set zwindow ""
}
proc Sincxz { } {
	global xwindow
	global ywindow
	global zwindow
	global xwindowchoice
	global ywindowchoice
	global zwindowchoice

	if {[string compare $ywindow ""] !=0} {
		set ywindowchoice $ywindow
	}
	.output.behavior.windowing.xyz.x.entry configure -state normal
	.output.behavior.windowing.xyz.y.entry configure -state disabled
	.output.behavior.windowing.xyz.z.entry configure -state normal
	set xwindow $xwindowchoice
	set ywindow ""
	set zwindow $zwindowchoice
}
proc Sincyz { } {
	global xwindow
	global ywindow
	global zwindow
	global xwindowchoice
	global ywindowchoice
	global zwindowchoice

	if {[string compare $xwindow ""] !=0} {
		set xwindowchoice $xwindow
	}
	.output.behavior.windowing.xyz.x.entry configure -state disabled
	.output.behavior.windowing.xyz.y.entry configure -state normal
	.output.behavior.windowing.xyz.z.entry configure -state normal

	set xwindow ""
	set ywindow $ywindowchoice
	set zwindow $zwindowchoice
}
proc Sinc { } {
	global xwindow
	global ywindow
	global zwindow
	global xwindowchoice
	global ywindowchoice
	global zwindowchoice

	.output.behavior.windowing.xyz.x.entry configure -state normal
	.output.behavior.windowing.xyz.y.entry configure -state normal
	.output.behavior.windowing.xyz.z.entry configure -state normal
	set xwindow $xwindowchoice
	set ywindow $ywindowchoice
	set zwindow $zwindowchoice
}
proc Wsincscan { } {
	global xwindow
	global ywindow
	global zwindow
	global xwindowchoice
	global ywindowchoice
	global zwindowchoice

	.output.behavior.windowing.xyz.x.entry configure -state normal
	.output.behavior.windowing.xyz.y.entry configure -state normal
	.output.behavior.windowing.xyz.z.entry configure -state normal
	set xwindow $xwindowchoice
	set ywindow $ywindowchoice
	set zwindow $zwindowchoice
}
proc Sincscan { } {
	global xwindow
	global ywindow
	global zwindow
	global xwindowchoice
	global ywindowchoice
	global zwindowchoice

	if {[string compare $xwindow ""] !=0} {
		set xwindowchoice $xwindow
	}
	if {[string compare $ywindow ""] !=0} {
		set ywindowchoice $ywindow
	}
	if {[string compare $zwindow ""] !=0} {
		set zwindowchoice $zwindow
	}
	.output.behavior.windowing.xyz.x.entry configure -state disabled
	.output.behavior.windowing.xyz.y.entry configure -state disabled
	.output.behavior.windowing.xyz.z.entry configure -state disabled
	set xwindow ""
	set ywindow ""
	set zwindow ""
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
		foreach f [lsort [glob -nocomplain *$filter*]] {
			$itemname.list insert end $f
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

frame .image.r -background $color2 -relief ridge -borderwidth 3

frame .image.r.program -background $color2
label .image.r.program.label -text "Reslice Program:" -background $color2
entry .image.r.program.entry -textvariable program
pack .image.r.program.label -side left
pack .image.r.program.entry -side right -padx 1 -pady 1 -fill x

	
frame .image.r.file -background $color2

label .image.r.file.label -text "File(s) to Reslice" -background $color2
pack .image.r.file.label
#create a scrolling listbox to put the file in
listbox .image.r.file.list -yscrollcommand [list .image.r.file.sy set] -selectmode extended
scrollbar .image.r.file.sy -orient vertical -command [list .image.r.file.list yview]

frame .image.r.file.buttons -background $color2
button .image.r.file.buttons.add -text "Add..." -command {
	if [winfo exists .select_reslice_file] {
		raise .select_reslice_file
		focus .select_reslice_file.entry
	} else {
		GetFileName .select_reslice_file .warp extended
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
pack .image.r.file.sy -side right -fill both -expand true -pady 3
pack .image.r.file.list -side left -fill both -expand true -pady 3

frame .image.o -background $color3 -relief ridge -borderwidth 3
label .image.o.label -text "Output File(s)" -background $color3
frame .image.o.name -background $color3
entry .image.o.name.entry -textvariable prefix -width 10
label .image.o.name.label -text "Output prefix:" -background $color3
pack .image.o.name.label .image.o.name.entry -side left -padx 1 -fill x -expand true

frame .image.o.overwrite -background $color3
radiobutton .image.o.overwrite.ok -variable overwrite -text "OK to overwrite existing files" -value 1 -background $color3 -highlightthickness 0 
radiobutton .image.o.overwrite.no -variable overwrite -text "Do not overwrite files" -value 0 -background $color3 -highlightthickness 0
pack .image.o.overwrite.ok .image.o.overwrite.no -side top -anchor w -padx 20


frame .image.o.cubic -background $color3
radiobutton .image.o.cubic.ok -variable cubic -text "Generate cubic voxels" -value 1 -background $color3 -highlightthickness 0 
radiobutton .image.o.cubic.no -variable cubic -text "Generate standard voxels" -value 0 -background $color3 -highlightthickness 0
pack .image.o.cubic.ok .image.o.cubic.no -side top -anchor w -padx 20

pack .image.o.label .image.o.name .image.o.overwrite .image.o.cubic -side top -fill x -pady 10
#pack .image.o.label .image.o.name -side top -fill x -pady 10
frame .image.o.buttons -background $color3
button .image.o.buttons.go -text "Reslice Now" -command Run -height 3
button .image.o.buttons.com -text "Save Command File..." -height 3 -command {
	if [winfo exists .com] {
		raise .com
		focus .com.fileentry
	} else {
		ComFile
	}
}
pack .image.o.buttons.go .image.o.buttons.com -side top -fill both -pady 1 -expand true
pack .image.o.buttons -side top -fill both -padx 20 -pady 20 -expand true

pack .image.r.program -side top -pady 1 -fill y
pack .image.r.file -side bottom -padx 5 -pady 30 -fill y -expand true

pack .image.o .image.r  -side right -fill both -expand true

frame .output

frame .output.config

frame .output.config.modality -background $color4 -relief ridge -borderwidth 3
label .output.config.modality.label -text "Interpolation" -background $color4
frame .output.config.modality.buttons -background $color4

radiobutton .output.config.modality.buttons.nearest -variable modality -text "Nearest neighbor" -value 0 -background $color4 -highlightthickness 0 -command Nearest
radiobutton .output.config.modality.buttons.trilinear -variable modality -text "Trilinear" -value 1 -background $color4 -highlightthickness 0 -command Trilinear
radiobutton .output.config.modality.buttons.sincxy -variable modality -text "Windowed sinc xy/Linear z" -value 2 -background $color4 -highlightthickness 0 -command Sincxy
radiobutton .output.config.modality.buttons.sincxz -variable modality -text "Windowed sinc xz/Linear y" -value 3 -background $color4 -highlightthickness 0 -command Sincxz
radiobutton .output.config.modality.buttons.sincyz -variable modality -text "Windowed sinc yz/Linear x" -value 4 -background $color4 -highlightthickness 0 -command Sincyz
radiobutton .output.config.modality.buttons.sinc -variable modality -text "3D windowed sinc" -value 5 -background $color4 -highlightthickness 0 -command Sinc

pack .output.config.modality.buttons.nearest  .output.config.modality.buttons.trilinear .output.config.modality.buttons.sincxy .output.config.modality.buttons.sincxz .output.config.modality.buttons.sincyz .output.config.modality.buttons.sinc -side top -anchor w -fill y

pack .output.config.modality.label -side top -fill x -pady 10 -anchor n
pack .output.config.modality.buttons -side bottom -fill x -fill y -pady 10 -anchor c


pack .output.config.modality -side top -fill both -expand true

frame .output.behavior -background $color1 -relief ridge -borderwidth 3
frame .output.behavior.windowing -background $color1
label .output.behavior.windowing.label -text "Sinc half-window widths\n(voxels)" -background $color1

frame .output.behavior.windowing.xyz -background $color1

frame .output.behavior.windowing.xyz.x -background $color1
label .output.behavior.windowing.xyz.x.label -text "X:" -background $color1
entry .output.behavior.windowing.xyz.x.entry -textvariable xwindow -width 4 -state disabled
pack .output.behavior.windowing.xyz.x.label -side left
pack .output.behavior.windowing.xyz.x.entry -side right

frame .output.behavior.windowing.xyz.y -background $color1
label .output.behavior.windowing.xyz.y.label -text "Y:" -background $color1
entry .output.behavior.windowing.xyz.y.entry -textvariable ywindow -width 4 -state disabled
pack .output.behavior.windowing.xyz.y.label -side left
pack .output.behavior.windowing.xyz.y.entry -side right

frame .output.behavior.windowing.xyz.z -background $color1
label .output.behavior.windowing.xyz.z.label -text "Z:" -background $color1
entry .output.behavior.windowing.xyz.z.entry -textvariable zwindow -width 4 -state disabled
pack .output.behavior.windowing.xyz.z.label -side left
pack .output.behavior.windowing.xyz.z.entry -side right

pack .output.behavior.windowing.label -pady 10
pack .output.behavior.windowing.xyz.x .output.behavior.windowing.xyz.y .output.behavior.windowing.xyz.z -anchor e -pady 5
pack .output.behavior.windowing.xyz -side top -pady 5 -anchor c -expand true


pack .output.behavior.windowing  -fill both -expand true



pack .image .output.config .output.behavior  -side left -fill both -expand true

frame .flags -background $color5
label .flags.label -text "Additional program flags:" -background $color5
entry .flags.entry -textvariable flags
pack .flags.label -side left
pack .flags.entry -side right -fill x -expand true

wm title . "AIR 5.0 Nonlinear Reslice Tool"


pack .image .output .flags -side top -fill both -expand true
#pack .image .output -side top -fill both -expand true

set xwindow ""
set ywindow ""
set zwindow ""
set xwindowchoice 7
set ywindowchoice 7
set zwindowchoice 7
set prefix "r"
set modality 1
set flags ""
set overwrite 0
set cubic 1
set program "reslice_warp"
.image.r.program.entry xview moveto 1

set answer { }


