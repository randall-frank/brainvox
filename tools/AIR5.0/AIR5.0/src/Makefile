# Generated automatically from Makefile.in by configure.
CFLAGS = -g -O2
CC = gcc
RANLIB = ranlib

libRPW.a : align.o alignwarp.o antiwarp2D.o antiwarp3D.o balanc.o balbak.o blat100.o blat1000.o blat40.o blat400.o caxpy.o cdiver.o cdotc.o cdvasb.o cexp.o cfftb1.o cfftf1.o cffti1.o cgefa.o cgemm.o cgesl.o chirpterp.o clip_suffix.o clog.o close_header.o combineair.o combinewarp_left.o combinewarp_right.o comloger.o comtloger.o consult_header.o create_vols.o cscal.o csqrt.o d1mach.o daxpy.o ddet.o ddot.o dgedi.o dgefa.o dgemm.o dgesl.o diffg2D.o diffg3D.o dmodposl.o do_align_warp.o do_alignlinear.o do_binarize.o do_binarymask.o do_binarymath.o do_cd_air.o do_cd_warp.o do_combine_air.o do_combine_warp.o do_crop.o do_definecommon_air.o do_determinant_mask.o do_fixheader.o do_fuse.o do_gsmooth.o do_identify.o do_invert_air.o do_layout.o do_magnify.o do_makeaheader.o do_mv_air.o do_mv_warp.o do_overlay_mask.o do_reconcile_air.o do_reorient.o do_resize.o do_reslice.o do_reslice_ucf.o do_reslice_unwarp.o do_reslice_unwarp_ucf.o do_reslice_vector.o do_reslice_warp.o do_reslice_warp_ucf.o do_reunite.o do_separate.o do_softmean.o do_softmeannorescale.o do_stereoproject.o do_straightmean.o do_straightmeannorescale.o do_strictmean.o do_strictmeannorescale.o do_zoomer.o dpofa.o dposl.o dqage.o dqk15.o dqpsrt.o dscal.o dswap.o eexper4.o eloger4.o eloger5.o exchng.o expderivut.o exput.o fclose_decompress.o file_from_path.o flip_x.o flip_y.o flip_z.o fopen_decompress.o fprob_errs.o fprobr.o fprobw.o free_c.o free_d.o free_f.o free_i.o free_vols.o gael.o gausser.o gausssmaller.o hash.o hqr.o hqr2.o hqr3.o icamax.o idamax.o identical.o interp_lin_2D.o interp_lin_3D.o interp_nn_2D.o interp_nn_3D.o interp_wsinc_2D.o interp_wsinc_3D.o interp_wsinc_xy.o interp_wsinc_xz.o interp_wsinc_yz.o inverter.o layerout.o load.o load1.o load1as1.o load_probr.o logderivut.o logut.o magnifier.o map_value.o mask.o matmul.o matrixs.o matrixsc.o matrixsf.o matrixsi.o maxvoxel.o meancalc.o meaner.o modchol.o mulmat.o open_header.o orthes.o ortran.o padetanh.o parse_double.o parse_file_list.o parse_float.o parse_int.o parse_long.o parse_schar.o parse_short.o parse_uchar.o parse_uint.o parse_ulong.o parse_ushort.o passb.o passb2.o passb3.o passb4.o passb5.o passf.o passf2.o passf3.o passf4.o passf5.o pfactor.o promote.o qrstep.o quickread.o quickwrite.o qvderiv2Dwarp.o qvderiv3Dwarp.o qvderivsLS12.o qvderivsLS15.o qvderivsLS6.o qvderivsLS8.o qvderivsN12.o qvderivsN15.o qvderivsN6.o qvderivsN8.o qvderivsRS12.o qvderivsRS15.o qvderivsRS6.o qvderivsRS8.o r_affine_lin.o r_persp_lin.o r_persp_nn.o r_persp_wsinc.o r_persp_wsinc_xy.o r_persp_wsinc_xz.o r_persp_wsinc_yz.o r_scan_chirp.o r_scan_chirp_xy.o r_scan_chirp_xz.o r_scan_chirp_yz.o r_scan_sinc.o r_scan_wsinc.o r_uwrp_2D.o r_uwrp_3D.o r_vec_warp_2D.o r_vec_warp_3D.o r_warp_2D.o r_warp_3D.o radb2.o radb3.o radb4.o radb5.o radbg.o radf2.o radf3.o radf4.o radf5.o radfg.o read_air16.o read_air16via_airw.o read_airfield.o read_airw.o read_image.o read_value.o report_error.o resizer.o rfftb1.o rfftf1.o rffti1.o rg.o rgschur.o rsftocsf.o same_dim.o same_size.o save.o save1as1.o save_probw.o saveas1.o sdpred.o sqrtut.o swap_x_and_y.o swap_x_and_z.o swap_y_and_z.o swapbytes.o sylv.o tritrixs.o uv2D3.o uv2D4.o uv2D5.o uv2D6.o uv2D8.o uv3D12.o uv3D15.o uv3D6.o uv3D7.o uv3D9.o uvderiv2Dwarp.o uvderiv3Dwarp.o uvderivsLS12.o uvderivsLS15.o uvderivsLS6.o uvderivsLS8.o uvderivsN12.o uvderivsN15.o uvderivsN6.o uvderivsN8.o uvderivsRS12.o uvderivsRS15.o uvderivsRS6.o uvderivsRS8.o uvrm.o warp2D.o warp3D.o write_air16.o write_airw.o write_header.o write_value.o zeros.o zoom.o
	ar r $@ $?
	${RANLIB} libRPW.a
	
.c.a :
	${CC} -c ${CFLAGS} $<
	ar rv $@ $*.o
	${RANLIB} libRPW.a
	rm -f $*.o
	
.c.o :
	${CC} ${CFLAGS} -c $<
	
clean:
	rm -f *.o
	rm -f libRPW.a
	
distclean:
	rm -f *.o
	rm -f libRPW.a
	rm -f Makefile
	rm -f config.h
	

