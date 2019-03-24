pro test_acquire

time = 10L
rate = 10000L
file = ":shmem:junk"
dest = "hades.cvio"

ret = cvio_create(file,time*rate,16,1,[1],0)
print, "create:",ret

cmd = "acquire " + string(time) + " 0 " + string(rate) + " " + file

spawn, cmd, ret
print, "acquire:",ret

spawn, "cvio_cp " + file + " " + dest, ret
print, "cp:",ret

ret = cvio_delete(file)
print, "delete:",ret

end

