LIBS=-L../cvio -I../cvio -lcvio -lz -lm -pthread

all: cvio_avg cvio_combine cvio_deconirf cvio_diff cvio_mux\
	cvio_speech_subtract cvio_terminate cvio_timegate real_time_stim

cvio_avg: cvio_avg.c
	$(CC) cvio_avg.c -o cvio_avg $(LIBS)

cvio_combine: cvio_combine.c
	$(CC) cvio_combine.c -o cvio_combine $(LIBS)

cvio_deconirf: cvio_deconirf.c
	$(CC) cvio_deconirf.c -o cvio_deconirf $(LIBS)

cvio_diff: cvio_diff.c
	$(CC) cvio_diff.c -o cvio_diff $(LIBS)

cvio_mux: cvio_mux.c
	$(CC) cvio_mux.c -o cvio_mux $(LIBS)

cvio_speech_subtract: cvio_speech_subtract.c
	$(CC) cvio_speech_subtract.c -o cvio_speech_subtract $(LIBS)

cvio_terminate: cvio_terminate.c
	$(CC) cvio_terminate.c -o cvio_terminate $(LIBS)

cvio_timegate: cvio_timegate.c
	$(CC) cvio_timegate.c -o cvio_timegate $(LIBS)

real_time_stim: real_time_stim.c
	$(CC) real_time_stim.c -o real_time_stim $(LIBS)
