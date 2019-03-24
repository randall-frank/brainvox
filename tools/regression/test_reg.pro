PRO test_reg

	reg = reg_create([-1,0], 6, [0,1,1,1,1,0])
	print, "Create:",reg

	x = fltarr(6,2)
	y = fltarr(6)

for pass=0,2 do begin
	seed = 1000L
	for i=0,9 do begin
		for j=0,5 do begin
			y[j] = j + i*(j+0.4)
			if (j and 1) then begin
				r = randomu(seed)*2.0-1.0
				y[j] = y[j] + r*0.1*float(j)
			end
			x[j,0] = 1.0;
			x[j,1] = i;
			if (pass GT 0) then begin
				y[j] = y[j] + 5.5
			end
		end
		if (pass LT 2) then begin
			status = reg_add_obs(reg,y,x)
		end else begin
			status = reg_remove_obs(reg,y,x)
		end
		print, "Add:",status
	end
	

	status = reg_calc_ss(reg, [0,1,1,1,1,0], 0, sst, sse, ssv)
	print, "SS:",status
	help,sst,sse,ssv
	print, sst
	print, sse
	print, ssv

	status = reg_calc_coef(reg, coef)
	print, "Coef:",status
	help,coef
	print, coef

	status = reg_calc_coef(reg, coef, STD_ERROR=se)
	print, "Std Error:",status
	help,se
	print, se
end

	status = reg_destroy(reg)
	print, "Destroy:",status

end

