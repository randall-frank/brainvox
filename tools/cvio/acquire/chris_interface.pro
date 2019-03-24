;----------------------------------------------------------------------
;
;	PURPOSE: event handler
;
;----------------------------------------------------------------------
pro interfaceHandler, sEvent

  if (TAG_NAMES(sEvent, /STRUCTURE_NAME) eq $
  	'WIDGET_KILL_REQUEST') then begin
  	WIDGET_CONTROL, sEvent.top, /DESTROY
  	RETURN
  endif

  WIDGET_CONTROL, sEvent.top, GET_UVALUE = sWidget

  case sEvent.id of

	sWidget.wPIDText : begin
		WIDGET_CONTROL, sWidget.wPIDText, GET_VALUE=temp
		sWidget.PID = temp
		print, 'PID = ', sWidget.PID
	END

	sWidget.wRunNumText: begin
		WIDGET_CONTROL, sWidget.wRunNumText, GET_VALUE=temp
		sWidget.RunNum = temp
		sWidget.filename1 = 'fmri' + sWidget.RunNum + '_card_' + 'raw.cvio'
		sWidget.filename2 = 'fmri' + sWidget.RunNum + '_resp_' + 'raw.cvio'
		sWidget.filename3 = 'fmri' + sWidget.RunNum + '_img_' + 'raw.cvio'
		sWidget.filename4 = 'fmri' + sWidget.RunNum + '_stim_' + 'raw.cvio'
		sWidget.filename5 = 'fmri' + sWidget.RunNum + '_tag_' + 'raw.cvio'
		IF sWidget.CardiacCheck EQ 1 THEN BEGIN
			WIDGET_CONTROL, sWidget.wCollectedCardiacButton, SET_VALUE=sWidget.filename1
			WIDGET_CONTROL, sWidget.wCollectedCardiacButton, SENSITIVE=1	
		ENDIF
		IF sWidget.RespiratoryCheck EQ 1 THEN BEGIN
			WIDGET_CONTROL, sWidget.wCollectedRespiratoryButton, SET_VALUE=sWidget.filename2
			WIDGET_CONTROL, sWidget.wCollectedRespiratoryButton, SENSITIVE=1			
		ENDIF
		IF sWidget.ImagAcqCheck EQ 1 THEN BEGIN
			WIDGET_CONTROL, sWidget.wCollectedImgAcqButton, SET_VALUE=sWidget.filename3
			WIDGET_CONTROL, sWidget.wCollectedImgAcqButton, SENSITIVE=1
		ENDIF
		IF sWidget.StimulusCheck EQ 1 THEN BEGIN
			WIDGET_CONTROL, sWidget.wCollectedStimulusButton, SET_VALUE=sWidget.filename4
			WIDGET_CONTROL, sWidget.wCollectedStimulusButton, SENSITIVE=1
		ENDIF
		print, 'Run Num = ', sWidget.RunNum
	END

	sWidget.wDateText: begin
		WIDGET_CONTROL, sWidget.wDateText, GET_VALUE=temp
		sWidget.Date = temp
	END

	sWidget.wRunLabText: begin
		WIDGET_CONTROL, sWidget.wRunLabText, GET_VALUE=temp
		sWidget.RunLabel = temp
	END

	sWidget.wStimTypeList: begin
		currentStimTypeList = WIDGET_INFO(sWidget.wStimTypeList, /DropList_Select)
		WIDGET_CONTROL, sWidget.wStimTypeList, GET_UVALUE=stimTypeListValues
		sWidget.StimType = stimTypeListValues[currentStimTypeList]
		print, 'Stimulus Type = ', sWidget.stimType
		IF (sWidget.StimulusReady NE 1) THEN BEGIN
			checkStimulus, sWidget
		ENDIF
	END

	sWidget.wStimLiList: begin
		currentStimLiList = WIDGET_INFO(sWidget.wStimLiList, /DropList_Select)
		WIDGET_CONTROL, sWidget.wStimLiList, GET_UVALUE=stimLiListValues
		sWidget.StimList = stimLiListValues[currentStimLiList]
		IF (sWidget.StimulusReady NE 1) THEN BEGIN
			checkStimulus, sWidget
		ENDIF
	END

	sWidget.wISIText: begin
		WIDGET_CONTROL, sWidget.wISIText, GET_VALUE=temp
		sWidget.ISI = temp
		IF (sWidget.StimulusReady NE 1) THEN BEGIN
			checkStimulus, sWidget
		ENDIF
	END

	sWidget.wJitterList: begin
		currentJitterList = WIDGET_INFO(sWidget.wJitterList, /DropList_Select)
		WIDGET_CONTROL, sWidget.wJitterList, GET_UVALUE=jitterListValues
		sWidget.Jitter = jitterListValues[currentJitterList]
		IF (sWidget.StimulusReady NE 1) THEN BEGIN
			checkStimulus, sWidget
		ENDIF
	END

	sWidget.wDurationText: begin
		WIDGET_CONTROL, sWidget.wDurationText, GET_VALUE=temp
		sWidget.Duration = temp
		print, 'Duration = ', sWidget.Duration
		IF (sWidget.StimulusReady NE 1) THEN BEGIN
			checkStimulus, sWidget
		ENDIF
	END

	sWidget.wSourceList: begin
		currentSourceList = WIDGET_INFO(sWidget.wSourceList, /DropList_Select)
		WIDGET_CONTROL, sWidget.wSourceList, GET_UVALUE=sourceListValues
		sWidget.Source = sourceListValues[currentSourceList]
		IF (sWidget.RegressionReady NE 1) THEN BEGIN
			checkRegression, sWidget
		ENDIF
	END

	sWidget.wNumImgText: begin
		WIDGET_CONTROL, sWidget.wNumImgText, GET_VALUE=temp
		sWidget.NumImg = temp
		IF (sWidget.RegressionReady NE 1) THEN BEGIN
			checkRegression, sWidget
		ENDIF
	END

	sWidget.wNumSliText: begin
		WIDGET_CONTROL, sWidget.wNumSliText, GET_VALUE=temp
		sWidget.NumSli = temp
		IF (sWidget.RegressionReady NE 1) THEN BEGIN
			checkRegression, sWidget
		ENDIF
	END
	
	sWidget.wRateText : begin
		WIDGET_CONTROL, sWidget.wRateText, GET_VALUE=temp
		sWidget.Rate = temp
		print, 'Rate = ', sWidget.Rate
		IF (sWidget.RegressionReady NE 1) THEN BEGIN
			checkRegression, sWidget
		ENDIF
	END

	sWidget.wRunNumText : begin
		WIDGET_CONTROL, sWidget.wRunNumText, GET_VALUE=temp
		sWidget.RunNum = temp
		print, 'RunNum = ', sWidget.RunNum
		IF (sWidget.RegressionReady NE 1) THEN BEGIN
			checkRegression, sWidget
		ENDIF
	END

	sWidget.wCVIOCardiacButton : begin
		IF sWidget.CardiacCheck NE 1 THEN BEGIN
			WIDGET_CONTROL, sWidget.wCVIOCardiacButton, SET_VALUE='*Cardiac*'
			sWidget.Channels = sWidget.Channels + 1
			WIDGET_CONTROL, sWidget.wCardiacButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wCollectedCardiacButton, SET_VALUE=sWidget.filename1
			WIDGET_CONTROL, sWidget.wCollectedCardiacButton, SENSITIVE=1
			sWidget.Acquire_Channels[0] = 0
			sWidget.Acquire_Files[0] = sWidget.filename1
			sWidget.CardiacCheck = 1
		ENDIF ELSE BEGIN
			WIDGET_CONTROL, sWidget.wCVIOCardiacButton, SET_VALUE='Cardiac'
			sWidget.Channels = sWidget.Channels - 1
			WIDGET_CONTROL, sWidget.wCollectedCardiacButton, SET_VALUE='Cardiac Data'
			WIDGET_CONTROL, sWidget.wCollectedCardiacButton, SENSITIVE=0
			WIDGET_CONTROL, sWidget.wCardiacButton, SENSITIVE=0
			sWidget.Acquire_Channels[0] = -1
			sWidget.Acquire_Files[0] = ''
			sWidget.CardiacCheck = -1
		ENDELSE
		print, 'cardiac check = ', sWidget.CardiacCheck
	END

	sWidget.wCVIORespiratoryButton : begin
		IF sWidget.RespiratoryCheck EQ -1 THEN BEGIN
			WIDGET_CONTROL, sWidget.wCVIORespiratoryButton, SET_VALUE='*Respiratory*'
			sWidget.Channels = sWidget.Channels + 1
			WIDGET_CONTROL, sWidget.wRespiratoryButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wCollectedRespiratoryButton, SET_VALUE=sWidget.filename2
			WIDGET_CONTROL, sWidget.wCollectedRespiratoryButton, SENSITIVE=1
			sWidget.Acquire_Channels[1] = 1
			sWidget.Acquire_Files[1] = sWidget.filename2			
			sWidget.RespiratoryCheck = 1
		ENDIF ELSE BEGIN
			WIDGET_CONTROL, sWidget.wCVIORespiratoryButton, SET_VALUE='Respiratory'
			sWidget.Channels = sWidget.Channels - 1
			WIDGET_CONTROL, sWidget.wCollectedRespiratoryButton, SET_VALUE='Respiratory Data'
			WIDGET_CONTROL, sWidget.wCollectedRespiratoryButton, SENSITIVE=0
			WIDGET_CONTROL, sWidget.wRespiratoryButton, SENSITIVE=0
			sWidget.Acquire_Channels[1] = -1
			sWidget.Acquire_Files[1] = ''
			sWidget.RespiratoryCheck = -1
		ENDELSE
	END

	sWidget.wCVIOImagAcqButton : begin
		IF sWidget.ImagAcqCheck EQ -1 THEN BEGIN
			WIDGET_CONTROL, sWidget.wCVIOImagAcqButton, SET_VALUE='*Image Acquisition*'
			sWidget.Channels = sWidget.Channels + 1
			WIDGET_CONTROL, sWidget.wImagAcqButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wCollectedImgAcqButton, SET_VALUE=sWidget.filename3
			WIDGET_CONTROL, sWidget.wCollectedImgAcqButton, SENSITIVE=1
			sWidget.Acquire_Channels[2] = 2
			sWidget.Acquire_Files[2] = sWidget.filename3
			sWidget.ImagAcqCheck = 1	
		ENDIF ELSE BEGIN
			WIDGET_CONTROL, sWidget.wCVIOImagAcqButton, SET_VALUE='Image Acquisition'
			sWidget.Channels = sWidget.Channels - 1
			WIDGET_CONTROL, sWidget.wCollectedImgAcqButton, SET_VALUE='Img Acq Data'
			WIDGET_CONTROL, sWidget.wCollectedImgAcqButton, SENSITIVE=0
			WIDGET_CONTROL, sWidget.wImagAcqButton, SENSITIVE=0
			sWidget.Acquire_Channels[2] = -1
			sWidget.Acquire_Files[2] = ''
			sWidget.ImagAcqCheck = -1
		ENDELSE
	END

	sWidget.wCVIOStimulusButton : begin
		IF sWidget.StimulusCheck EQ -1 THEN BEGIN
			WIDGET_CONTROL, sWidget.wCVIOStimulusButton, SET_VALUE='*Stimulus*'
			sWidget.Channels = sWidget.Channels + 1
			WIDGET_CONTROL, sWidget.wStimulusButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wCollectedStimulusButton, SET_VALUE=sWidget.filename4
			WIDGET_CONTROL, sWidget.wCollectedStimulusButton, SENSITIVE=1
			sWidget.Acquire_Channels[3] = 3
			sWidget.Acquire_Files[3] = sWidget.filename3
			sWidget.StimulusCheck = 1
		ENDIF ELSE BEGIN
			WIDGET_CONTROL, sWidget.wCVIOStimulusButton, SET_VALUE='Stimulus'
			sWidget.Channels = sWidget.Channels - 1
			WIDGET_CONTROL, sWidget.wCollectedStimulusButton, SET_VALUE='Stimulus Data'
			WIDGET_CONTROL, sWidget.wCollectedStimulusButton, SENSITIVE=0
			WIDGET_CONTROL, sWidget.wStimulusButton, SENSITIVE=0
			sWidget.Acquire_Channels[3] = -1
			sWidget.Acquire_Files[3] = ''
			sWidget.StimulusCheck = -1
		ENDELSE
	END

	sWidget.wOrderList: begin
		currentOrderList = WIDGET_INFO(sWidget.wOrderList, /DropList_Select)
		WIDGET_CONTROL, sWidget.wOrderList, GET_UVALUE=orderListValues
		sWidget.Order = orderListValues[currentOrderList]
		IF (sWidget.RegressionReady NE 1) THEN BEGIN
			checkRegression, sWidget
		ENDIF
	END

	sWidget.wWinText : begin
		WIDGET_CONTROL, sWidget.wWinText, GET_VALUE = temp
		sWidget.SlidingWindow = temp
		IF (sWidget.RegressionReady NE 1) THEN BEGIN
			checkRegression, sWidget
		ENDIF
	END

	sWidget.wCardNoneButton : begin
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wCardNoneButton, SET_VALUE='*None*'
		WIDGET_CONTROL, sWidget.wCardPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wCardPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wCardRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wCardConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wCardSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wCardCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wCardLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wCardPostEventButton : begin 
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wCardNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wCardPostEventButton, SET_VALUE='*Post-Event Interval*'
		WIDGET_CONTROL, sWidget.wCardPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wCardRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wCardConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wCardSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wCardCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wCardLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wCardPreEventButton : begin
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wCardNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wCardPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wCardPreEventButton, SET_VALUE='*Pre-Event Interval*'
		WIDGET_CONTROL, sWidget.wCardRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wCardConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wCardSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wCardCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wCardLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wCardRelativeCycleButton : begin
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wCardNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wCardPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wCardPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wCardRelativeCycleButton, $
			SET_VALUE='*Relative Cycle Position*'
		WIDGET_CONTROL, sWidget.wCardConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wCardSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wCardCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wCardLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wCardConvolutionButton : begin
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wCardNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wCardPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wCardPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wCardRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wCardConvolutionButton, $
			SET_VALUE='*Convolution with HRF*'
		WIDGET_CONTROL, sWidget.wCardSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wCardCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wCardLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wCardSineButton : begin
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wCardNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wCardPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wCardPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wCardRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wCardConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wCardSineButton, SET_VALUE='*Sine*'
		WIDGET_CONTROL, sWidget.wCardCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wCardLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wCardCosineButton : begin
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wCardNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wCardPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wCardPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wCardRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wCardConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wCardSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wCardCosineButton, SET_VALUE='*Cosine*'
		WIDGET_CONTROL, sWidget.wCardLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wCardLookupButton : begin 
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wCardNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wCardPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wCardPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wCardRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wCardConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wCardSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wCardCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wCardLookupButton, SET_VALUE='*Lookup Value*'
	END

	sWidget.wRespNoneButton : begin 
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wRespNoneButton, SET_VALUE='*None*'
		WIDGET_CONTROL, sWidget.wRespPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wRespPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wRespRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wRespConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wRespSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wRespCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wRespLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wRespPostEventButton : begin 
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wRespNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wRespPostEventButton, SET_VALUE='*Post-Event Interval*'
		WIDGET_CONTROL, sWidget.wRespPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wRespRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wRespConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wRespSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wRespCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wRespLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wRespPreEventButton : begin
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wRespNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wRespPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wRespPreEventButton, SET_VALUE='*Pre-Event Interval*'
		WIDGET_CONTROL, sWidget.wRespRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wRespConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wRespSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wRespCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wRespLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wRespRelativeCycleButton : begin
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wRespNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wRespPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wRespPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wRespRelativeCycleButton, $
			SET_VALUE='*Relative Cycle Position*'
		WIDGET_CONTROL, sWidget.wRespConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wRespSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wRespCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wRespLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wRespConvolutionButton : begin
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wRespNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wRespPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wRespPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wRespRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wRespConvolutionButton, $
			SET_VALUE='*Convolution with HRF*'
		WIDGET_CONTROL, sWidget.wRespSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wRespCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wRespLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wRespSineButton : begin
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wRespNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wRespPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wRespPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wRespRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wRespConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wRespSineButton, SET_VALUE='*Sine*'
		WIDGET_CONTROL, sWidget.wRespCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wRespLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wRespCosineButton : begin
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wRespNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wRespPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wRespPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wRespRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wRespConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wRespSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wRespCosineButton, SET_VALUE='*Cosine*'
		WIDGET_CONTROL, sWidget.wRespLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wRespLookupButton : begin 
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wRespNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wRespPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wRespPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wRespRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wRespConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wRespSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wRespCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wRespLookupButton, SET_VALUE='*Lookup Value*'
	END

	sWidget.wImgNoneButton : begin 
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wImgNoneButton, SET_VALUE='*None*'
		WIDGET_CONTROL, sWidget.wImgPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wImgPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wImgRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wImgConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wImgSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wImgCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wImgLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wImgPostEventButton : begin 
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wImgNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wImgPostEventButton, SET_VALUE='*Post-Event Interval*'
		WIDGET_CONTROL, sWidget.wImgPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wImgRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wImgConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wImgSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wImgCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wImgLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wImgPreEventButton : begin
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wImgNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wImgPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wImgPreEventButton, SET_VALUE='*Pre-Event Interval*'
		WIDGET_CONTROL, sWidget.wImgRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wImgConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wImgSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wImgCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wImgLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wImgRelativeCycleButton : begin
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wImgNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wImgPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wImgPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wImgRelativeCycleButton, $
			SET_VALUE='*Relative Cycle Position*'
		WIDGET_CONTROL, sWidget.wImgConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wImgSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wImgCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wImgLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wImgConvolutionButton : begin
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wImgNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wImgPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wImgPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wImgRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wImgConvolutionButton, $
			SET_VALUE='*Convolution with HRF*'
		WIDGET_CONTROL, sWidget.wImgSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wImgCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wImgLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wImgSineButton : begin
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wImgNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wImgPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wImgPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wImgRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wImgConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wImgSineButton, SET_VALUE='*Sine*'
		WIDGET_CONTROL, sWidget.wImgCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wImgLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wImgCosineButton : begin
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wImgNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wImgPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wImgPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wImgRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wImgConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wImgSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wImgCosineButton, SET_VALUE='*Cosine*'
		WIDGET_CONTROL, sWidget.wImgLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wImgLookupButton : begin 
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wImgNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wImgPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wImgPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wImgRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wImgConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wImgSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wImgCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wImgLookupButton, SET_VALUE='*Lookup Value*'
	END

	sWidget.wStimNoneButton : begin 
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wStimNoneButton, SET_VALUE='*None*'
		WIDGET_CONTROL, sWidget.wStimPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wStimPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wStimRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wStimConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wStimSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wStimCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wStimLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wStimPostEventButton : begin 
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wStimNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wStimPostEventButton, SET_VALUE='*Post-Event Interval*'
		WIDGET_CONTROL, sWidget.wStimPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wStimRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wStimConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wStimSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wStimCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wStimLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wStimPreEventButton : begin
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wStimNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wStimPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wStimPreEventButton, SET_VALUE='*Pre-Event Interval*'
		WIDGET_CONTROL, sWidget.wStimRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wStimConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wStimSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wStimCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wStimLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wStimRelativeCycleButton : begin
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wStimNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wStimPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wStimPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wStimRelativeCycleButton, $
			SET_VALUE='*Relative Cycle Position*'
		WIDGET_CONTROL, sWidget.wStimConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wStimSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wStimCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wStimLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wStimConvolutionButton : begin
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wStimNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wStimPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wStimPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wStimRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wStimConvolutionButton, $
			SET_VALUE='*Convolution with HRF*'
		WIDGET_CONTROL, sWidget.wStimSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wStimCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wStimLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wStimSineButton : begin
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wStimNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wStimPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wStimPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wStimRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wStimConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wStimSineButton, SET_VALUE='*Sine*'
		WIDGET_CONTROL, sWidget.wStimCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wStimLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wStimCosineButton : begin
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wStimNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wStimPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wStimPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wStimRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wStimConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wStimSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wStimCosineButton, SET_VALUE='*Cosine*'
		WIDGET_CONTROL, sWidget.wStimLookupButton, SET_VALUE='Lookup Value'
	END

	sWidget.wStimLookupButton : begin 
		;sWidget. ... 		Assign variable values
		WIDGET_CONTROL, sWidget.wStimNoneButton, SET_VALUE='None'
		WIDGET_CONTROL, sWidget.wStimPostEventButton, SET_VALUE='Post-Event Interval'
		WIDGET_CONTROL, sWidget.wStimPreEventButton, SET_VALUE='Pre-Event Interval'
		WIDGET_CONTROL, sWidget.wStimRelativeCycleButton, $
			SET_VALUE='Relative Cycle Position'
		WIDGET_CONTROL, sWidget.wStimConvolutionButton, $
			SET_VALUE='Convolution with HRF'
		WIDGET_CONTROL, sWidget.wStimSineButton, SET_VALUE='Sine'
		WIDGET_CONTROL, sWidget.wStimCosineButton, SET_VALUE='Cosine'
		WIDGET_CONTROL, sWidget.wStimLookupButton, SET_VALUE='*Lookup Value*'
	END

	sWidget.wConstantButton : begin
		;sWidget. ... 		Assign variable values 
	END

	sWidget.wAutoRegressionButton : begin
		;sWidget. ... 		Assign variable values
	END

	sWidget.wStimulusList : begin
		currentStimulusList = WIDGET_INFO(sWidget.wStimulusList, /DropList_Select)
		WIDGET_CONTROL, sWidget.wStimulusList, GET_UVALUE=stimulusListValues
		sWidget.StimStatus = stimulusListValues[currentStimulusList]
		IF (sWidget.StimStatus EQ 'Yes') AND (sWidget.StimulusReady EQ 1) THEN BEGIN
			; load stimulus
			loadStimulus, sWidget
		ENDIF ELSE BEGIN
			; do nothing
			WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Must Enter All' + $
			' Necessary Stimulus Information Before Images Can Be Loaded'
		ENDELSE
	END

	sWidget.wFeedbackList : begin
		currentFeedbackList = WIDGET_INFO(sWidget.wFeedbackList, /DropList_Select)
		WIDGET_CONTROL, sWidget.wFeedbackList, GET_UVALUE = feedbackListValues
		sWidget.FeedbackType = feedbackListValues[currentFeedbackList]
		IF (sWidget.FeedbackType NE 'None') THEN BEGIN
			; give appropriate form of feedback
			IF (sWidget.FeedbackType EQ 'Image - Visual') THEN BEGIN
				RETURN
			ENDIF
			IF (sWidget.FeedbackType EQ 'Image - Auditory') THEN BEGIN
				RETURN
			ENDIF
			IF (sWidget.FeedbackType EQ 'Stripchart') THEN BEGIN
				RETURN
			ENDIF
		ENDIF
	END

	sWidget.wStripchartList : begin
		currentStripchartList = WIDGET_INFO(sWidget.wStripchartList, /DropList_Select)
		WIDGET_CONTROL, sWidget.wStripchartList, GET_UVALUE=stripchartListValues
		sWidget.StripchartType = stripchartListValues[currentStripchartList]
		IF (sWidget.Source EQ 'Disk') THEN BEGIN
			WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Stripchart' + $
			'image only possible if in real time mode'
		ENDIF ELSE BEGIN
			IF (sWidget.StripchartType EQ 'Sum of Squares') THEN BEGIN
				; do nothing
			ENDIF
			IF (sWidget.StripchartType EQ 'Regression Coefficient') THEN BEGIN
				; do nothing
			ENDIF
			IF (sWidget.StripchartType EQ 'Effect Size') THEN BEGIN
				; do nothing
			ENDIF
			IF (sWidget.StripchartType EQ 'F-statistic') THEN BEGIN
				; do nothing
			ENDIF
			IF (sWidget.StripchartType EQ 't-statistic') THEN BEGIN
				; do nothing
			ENDIF
		ENDELSE
	END

	sWidget.wOutputList : begin
		currentOutputList = WIDGET_INFO(sWidget.wOutputList, /DropList_Select)
		WIDGET_CONTROL, sWidget.wOutputList, GET_UVALUE=outputListValues
		sWidget.OutputType = outputListValues[currentOutputList]
		IF (sWidget.OutputType EQ 'Sum of Squares') THEN BEGIN
			; do nothing 
		ENDIF
		IF (sWidget.StripchartType EQ 'Regression Coefficien') THEN BEGIN
			; do nothing
		ENDIF
		IF (sWidget.StripchartType EQ 'Effect Size') THEN BEGIN
			; do nothing
		ENDIF
		IF (sWidget.StripchartType EQ 'F-statistic') THEN BEGIN
			; do nothing
		ENDIF
		IF (sWidget.StripchartType EQ 't-statistic') THEN BEGIN
			; do nothing
		ENDIF
	END

	sWidget.wOutputFileList : begin
		currentOutputFileList = WIDGET_INFO(sWidget.wOutputFileList, /DropList_Select)
		WIDGET_CONTROL, sWidget.wOutputFileList, GET_UVALUE=outputFileListValues
		sWidget.OutputFileType = outputFileListValues[currentOutputFileList]
	END

	sWidget.wInputFileText : begin
		WIDGET_CONTROL, sWidget.wInputFileText, GET_VALUE = temp
		sWidget.InputFile = temp
	END

	sWidget.wOutputFileText : begin
		WIDGET_CONTROL, sWidget.wOutputFileText, GET_VALUE = temp
		sWidget.OutputFile = temp
		IF (sWidget.OutputFileType EQ '') THEN BEGIN
			WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Must Enter Output' + $
			' File Type First'
		ENDIF ELSE BEGIN
			IF (sWidget.OutputFileType EQ 'Static') THEN BEGIN
				OPENW, 20, FILEPATH(sWidget.OutputFile, ROOT_DIR=sWidget.output_dir)
			ENDIF ELSE BEGIN
				err = CVIO_CREATE(sWidget.OutputFile, 100, 16, 1, [1], 0)
				err = CVIO_OPEN(sWidget.OutputFile, 2, write_out)
			ENDELSE
		ENDELSE
		WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Output File Successfully' + $
		' Created'
	END

	sWidget.wSaveButton : begin
		plotStripChart, sWidget
		IF (sWidget.OutputFile EQ '') OR (sWidget.InputFile EQ '') THEN BEGIN
			WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Must Enter Output File' + $
			' Type and Input and Output File Names First'
		ENDIF ELSE BEGIN
			IF (sWidget.OutputFileType EQ 'Static') THEN BEGIN
				writeInputs, sWidget
				WRITEU, 20, result_dir
				CLOSE, 20
				WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Information ' + $
				'Successfully Saved'
			ENDIF ELSE BEGIN
				writeInputs, sWidget
				err = CVIO_ADD_SAMPLES(write_out, (indgen(10)+10)*10,10*indgen(10),10)
				err = CVIO_CLOSE(write_out)
				WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Information ' + $
				'Successfully Saved'
			ENDELSE
		ENDELSE
	END

	sWidget.wROIFileText : begin
		WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Loading desired original' + $
  		' image'

		WIDGET_CONTROL, sWidget.wROIFileText, GET_VALUE=temp1

  		slice = intarr(sWidget.orig_sizex, sWidget.orig_sizey)

  		openr, 1, FILEPATH(temp1, ROOT_DIR = sWidget.origImage_dir)
  		readu, 1, slice
  		close, 1

  		; Scale images into grayscale and increase size
  		slice = REVERSE (slice, 2)
  		slice = (bytscl(slice))/2
  		big_slice = REBIN(slice, sWidget.disp_sizex, sWidget.disp_sizey, /SAMPLE)

  		WSET, sWidget.orig_win_id
  		TV, big_slice

  		WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = ''
	END

	sWidget.wROIButton : begin
		IF (sWidget.wROIFile EQ '') THEN BEGIN
			WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Must Enter an' +  $
			' Image Name to be Loaded'
		ENDIF ELSE BEGIN
			movbox, x, y, dx, dy, 4
			sWidget.ROI = [ x, y, dx, dy ]
		ENDELSE
		checkRegression, sWidget
	END

	sWidget.wBaseNumText: begin
		WIDGET_CONTROL, sWidget.wBaseNumText, GET_VALUE=temp
		sWidget.BaseNum = temp
		print, 'Run Num = ', sWidget.RunNum
	END
	
	sWidget.wGoButton : begin
		acquire_start, sWidget.Duration, sWidget.Rate, sWidget.Acquire_Channels, sWidget.Acquire_Files, $
		3, sWidget.filename5, sWidget.BaseNum, sWidget.NumImg
		wait, sWidget.Duration+2
		acquire_finish, sWidget.Duration, sWidget.Rate, sWidget.Acquire_Channels, sWidget.Acquire_Files, $
		3, sWidget.filename5, sWidget.BaseNum, sWidget.NumImg
	END

	sWidget.wExitButton : begin
		runProgram, sWidget
	END

  ENDCASE

  WIDGET_CONTROL, sEvent.top, SET_UVALUE=sWidget

END ; of function interface handler

;--------------------------------------------------------------------------------
;
;	PURPOSE: interface initiation
;
;--------------------------------------------------------------------------------
pro interfacePrep, wBigBase

	WIDGET_CONTROL, wBigBase, GET_UVALUE = sWidget 

  	; Load split color table
 	;testColors

  	; Set ID's for draw windows
  	WIDGET_CONTROL, sWidget.wDrawOriginal, GET_VALUE=win_temp
  	sWidget.orig_win_id = win_temp

  	WIDGET_CONTROL, sWidget.wDrawOutput, GET_VALUE=win_temp
  	sWidget.result_win_id = win_temp

  	WIDGET_CONTROL, sWidget.wDrawChart, GET_VALUE=win_temp
  	sWidget.chart_win_id = win_temp

  	WIDGET_CONTROL, sWidget.wDrawStimulus, GET_VALUE=win_temp
  	sWidget.stim_win_id = win_temp

  	WIDGET_CONTROL, wBigBase, SET_UVALUE = sWidget 

END

;--------------------------------------------------------------------------------
;
;	PURPOSE: check to see if all necessary stimulus information
;		 has been entered and if so call the stimulus function
;
;--------------------------------------------------------------------------------
pro checkStimulus, sWidget

	; determine whether each necessary value has been entered
	IF (sWidget.StimType NE '') AND (sWidget.StimList NE '') AND (sWidget.ISI NE -1) $
		AND (sWidget.Jitter NE '') AND (sWidget.Duration NE -1) THEN BEGIN
		; Change variable to signify all necessary information has been entered
		sWidget.StimulusReady = 1

		WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'All Necessary Stimlus' +  $
		' Information has been Entered'
	ENDIF ELSE BEGIN
		; do nothing
		RETURN
	ENDELSE

	; display stimulust if all necessary information has been entered
	IF (sWidget.StimStatus EQ 'Yes') AND (sWidget.StimulusReady EQ 1) THEN BEGIN
		; load stimulus
		loadStimulus, sWidget
	ENDIF ELSE BEGIN
		; do nothing
		WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Must Enter All' + $
		' Necessary Stimulus Information Before Images Can Be Loaded'
	ENDELSE

END ; of function checkStimulus

;--------------------------------------------------------------------------------
;
;	PURPOSE: load stimulus data from specified location
;
;--------------------------------------------------------------------------------
pro loadStimulus, sWidget

  	; Create array to place stimulus data in
  	slice = intarr(sWidget.orig_sizex, sWidget.orig_sizey)

  	; determine whether loading from shared memory or disk
  	IF (sWidget.Source EQ 'Shared Memory') THEN BEGIN

		RETURN ; Can we get stimulus from shared memory?

	ENDIF ELSE BEGIN
		FOR it=1, sWidget.duration DO BEGIN

			; determine filename based on user input

			OPENR, 1, FILEPATH(filename, ROOT_DIR=sWidget.stimulus_dir)
			READU, 1, slice
			CLOSE, 1

			; Scale images
			slice = REVERSE(slice, 2)
			slice = (BYTSCL(slice))/2
			big_slice = REBIN(slice, sWidget.disp_sizex, sWidget.disp_sizey, $
				/SAMPLE)
			sWidget.stimImgArray(*,*,it) = big_slice
		ENDFOR
	ENDELSE

END ; of function loadStimulus

;--------------------------------------------------------------------------------
;
;	PURPOSE: check to see if all necessary regression information
;			 has been entered and if so call the regression function
;
;--------------------------------------------------------------------------------
pro checkRegression, sWidget

	; determine whether each necessary value have been entered
	IF (sWidget.NumImg NE -1) AND (sWidget.NumSli NE -1) AND (sWidget.Order NE '') AND $
	   (sWidget.Model NE '') AND (sWidget.SlidingWindow NE -1) AND (sWidget.ROI[0] NE -1) $
	   AND (sWidget.ROI[1] NE -1) AND (sWidget.ROI[2] NE -1) AND (sWidget.ROI[3] NE -1) $
	   THEN BEGIN
		; Change variable to signify all necessary information has been entered
		sWidget.RegressionReady = 1

		WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'All Necessary' +  $
		' Regression Information has been Entered'
	ENDIF ELSE BEGIN
		; do nothing
		RETURN
	ENDELSE

END ; of function checkRegression

;--------------------------------------------------------------------------------
;
;	PURPOSE: add new attributes to the header
;
;--------------------------------------------------------------------------------
pro headerInfo, sWidget

	err = CVIO_OPEN(sWidget.OutputFile, 2, file)

	err = CVIO_SETATTRIBUTE(file, "PID_ST", sWidget.PID)
	err = CVIO_SETATTRIBUTE(file, "Date_ST", sWidget.Date)
	err = CVIO_SETATTRIBUTE(file, "Run_Number_ST", sWidget.RunNum)
	err = CVIO_SETATTRIBUTE(file, "Run_Label_ST", sWidget.RunLabel)
	err = CVIO_SETATTRIBUTE(file, "Stimulus_Type_ST", sWidget.StimType)
	err = CVIO_SETATTRIBUTE(file, "Stimulus_List_ST", sWidget.StimList)
	err = CVIO_SETATTRIBUTE(file, "ISI_ST", sWidget.ISI)
	err = CVIO_SETATTRIBUTE(file, "Jitter_Present_ST", sWidget.Jitter)
	err = CVIO_SETATTRIBUTE(file, "Duration_ST", sWidget.Duration)
	err = CVIO_SETATTRIBUTE(file, "Source_ST", sWidget.Source)
	err = CVIO_SETATTRIBUTE(file, "Number_Images_ST", sWidget.NumImg)
	err = CVIO_SETATTRIBUTE(file, "Number_Slices_ST", sWidget.NumSli)
	err = CVIO_SETATTRIBUTE(file, "Order_ST", sWidget.Order)
	err = CVIO_SETATTRIBUTE(file, "CVIO_Channels_ST", sWidget.Channels)
	err = CVIO_SETATTRIBUTE(file, "Model_ST", sWidget.Model)
	err = CVIO_SETATTRIBUTE(file, "Sliding_Window_Length_ST", sWidget.SlidingWindow)
	err = CVIO_SETATTRIBUTE(file, "Feedback_Present_ST", sWidget.FeedbackType)
	err = CVIO_SETATTRIBUTE(file, "Output_Type_ST", sWidget.OutputFileType)
	err = CVIO_SETATTRIBUTE(file, "Stipchart_Output_ST", sWidget.StripchartType)
	err = CVIO_SETATTRIBUTE(file, "Output_File_Name_ST", sWidget.OutputFile)
	err = CVIO_SETATTRIBUTE(file, "x_Dimension_ST", sWidget.ROI[2])
	err = CVIO_SETATTRIBUTE(file, "y_Dimension_ST", sWidget.ROI[3])

	err = CVIO_CLOSE(file)

END ; of function headerInfo

;--------------------------------------------------------------------------------
;
;	PURPOSE: write user input information to specified file
;
;--------------------------------------------------------------------------------
pro writeInputs, sWidget

	OPENW, 10, FILEPATH(sWidget.InputFile, ROOT_DIR=sWidget.output_dir)

	PRINTF, 10, 'Patient ID: ', sWidget.PID
	PRINTF, 10, 'Date: ', sWidget.Date
	PRINTF, 10, 'Run Number: ', sWidget.RunNum
	PRINTF, 10, 'Run Label: ', sWidget.RunLabel
	PRINTF, 10, 'Stimulus Type: ', sWidget.StimType
	PRINTF, 10, 'Stimulus List: ', sWidget.StimList
	PRINTF, 10, 'Interstimulus Interval: ', sWidget.ISI
	PRINTF, 10, 'Jitter Present: ', sWidget.Jitter
	PRINTF, 10, 'Run Duration: ', sWidget.Duration
	PRINTF, 10, 'Data Source: ', sWidget.Source
	PRINTF, 10, 'Number of Images: ', sWidget.NumImg
	PRINTF, 10, 'Number of Slices: ', sWidget.NumSli
	PRINTF, 10, 'Collection Order: Order'
	PRINTF, 10, 'CVIO Channels: ', sWidget.Channels
	PRINTF, 10, 'Regression Model: ', sWidget.Model
	PRINTF, 10, 'Sliding Window Size: ', sWidget.SlidingWindow
	PRINTF, 10, 'Feedback Type: ', sWidget.FeedbackType
	PRINTF, 10, 'Output Type: ', sWidget.OutputType
	PRINTF, 10, 'Output File Type: ', sWidget.OutputFileType
	PRINTF, 10, 'Output File Name: ', sWidget.OutputFile
	PRINTF, 10, 'ROI Dimensions (x,y): ', sWidget.ROI[2], sWidget.ROI[3]

	WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'File Successfully' + $
	' Created'

	CLOSE, 10

END ; of function writeInputs

;--------------------------------------------------------------------------------
;
;	PURPOSE: plot stripchart of CVIO data
;
;--------------------------------------------------------------------------------
pro plotStripchart, sWidget

	name = ":shmem:cardiac"
	err = CVIO_OPEN(name,1,read_from)

	err = CVIO_GETATTRIBUTE(read_from,"REQ_ATTR_NSAMP_UL",current_count)

	wset, chart_win_id 

	; get the initial sample and plot
	err = CVIO_READ_NEXT_SAMPLES(read_from, time, data, 500)
	PLOT, data

	total_count = 500

	WHILE total_count LT current_count DO BEGIN
		OPLOT, data, color=0
		err = CVIO_READ_NEXT_SAMPLES(read, time, data, 500)
		OPLOT, data, color=255
		total_count = total_count + 500
		wait, 1 ; (don't know if this is necessary or not)
		err = CVIO_GETATTRIBUTE(read_from,"REQ_ATTR_NSAMP_UL",current_count)
	ENDWHILE

END ; of function plotStripchart

;--------------------------------------------------------------------------------
;
;	PURPOSE: perform the desired processes after all information has
;			 been entered
;
;--------------------------------------------------------------------------------
pro runProgram, sWidget

	; Make sure all necessary information has been entered
	IF (RegressionReady EQ 1) AND (StimulusReady EQ 1) THEN BEGIN
		IF (sWidget.OutputFileType EQ 'CVIO') THEN BEGIN
			headerInfo, sWidget
		ENDIF

		; Perform specified regression display specified results
		; Call the stimulus program
		;STIMULUS(sWidget.StimType, sWidget.StimList, sWidget.ISI, sWidget.Jitter, $
		;	sWidget.Duration)

		; Perform necessary calculations to pass info into regression function
		FOR I=0, sWidget.nvars-1 DO sWidget.var_num_data[I] = sWidget.Duration $
				 / var_step[I]

		sWidget.totalImages = sWidget.Duration / sWidget.step

		sWidget.totalCorrelations = sWidget.totalImages / sWidget.correlationNum
		IF (ABS(sWidget.totalCorrelations-DOUBLE(sWidget.totalImages)) / $
			((DOUBLE(sWidget.correlationNum))) GT 0) THEN BEGIN
				sWidget.totalCorrelations = sWidget.totalCorrelations + 1
		ENDIF

		sWidget.nmask = ROI[3]*ROI[2]

		; Create output image array
		sWidget.Output = UIndGen(sWidget.disp_sizex,sWidget.disp_sizey)

		; Call the regression program
		;REGRESSION(sWidget.orig_sizex, sWidget.orig_sizey, sWidget.nmask, $
		;	sWidget.NumImg, sWidget.NumSli, sWidget.Order, sWidget.Model, $
		;	sWidget.SlidingWindow, sWidget.Indpendent, sWidget.Dependent, $
		;	sWidget.Transform, sWidget.Duration, sWidget.Output)

		; Call the CVIO function


		; Do whatever else is necessary


		; Exit the program
		WIDGET_CONTROL, sEvent.top, /DESTROY
		RETURN

	ENDIF ELSE BEGIN

		WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'All Necessary ' +  $
			' Information Has Not Been Entered'

	ENDELSE

END ; of function runProgram

;--------------------------------------------------------------------------------
;
;	PURPOSE: interface creation program
;
;--------------------------------------------------------------------------------
pro chris_interface
device, true=24, decomposed=0
  Title = "To be Determined"
  Name = 'my_interface'

  orig_sizex = 128
  orig_sizey = 128
  disp_sizex = 256
  disp_sizey = 256
  origImage_dir = '/home/people/smyser/project/idl_interface'
  image_dir = '/home/people/smyser/project/idl_interface'
  ouput_dir = '/home/people/smyser/project/idl_interface'
  stimulus_dir = '/home/people/smyser/project/idl_interface'
  result_dir = '/home/people/smyser/project/idl_interface'

  my_font = '-*-times-*-r-*-*-15-*-*-*-*-*-*-*'
  title_font = '-*-times-bold-*-*-*-20-*-*-*-*-*-*-*'

  ; Define original images array
  origImgArray = intarr(disp_sizex, disp_sizey)

  ; Define result images array
  resultImgArray = intarr(disp_sizex, disp_sizey)

  ; Define stimulus images array
  stimImgArray = intarr(disp_sizex, disp_sizey)

  wBigBase = Widget_Base( GROUP_LEADER=wGroup, UNAME='wBigBase'  $
  	,TITLE='Chris' ,SPACE=3 ,/ROW, TLB_FRAME_ATTR=1, MAP=0, $
	/TLB_KILL_REQUEST_EVENTS)

	wBigRightBase = Widget_Base(wBigBase, UNAME = 'wBigRightBase' $
		,/COLUMN)

 	wDemoBase = Widget_Base(wBigRightBase, UNAME='wDemoBase'  $
      		,TITLE='IDL' ,SPACE=3 ,/COLUMN)

  		wDemoLabel = Widget_Label(wDemoBase, UNAME='wDemoLabel'  $
    	  		,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Demographics' $
			,FONT=title_font)

		wDemoTopBase = Widget_Base(wDemoBase, UNAME='wDemoTopBase' $
			,/ROW)

		wPIDLabel = Widget_Label(wDemoTopBase, UNAME='wPIDLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Patient ID:' $
			,FONT=my_font)

  		wPIDText = Widget_Text(wDemoTopBase, UNAME='wPIDText' $
    	  		,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS)

		wDateLabel = Widget_Label(wDemoTopBase, UNAME='wDateLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Date:' ,FONT=my_font)

  		wDateText = Widget_Text(wDemoTopBase, UNAME='wDateText'  $
      			,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS)

		wDemoBottomBase = Widget_Base(wDemoBase, UNAME='wDemoBottomBase' $
			,/ROW)

  		wRunNumLabel = Widget_Label(wDemoBottomBase, UNAME='wRunNumLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Run Number: ' ,FONT=my_font)

  		wRunNumText = Widget_Text(wDemoBottomBase, UNAME='wRunNumText' $
 			,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS)

  		wRunLabLabel = Widget_Label(wDemoBottomBase, UNAME='wRunLabLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Run Label: ' ,FONT=my_font)

  		wRunLabText = Widget_Text(wDemoBottomBase, UNAME='wRunLabText'  $
      			,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS)

  	wDesignBase = Widget_Base(wBigRightBase, UNAME='wDesignBase' $
      		,TITLE='IDL' ,SPACE=3 ,/COLUMN)

  		wDesignLabel = Widget_Label(wDesignBase, UNAME='wDesignLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Design' ,FONT=title_font)

		wDesignTopBase = Widget_Base(wDesignBase, UNAME='wDesignTopBase'  $
			,/ROW)

		stimTypeListValues = [ 'pigs', 'dogs', 'horses' ]
  		wStimTypeList = Widget_Droplist(wDesignTopBase, UNAME='wStimTypeList'  $
      			,TITLE='Stimulus Type: ' ,VALUE=stimTypeListValues ,FONT=my_font, $
			UVALUE=stimTypeListValues)

		stimLiListValues = [ 'peaches', 'apples', 'mangoes' ]
  		wStimLiList = Widget_Droplist(wDesignTopBase, UNAME='wStimLiList'  $
      			,TITLE='Stimulus List: ' ,VALUE = stimLiListValues ,FONT=my_font, $
			UVALUE=stimLiListValues)

		wDesignMiddleBase = Widget_Base(wDesignBase, UNAME='wDesignMiddleBase'  $
			,/ROW)

  		wISILabel = Widget_Label(wDesignMiddleBase, UNAME='wISILabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Mean ISI: ' ,FONT=my_font)

  		wISIText = Widget_Text(wDesignMiddleBase, UNAME='wISIText' $
			,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS)

		jitterListValues = [ 'Yes', 'No' ]
  		wJitterList = Widget_Droplist(wDesignMiddleBase, UNAME='wJitterList'  $
      			,TITLE='Jitter: ' ,VALUE=jitterListValues ,FONT=my_font, $
			UVALUE=jitterListValues)

		wDesignBottomBase = Widget_Base(wDesignBase, UNAME='wDesignBottomBase'  $
			,/ROW)

  		wDurationLabel = Widget_Label(wDesignBottomBase, UNAME='wDurationLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Run Duration (sec): ' ,FONT=my_font)

  		wDurationText = Widget_Text(wDesignBottomBase, UNAME='wDurationText'  $
      			,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS)

  	wCollectionBase = Widget_Base(wBigRightBase, UNAME='wCollectionBase'  $
      		,TITLE='IDL' ,SPACE=3 ,/COLUMN)

  		wCollectionLabel = Widget_Label(wCollectionBase, UNAME='Data Source'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Data Source' ,FONT=title_font)

		wCollectionTopBase = Widget_Base(wCollectionBase, UNAME='wCollectionTopBase'  $
			,/ROW)

		sourceListValues = [ 'Shared Memory', 'Disk' ]
  		wSourceList = Widget_Droplist(wCollectionTopBase, UNAME='wSourceList'  $
      			,TITLE='Run Mode: ' ,VALUE=sourceListValues ,FONT=my_font, $
			UVALUE=sourceListValues)

		orderListValues = [ 'Interleaved', 'Sequential' ]
  		wOrderList = Widget_Droplist(wCollectionTopBase, UNAME='wOrderList'  $
      			,TITLE='Acquisition Order: ' ,VALUE=orderListValues ,FONT=my_font $
			,UVALUE=orderListValues)

		wCollectionMiddleBase = Widget_Base(wCollectionBase,  $
			UNAME='wCollectionMiddleBase' ,/ROW)

  		wNumImgLabel = Widget_Label(wCollectionMiddleBase, UNAME='wNumImgLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Number of Images: ' ,FONT=my_font)

  		wNumImgText = Widget_Text(wCollectionMiddleBase, UNAME='wNumImgText'  $
      			,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS)

  		wNumSliLabel = Widget_Label(wCollectionMiddleBase, UNAME='wNumSliLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Number of Slices: ' ,FONT=my_font)

  		wNumSliText = Widget_Text(wCollectionMiddleBase, UNAME='wNumSliText'  $
      			,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS)

		wCollectionBottomBase = Widget_Base(wCollectionBase, $
			UNAME='wCollectionBottomBase' ,/ROW)

		wChannelsButton = Widget_Button(wCollectionBottomBase, UNAME='wChannelsButton', $
			VALUE='CVIO Channels', /MENU ,FONT=my_font, /DYNAMIC_RESIZE)
	
			wCVIOCardiacButton = Widget_Button(wChannelsButton, $
				UNAME='wCVIOCardiacButton', VALUE='Cardiac', $
				SENSITIVE=1, FONT=my_font, /DYNAMIC_RESIZE)

			wCVIORespiratoryButton = Widget_Button(wChannelsButton, $
				UNAME='wCVIORespiratoryButton', VALUE='Respiratory', $
				SENSITIVE=1, FONT=my_font, /DYNAMIC_RESIZE)

			wCVIOImagAcqButton = Widget_Button(wChannelsButton, $
				UNAME='wCVIOImagAcqButton', VALUE='Image Acquisition', $
				SENSITIVE=1, FONT=my_font, /DYNAMIC_RESIZE)

			wCVIOStimulusButton = Widget_Button(wChannelsButton, $
				UNAME='wCVIOStimulusButton', VALUE='Stimulus', $
				SENSITIVE=1, FONT=my_font, /DYNAMIC_RESIZE)

		wRateLabel = Widget_Label(wCollectionBottomBase, UNAME='wRateLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Collection Rate (Hz): ' ,FONT=my_font)

		wRateText = Widget_Text(wCollectionBottomBase, UNAME='wRateText'  $
      			,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS)

  	wAnalysisBase = Widget_Base(wBigRightBase, UNAME='wAnalysisBase'  $
   		,TITLE='IDL' ,SPACE=3 ,/COLUMN)

  		wAnalysisLabel = Widget_Label(wAnalysisBase, UNAME='wAnalysisLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Analysis' ,FONT=title_font)

		wAnalysisTopBase = Widget_Base(wAnalysisBase, UNAME = 'wAnalysisTopBase' $
			,/ROW)

		wRegressionButton = Widget_Button(wAnalysisTopBase, VALUE='Regression Model', $
			UNAME='wRegressionButton', /Menu ,FONT=my_font, /DYNAMIC_RESIZE)

		wDependentButton = Widget_Button(wRegressionButton, $
			VALUE='Select Dependent Variable', UVALUE='DEPENDENT', SENSITIVE=1, $
			UNAME='wDependentButton', /MENU ,FONT=my_font, /DYNAMIC_RESIZE)

			wDependentVarLabelButton = Widget_Button(wDependentButton, $
				VALUE='Dependent Variable', FONT=my_font, /DYNAMIC_RESIZE, $
				SENSITIVE=0, UNAME='wDependentVarLabel')

			wImageTimeSeriesButton = Widget_Button(wDependentButton, $
				VALUE='MRI Images', $
				UVALUE='IMAGETIMESERIES', SENSITIVE=1, FONT=my_font, $
				UNAME='wImageTimeSeriesButton', /MENU, /DYNAMIC_RESIZE)

				wImageTimeSeriesLabelButton = Widget_Button( $
					wImageTimeSeriesButton, VALUE='Available Data Files', $
					SENSITIVE=0, UNAME='wImageTimeSeriesLabel', FONT=my_font $
					, /DYNAMIC_RESIZE)

				wCollectedCardiacButton = Widget_Button(wImageTimeSeriesButton, $
					VALUE='Cardiac Data', /DYNAMIC_RESIZE, $
					UVALUE='COLLECTEDCARDDATA', SENSITIVE=0 $
					,FONT=my_font ,UNAME='wCollectedCardiacButton')
				
				wCollectedRespiratoryButton = Widget_Button(wImageTimeSeriesButton, $
					VALUE='Respiratory Data', /DYNAMIC_RESIZE, $
					UVALUE='COLLECTEDRESPDATA', SENSITIVE=0 $
					,FONT=my_font ,UNAME='wCollectedRespiratoryButton')

				wCollectedImgAcqButton = Widget_Button(wImageTimeSeriesButton, $
					VALUE='Img Acq Data', /DYNAMIC_RESIZE, $
					UVALUE='COLLECTEDIMGDATA', SENSITIVE=0 $
					,FONT=my_font ,UNAME='wCollectedImgAcqButton')
				
				wCollectedStimulusButton = Widget_Button(wImageTimeSeriesButton, $
					VALUE='Stimulus Data', /DYNAMIC_RESIZE, $
					UVALUE='COLLECTEDSTIMDATA', SENSITIVE=0 $
					,FONT=my_font ,UNAME='wCollectedStimulusButton')
	
		wIndependentButton = Widget_Button(wRegressionButton, $
			VALUE='Select Independent Variable', UVALUE='INDEPENDENT', /MENU, $
			UNAME='wIndependentButton' ,FONT=my_font)

			wIndependentVarLabelButton = Widget_Button(wIndependentButton, $
				VALUE='Available Covariables', $
				SENSITIVE=0, UNAME='wIndependentVarLabel' ,FONT=my_font)
	
			wCardiacButton = Widget_Button(wIndependentButton, VALUE='Cardiac', $
				UVALUE='CARDIAC', SENSITIVE=0, /MENU, UNAME='wCardiacButton' ,FONT=my_font)

				wCardTransformLabelButton = Widget_Button(wCardiacButton, $
					VALUE='Supported Transforms', UVALUE='CARDIAC_TRANSFORM', $
					SENSITIVE=0, UNAME='wCardTransformLabel' ,FONT=my_font)

				wCardNoneButton = Widget_Button(wCardiacButton, $
					SENSITIVE=1, FONT=my_font, VALUE='None', UVALUE='CARDNONE', $
					UNAME='wCardNoneButton', /DYNAMIC_RESIZE)
		
				wCardPostEventButton = Widget_Button(wCardiacButton, FONT=my_font, $
					VALUE='Post-Event Interval', SENSITIVE=1, UVALUE='CARDPOSTEVENT', $
					UNAME='wCardPostEventButton', /DYNAMIC_RESIZE)

				wCardPreEventButton = Widget_Button(wCardiacButton, FONT=my_font, $
					VALUE='Pre-Event Interval', SENSITIVE=1, UVALUE='CARDPREEVENT', $
					UNAME='wCardPreEventButton', /DYNAMIC_RESIZE)

				wCardRelativeCycleButton = Widget_Button(wCardiacButton, FONT=my_font, $
					VALUE='Relative Cycle Position', SENSITIVE=1, UVALUE='CARDRELATIVEPOSITION', $
					UNAME='wCardRelativeCycleButton', /DYNAMIC_RESIZE)

				wCardConvolutionButton = Widget_Button(wCardiacButton, FONT=my_font, $
					VALUE='Convolution with HRF', SENSITIVE=1, UVALUE='CARDCONVOLUTION', $
					UNAME='wCardConvolutionButton', /DYNAMIC_RESIZE)

				wCardSineButton = Widget_Button(wCardiacButton, $
					FONT=my_font, SENSITIVE=1, VALUE='Sine', UVALUE='CARDSINE', $
					UNAME='wCardSineButton', /DYNAMIC_RESIZE)
		
				wCardCosineButton = Widget_Button(wCardiacButton, FONT=my_font, $
					VALUE='Cosine', SENSITIVE=1, UVALUE='CARDCOSINE', $
					UNAME='wCardCosineButton', /DYNAMIC_RESIZE)

				wCardLookupButton = Widget_Button(wCardiacButton, FONT=my_font, $
					VALUE='Lookup Value', SENSITIVE=1, UVALUE='CARDLOOKUP', $
					UNAME='wCardLookupButton', /DYNAMIC_RESIZE)

			wRespiratoryButton = Widget_Button(wIndependentButton, FONT=my_font, $
				VALUE='Respiratory', /MENU, SENSITIVE=0, UVALUE='RESPIRATORY', $
				UNAME='wRespiratoryButton', /DYNAMIC_RESIZE)

				wRespTransformLabelButton = Widget_Button(wRespiratoryButton, $
					FONT=my_font, VALUE='Supported Transforms', $
					UVALUE='RESPIRATORY_TRANSFORM', UNAME='wRespTransformLabel', $
					SENSITIVE=0, /DYNAMIC_RESIZE)

				wRespNoneButton = Widget_Button(wRespiratoryButton, $
					SENSITIVE=1, VALUE='None', UVALUE='RESPNONE', FONT=my_font,$
					UNAME='wRespNoneButton', /DYNAMIC_RESIZE)
		
				wRespPostEventButton = Widget_Button(wRespiratoryButton, $
					 VALUE='Post-Event Interval', $
					SENSITIVE=1, UVALUE='RESPPOSTEVENT', FONT=my_font,$
					UNAME='wRespPostEventButton', /DYNAMIC_RESIZE)

				wRespPreEventButton = Widget_Button(wRespiratoryButton, FONT=my_font, $
					VALUE='Pre-Event Interval', SENSITIVE=1, UVALUE='RESPPREEVENT', $
					UNAME='wRespPreEventButton', /DYNAMIC_RESIZE)

				wRespRelativeCycleButton = Widget_Button(wRespiratoryButton, $
					VALUE='Relative Cycle Position', SENSITIVE=1, $
					UVALUE='RESPRELATIVEPOSITION', FONT=my_font, $
					UNAME='wRespRelativeCycleButton', /DYNAMIC_RESIZE)

				wRespConvolutionButton = Widget_Button(wRespiratoryButton, $
					VALUE='Convolution with HRF', SENSITIVE=1, UVALUE='RESPCONVOLUTION', $
					FONT=my_font, UNAME='wRespConvolutionButton', /DYNAMIC_RESIZE)

				wRespSineButton = Widget_Button(wRespiratoryButton, FONT=my_font, $
					UNAME='wRespSineButton', SENSITIVE=1, VALUE='Sine', UVALUE='RESPSINE' $
					, /DYNAMIC_RESIZE)
		
				wRespCosineButton = Widget_Button(wRespiratoryButton, $
					VALUE='Cosine', FONT=my_font, SENSITIVE=1, UVALUE='RESPCOSINE', $
					UNAME='wRespCosineButton', /DYNAMIC_RESIZE)

				wRespLookupButton = Widget_Button(wRespiratoryButton, FONT=my_font, $
					VALUE='Lookup Value', SENSITIVE=1, UVALUE='RESPLOOKUP', $
					UNAME='wRespLookupButton', /DYNAMIC_RESIZE)

			wImagAcqButton = Widget_Button(wIndependentButton, FONT=my_font, $
				VALUE='Image Acquisition', SENSITIVE=0, UVALUE='IMAGACQ', $
				UNAME='wImagAcqButton', /MENU, /DYNAMIC_RESIZE)

				wImgTransformLabelButton = Widget_Button(wImagAcqButton, $
					VALUE='Supported Transforms', UVALUE='IMAGACQ_TRANSFORM', $
					SENSITIVE=0, UNAME='wImgTransformLabel', FONT=my_font $
					, /DYNAMIC_RESIZE)

				wImgNoneButton = Widget_Button(wImagAcqButton, FONT=my_font, $
					SENSITIVE=1, VALUE='None', UVALUE='IMGNONE', UNAME='wImgNoneButton' $
					, /DYNAMIC_RESIZE)
		
				wImgPostEventButton = Widget_Button(wImagAcqButton, FONT=my_font, $
					VALUE='Post-Event Interval', SENSITIVE=1, UVALUE='IMGPOSTEVENT', $
					UNAME='wImgPostEventButton', /DYNAMIC_RESIZE)

				wImgPreEventButton = Widget_Button(wImagAcqButton, FONT=my_font, $
					VALUE='Pre-Event Interval', SENSITIVE=1, UVALUE='IMGPREEVENT', $
					UNAME='wImgPreEventButton', /DYNAMIC_RESIZE)

				wImgRelativeCycleButton = Widget_Button(wImagAcqButton, $
					VALUE='Relative Cycle Position', SENSITIVE=1, $
					UVALUE='IMGRELATIVEPOSITION', FONT=my_font, $
					UNAME='wImgRelativeCycleButton', /DYNAMIC_RESIZE)

				wImgConvolutionButton = Widget_Button(wImagAcqButton, $
					VALUE='Convolution with HRF', SENSITIVE=1, UVALUE='IMGCONVOLUTION', $
					FONT=my_font, UNAME='wImgConvolutionButton', /DYNAMIC_RESIZE)

				wImgSineButton = Widget_Button(wImagAcqButton, FONT=my_font, $
					SENSITIVE=1, VALUE='Sine', UVALUE='IMGSINE', $
					UNAME='wImgSineButton', /DYNAMIC_RESIZE)
		
				wImgCosineButton = Widget_Button(wImagAcqButton, FONT=my_font, $
					VALUE='Cosine', SENSITIVE=1, UVALUE='IMGCOSINE', $
					UNAME='wImgCosineButton', /DYNAMIC_RESIZE)

				wImgLookupButton = Widget_Button(wImagAcqButton, FONT=my_font, $
					VALUE='Lookup Value', SENSITIVE=1, UVALUE='IMGLOOKUP', $
					UNAME='wImgLookupButton', /DYNAMIC_RESIZE)

			wStimulusButton = Widget_Button(wIndependentButton, FONT=my_font, $
				VALUE='Stimulus', SENSITIVE=0, UVALUE='STIMULUS', $
				UNAME='wStimulusButton', /MENU, /DYNAMIC_RESIZE)

				wStimTransformLabelButton = Widget_Button(wStimulusButton, FONT=my_font, $
					VALUE='Supported Transforms', UVALUE='STIMULUS_TRANSFORM', $
					SENSITIVE=0, UNAME='wStimTransformLabel', /DYNAMIC_RESIZE)

				wStimNoneButton = Widget_Button(wStimulusButton, FONT=my_font, $
					SENSITIVE=1, VALUE='None', UVALUE='STIMNONE', $
					UNAME='wStimNoneButton', /DYNAMIC_RESIZE)
		
				wStimPostEventButton = Widget_Button(wStimulusButton, FONT=my_font, $
					VALUE='Post-Event Interval', SENSITIVE=1, UVALUE='STIMPOSTEVENT', $
					UNAME='wStimPostEventButton', /DYNAMIC_RESIZE)

				wStimPreEventButton = Widget_Button(wStimulusButton, FONT=my_font, $
					VALUE='Pre-Event Interval', SENSITIVE=1, UVALUE='STIMPREEVENT', $
					UNAME='wStimPreEventButton', /DYNAMIC_RESIZE)

				wStimRelativeCycleButton = Widget_Button(wStimulusButton, FONT=my_font, $
					VALUE='Relative Cycle Position', SENSITIVE=1, UVALUE='STIMRELATIVEPOSITION', $
					UNAME='wStimRelativeCycleButton', /DYNAMIC_RESIZE)

				wStimConvolutionButton = Widget_Button(wStimulusButton, FONT=my_font, $
					VALUE='Convolution with HRF', SENSITIVE=1, UVALUE='STIMCONVOLUTION', $
					UNAME='wStimConvolutionButton', /DYNAMIC_RESIZE)

				wStimSineButton = Widget_Button(wStimulusButton, FONT=my_font, $
					SENSITIVE=1, $
					VALUE='Sine', UVALUE='STIMSINE', $
					UNAME='wStimSineButton', /DYNAMIC_RESIZE)
		
				wStimCosineButton = Widget_Button(wStimulusButton, FONT=my_font, $
					VALUE='Cosine', SENSITIVE=1, UVALUE='STIMCOSINE', $
					UNAME='wStimCosineButton', /DYNAMIC_RESIZE)

				wStimLookupButton = Widget_Button(wStimulusButton, FONT=my_font, $
					VALUE='Lookup Value', SENSITIVE=1, UVALUE='STIMLOOKUP', $
					UNAME='wStimLookupButton', /DYNAMIC_RESIZE)

		wConstantButton = Widget_Button(wRegressionButton, VALUE='Include Constant', $
			UVALUE='CONSTANT', UNAME='wConstantButton' ,FONT=my_font, /DYNAMIC_RESIZE)

		wAutoRegressionButton = Widget_Button(wRegressionButton, FONT=my_font, $
			VALUE='First Order Autoregression Model', UVALUE='AUTOREGRESSION', $
			UNAME='wAutoRegressionButton', /DYNAMIC_RESIZE)

		wAnalysisMiddleBase = Widget_Base(wAnalysisBase, UNAME = 'wAnalysisMiddleBase' $
			,/ROW)

  		wWinLabel = Widget_Label(wAnalysisMiddleBase, UNAME='wWinLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Slding Window'+ $
      			' Width (sec): ' ,FONT=my_font)

  		wWinText = Widget_Text(wAnalysisMiddleBase, UNAME='wWinText'  $
      			,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS)

		wAnalysisBottomBase = Widget_Base(wAnalysisBase, UNAME = 'wAnalysisBottomBase' $
			,/ROW)

		wROIFileLabel = Widget_Label(wAnalysisBottomBase, UNAME='wROIFileLabel'  $
	      		,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Image to Select ROI On' $
			,FONT=my_font)

		wROIFileText = Widget_Text(wAnalysisBottomBase, UNAME='wROIFileText'  $
	      		,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS)

		wROILabel = Widget_Label(wAnalysisBottomBase, UNAME='wROILabel'  $
	      		,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Click Here to Select ROI' $
			,FONT=my_font)

  		wROIButton = Widget_Button(wAnalysisBottomBase, UNAME='wROIButton'  $
	     		,FRAME=1 ,/ALIGN_CENTER ,VALUE='ROI' ,FONT=my_font)

  	wOutputBase = Widget_Base(wBigRightBase, UNAME='wOutputBase'  $
      		,TITLE='IDL' ,SPACE=3 ,/COLUMN)

  		wOutputLabel = Widget_Label(wOutputBase, UNAME='wOutputLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Output' ,FONT=title_font)

		wOutputTopBase = Widget_Base(wOutputBase, UNAME = 'OutputTopBase' $
			,/ROW)

		outputListValues = [ 'Sum of Squares', 'Regression Coefficient', 'Effect Size', $
      			'F-statistic', 't-statistic' ]
  		wOutputList = Widget_Droplist(wOutputTopBase,  $
      			UNAME='wOutputList' ,TITLE='Output Image Type: '  $
      			,VALUE=outputListValues ,FONT=my_font, UVALUE=outputListValues)

		feedbackListValues = [ 'None', 'Image - Visual', 'Image - Auditory', $
      			'Stripchart' ]
  		wFeedbackList = Widget_Droplist(wOutputTopBase,  $
      			UNAME='wFeedbackList' ,TITLE='Subject Feedback: ', $
      			VALUE=feedbackListValues ,FONT=my_font, UVALUE=feedbackListValues)

		wOutputMiddleBase = Widget_Base(wOutputBase, UNAME = 'OutputMiddleBase' $
			,/ROW)

		stripchartListValues = [ 'Sum of Squares', $
	      		'Regression Coefficient', 'Effect Size', 'F-statistic', 't-statistic' ]
  		wStripchartList = Widget_Droplist(wOutputMiddleBase,  $
	      		UNAME='wStripchartList' ,TITLE='Stripchart Data Type: ' $
			,VALUE=stripchartListValues ,FONT=my_font, UVALUE=stripchartListValues)

		stimulusListValues =[ 'Yes', 'No' ]
  		wStimulusList = Widget_Droplist(wOutputMiddleBase,  $
	      		UNAME='wStimulusList' ,TITLE='View Stimulus: ' $
			,VALUE=stimulusListValues ,FONT=my_font, UVALUE=stimulusListValues)

		wOutputBottomBase = Widget_Base(wOutputBase, UNAME = 'OutputBottomBase' $
			,/ROW)

  		wInputFileLabel = Widget_Label(wOutputBottomBase, UNAME='wInputFileLabel'  $
 	     		,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Study Summary File'+ $
 	     		' Name: ' ,FONT=my_font)

		wInputFileText = Widget_Text(wOutputBottomBase, UNAME='wInputFileText'  $
	      		,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS)

  		wOutputFileLabel = Widget_Label(wOutputBottomBase, UNAME='wOutputFileLabel'  $
 	     		,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Output Data File Name: ' ,FONT=my_font)

		wOutputFileText = Widget_Text(wOutputBottomBase, UNAME='wOutputFileText'  $
	      		,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS)

		wOutputEndBase = Widget_Base(wOutputBase, UNAME = 'OutputEndBase' $
			,/ROW)

		outputFileListValues = [ 'Static', 'CVIO' ]
		wOutputFileList = Widget_Droplist(wOutputEndBase,  $
      			UNAME='wOutputFileList' ,TITLE='Output Data File Type: ' $
	      		,VALUE=outputFileListValues ,FONT=my_font, UVALUE=outputFileListValues)

		wSaveLabel = Widget_Label(wOutputEndBase, UNAME='wSaveLabel'  $
	      		,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Click Here to Save to Files' $
			,FONT=my_font)

  		wSaveButton = Widget_Button(wOutputEndBase, UNAME='wSaveButton'  $
	      		,FRAME=1 ,/ALIGN_CENTER ,VALUE='SAVE' ,FONT=my_font)

	wBigLeftBase = Widget_Base(wBigBase, UNAME = 'wBigLeftBase' $
		,/COLUMN)

  	wBasePictures = Widget_Base(wBigLeftBase, UNAME='wBasePictures'  $
      		,TITLE='IDL' ,SPACE=3 ,/COLUMN)

		wBaseTopPictures = Widget_Base(wBasePictures, UNAME='wBaseTopPictures' $
			,/ROW)

		wBaseTopOutputPictures= Widget_Base(wBaseTopPictures, $
			UNAME='wBaseTopOutputPictures' ,/COLUMN)

  		wDrawOutput = Widget_Draw(wBaseTopOutputPictures, UNAME='wDrawOutput'  $
	      		 ,GRAPHICS_LEVEL=1 ,SCR_XSIZE=256 ,SCR_YSIZE=256 )

		wDrawOutputLabel = Widget_Label(wBaseTopOutputPictures, UNAME='wDrawOutputLabel'  $
	      		,SENSITIVE=1 ,VALUE='Output Image' ,FONT=my_font)

		wBaseTopChartPictures= Widget_Base(wBaseTopPictures, $
			UNAME='wBaseTopChartPictures' ,/COLUMN)

  		wDrawChart = Widget_Draw(wBaseTopChartPictures, UNAME='wDrawChart' $
			,GRAPHICS_LEVEL=1 ,SCR_XSIZE=256 ,SCR_YSIZE=256 )

  		wDrawChartLabel = Widget_Label(wBaseTopChartPictures, UNAME='wDrawChartLabel'  $
	      		,SENSITIVE=1 ,VALUE='Strip Chart' ,FONT=my_font)

		wBaseBottomPictures = Widget_Base(wBasePictures, $
			UNAME='wBaseBottomPictures' ,/ROW)

		wBaseBottomStimPictures= Widget_Base(wBaseBottomPictures, $
			UNAME='wBaseBottomStimPictures' ,/COLUMN)

  		wDrawStimulus = Widget_Draw(wBaseBottomStimPictures, UNAME='wDrawStimulus'  $
	      		,GRAPHICS_LEVEL=1 ,SCR_XSIZE=256 ,SCR_YSIZE=256 )

		wDrawStimulusLabel = Widget_Label(wBaseBottomStimPictures,  $
			UNAME='wDrawStimulusLabel' ,SENSITIVE=1 ,VALUE='Stimulus' $
			,FONT=my_font)

		wBaseBottomOrigPictures= Widget_Base(wBaseBottomPictures, $
			UNAME='wBaseBottomOrigPictures' ,/COLUMN)

  		wDrawOriginal = Widget_Draw(wBaseBottomOrigPictures, UNAME='wDrawOriginal'  $
	      		,GRAPHICS_LEVEL=1 ,SCR_XSIZE=256 ,SCR_YSIZE=256)

  		wDrawOriginalLabel = Widget_Label(wBaseBottomOrigPictures, $
			UNAME='wDrawOriginalLabel' ,SENSITIVE=1 ,VALUE='Original Image' $
			,FONT=my_font)

 	wUpdateBase = Widget_Base(wBigLeftBase, UNAME='wUpdateBase'  $
      		,TITLE='IDL' ,SPACE=3 ,/COLUMN)

  		wUpdateText = Widget_Text(wUpdateBase, UNAME='wUpdateText' ,FONT=my_font $
			,SCR_XSIZE=512)

  	wExitBase = Widget_Base(wBigLeftBase, UNAME='wExitBase' $
      		,TITLE='IDL' ,SPACE=3 ,/COLUMN)

		wBaseNumLabel = Widget_Label(wExitBase, UNAME='wBaseNumLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Image Base Number: ' ,FONT=my_font)

  		wBaseNumText = Widget_Text(wExitBase, UNAME='wBaseNumText' $
 			,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS)

		wGoButton = Widget_Button(wExitBase, UNAME='wGoButton'  $
     			,FRAME=1 ,VALUE='GO' ,FONT=my_font)

  		wExitButton = Widget_Button(wExitBase, UNAME='wExitButton'  $
     			,FRAME=1 ,VALUE='EXIT' ,FONT=my_font)

	; Create a structure to hold information required by the program

  	sWidget = { orig_sizex : orig_sizex, $
  		    orig_sizey : orig_sizey, $
		    disp_sizex : disp_sizex, $
		    disp_sizey : disp_sizey, $
		    image_dir : image_dir, $
		    result_dir : result_dir, $
		    origImage_dir : origImage_dir, $
		    stimulus_dir : stimulus_dir, $
		    origImgArray : origImgArray, $
		    resultImgArray : resultImgArray, $
		    stimImgArray : stimImgArray, $
		    wDemoBase : wDemoBase, $
		    wPIDText : wPIDText, $
		    wRunNumText : wRunNumText, $
		    wDateText : wDateText, $
		    wRunLabText : wRunLabText, $
		    wDesignBase : wDesignBase, $
		    wStimTypeList : wStimTypeList, $
		    wStimLiList : wStimLiList, $
		    wISIText : wISIText, $
		    wJitterList : wJitterList, $
		    wDurationText : wDurationText, $
		    wCollectionBase : wCollectionBase, $
		    wSourceList : wSourceList, $
		    wNumImgText : wNumImgText, $
		    wNumSliText : wNumSliText, $
		    wRateText : wRateText, $
		    wCVIOCardiacButton : wCVIOCardiacButton, $
		    wCVIORespiratoryButton : wCVIORespiratoryButton, $
		    wCVIOImagAcqButton : wCVIOImagAcqButton, $
		    wCVIOStimulusButton : wCVIOStimulusButton, $
		    wRegressionButton : wRegressionButton, $
		    wDependentButton : wDependentButton, $
		    wImageTimeSeriesButton : wImageTimeSeriesButton, $
		    wCollectedCardiacButton : wCollectedCardiacButton, $
		    wCollectedRespiratoryButton : wCollectedRespiratoryButton, $
		    wCollectedImgAcqButton : wCollectedImgAcqButton, $
		    wCollectedStimulusButton : wCollectedStimulusButton, $
		    wIndependentButton : wIndependentButton, $
		    wCardiacButton : wCardiacButton, $
		    wCardNoneButton : wCardNoneButton, $
		    wCardPostEventButton : wCardPostEventButton, $
		    wCardPreEventButton : wCardPreEventButton, $
		    wCardRelativeCycleButton : wCardRelativeCycleButton, $
		    wCardConvolutionButton : wCardConvolutionButton, $
		    wCardSineButton : wCardSineButton, $
		    wCardCosineButton : wCardCosineButton, $
		    wCardLookupButton : wCardLookupButton, $
		    wRespiratoryButton : wRespiratoryButton, $
		    wRespNoneButton : wRespNoneButton, $
		    wRespPostEventButton : wRespPostEventButton, $
		    wRespPreEventButton : wRespPreEventButton, $
		    wRespRelativeCycleButton : wRespRelativeCycleButton, $
		    wRespConvolutionButton : wRespConvolutionButton, $
		    wRespSineButton : wRespSineButton, $
		    wRespCosineButton : wRespCosineButton, $
		    wRespLookupButton : wRespLookupButton, $
		    wImagAcqButton : wImagAcqButton, $
		    wImgNoneButton : wImgNoneButton, $
		    wImgPostEventButton : wImgPostEventButton, $
		    wImgPreEventButton : wImgPreEventButton, $
		    wImgRelativeCycleButton : wImgRelativeCycleButton, $
		    wImgConvolutionButton : wImgConvolutionButton, $
		    wImgSineButton : wImgSineButton, $
		    wImgCosineButton : wImgCosineButton, $
		    wImgLookupButton : wImgLookupButton, $
		    wStimulusButton : wStimulusButton, $
		    wStimNoneButton : wStimNoneButton, $
		    wStimPostEventButton : wStimPostEventButton, $
		    wStimPreEventButton : wStimPreEventButton, $
		    wStimRelativeCycleButton : wStimRelativeCycleButton, $
		    wStimConvolutionButton : wStimConvolutionButton, $
		    wStimSineButton : wStimSineButton, $
		    wStimCosineButton : wStimCosineButton, $
		    wStimLookupButton : wStimLookupButton, $
		    wConstantButton : wConstantButton, $
		    wAutoRegressionButton : wAutoRegressionButton, $
		    wOrderList : wOrderList, $
		    wAnalysisBase : wAnalysisBase, $
		    wWinText : wWinText, $
		    wROIButton : wROIButton, $
		    wROIFileText : wROIFileText, $
		    wOutputBase : wOutputBase, $
		    wOutputList : wOutputList, $
		    wFeedbackList : wFeedbackList, $
		    wStripchartList : wStripchartList, $
		    wStimulusList : wStimulusList, $
		    wInputFileText : wInputFileText, $
		    wOutputFileList : wOutputFileList, $
		    wOutputFileText : wOutputFileText, $
		    wSaveButton : wSaveButton, $
		    wBasePictures : wBasePictures, $
		    wDrawOutput : wDrawOutput, $
		    wDrawChart : wDrawChart, $
		    wDrawStimulus : wDrawStimulus, $
		    wDrawOriginal : wDrawOriginal, $
		    wUpdateBase : wUpdateBase, $
		    wUpdateText : wUpdateText, $
		    wExitBase : wExitBase, $
		    wGoButton : wGoButton, $
		    wExitButton : wExitButton, $
		    orig_win_id : -1, $
		    stim_win_id : -1, $
		    result_win_id : -1, $
		    chart_win_id : -1, $
		    PID : '', $
		    RunNum : '', $
		    filename1 : '', $
		    filename2 : '', $
		    filename3 : '', $
		    filename4 : '', $
		    filename5 : '', $
		    Date : '', $
		    RunLabel : '', $
		    StimType : '', $
		    StimList : '', $
		    ISI : -1, $
		    Jitter : '', $
		    Duration : -1, $
		    Rate : -1, $
		    Source : '', $
		    NumImg : -1, $
		    NumSli : -1, $
		    Order : '', $
		    Channels : 0, $
		    Acquire_Channels : [-1,-1,-1,-1], $
		    Acquire_Files : ['','','',''], $
		    Model : '', $
		    Dependent : '', $
		    Collected : '', $
		    Independent : '', $
		    Transform : '', $
		    ROIFile : '', $
		    ROI : [-1,-1,-1,-1], $
		    OutputType : '', $
		    SlidingWindow : -1, $
		    FeedbackType : '', $
		    StripchartType : '', $
	 	    StimStatus : '', $
		    InputFile: '', $
		    OutputFileType : '', $
		    OutputFile : '', $
		    RegressionReady : '', $
		    StimulusReady : '', $
		    CardiacCheck : -1, $
		    RespiratoryCheck : -1, $
		    ImagAcqCheck : -1, $
		    StimulusCheck : -1, $
		    BaseNum : -1 }

  ; Store the structure in the user value of the top-level base
  Widget_Control, wBigBase, SET_UVALUE=sWidget
  Widget_Control, wBigBase, MAP=1, /REALIZE
  Widget_Control, wDemoBase, Sensitive=1
  Widget_Control, wDesignBase, Sensitive=1
  Widget_Control, wCollectionBase, Sensitive=1
  Widget_Control, wAnalysisBase, Sensitive=1
  Widget_Control, WOutputBase, Sensitive=1
  Widget_Control, wUpdateText, SET_VALUE = 'Please enter all necessary' + $
  	' information in specified areas'
  interfacePrep, wBigBase
  XManager, 'NAME', wBigBase, EVENT_HANDLER='interfaceHandler', /No_Block

end
