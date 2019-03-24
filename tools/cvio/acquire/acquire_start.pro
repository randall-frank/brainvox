pro acquire_start,length,rate,channels,files,imgchn,imgfile,nimgs,basnum

for i=0,n_elements(channels)-1 do begin
	if (channels[i] GE 0) then begin
		ret = cvio_create(":shmem:"+files[i],length*rate,16,1,[1],0)
		print, "create:",ret
	end
end

cmd = "acquire " + string(length)

for i=0,n_elements(channels)-1 do begin
	if (channels[i] GE 0) then begin
		cmd = cmd + " " + string(channels[i]) + " " +  $
			string(rate) + " :shmem:" + files[i]
	end
end

cmd = cmd + " &"

spawn, cmd, ret

if (imgchn GE 0) then begin
	ret = cvio_create(":shmem:"+imgfile,nimgs,16,2,[128,128],0)
	print, "create:",ret

	for i=0,n_elements(channels)-1 do begin
		if (channels[i] eq imgchn) then begin
			infile = ":shmem:"+files[i]
		end
	end
	cmd = "cvio_image_tag "+infile+" :shmem:"+imgfile + " /usr/CVMR " +  $
		string(basnum)+ " " + string(nimgs) + " &"

	print,"CMD:",cmd
	spawn, cmd, ret
end

end

