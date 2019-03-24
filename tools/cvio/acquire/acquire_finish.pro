pro acquire_finish,length,rate,channels,files,imgchn,imgfile,nimgs,basnum

for i=0,n_elements(channels)-1 do begin

	if (channels[i] GE 0) then begin

		spawn, "cvio_cp :shmem:" + files[i] + " " + files[i], ret
		print, "cp:",ret

		ret = cvio_delete(":shmem:"+files[i])
		print, "delete:",ret

	end

end

if (imgchn GE 0) then begin
	spawn, "cvio_cp :shmem:"+imgfile+ " " + imgfile

	ret = cvio_delete(":shmem:"+imgfile)
	print, "delete:",ret
end

end

