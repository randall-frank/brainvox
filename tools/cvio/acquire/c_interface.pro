;----------------------------------------------------------------------
;
;	PURPOSE: event handler
;
;----------------------------------------------------------------------
pro interfaceHandler, sEvent

  if (TAG_NAMES(sEvent, /STRUCTURE_NAME) eq 'WIDGET_KILL_REQUEST') then begin
  	WIDGET_CONTROL, sEvent.top, /DESTROY
  	RETURN
  endif

  WIDGET_CONTROL, sEvent.top, GET_UVALUE = sWidget

  case sEvent.id of

	sWidget.wPIDText : begin
		; Get the patient ID from the user
		WIDGET_CONTROL, sWidget.wPIDText, GET_VALUE=temp
		sWidget.PID = temp
		; Update the user on status
		WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Patient ID = '+ sWidget.PID
	END

	sWidget.wRunNumText : begin
		; If want to perform another real-time regression after already doing so, 
		; reset the collection arrays
		if ((sWidget.Already_Regressed eq 1) and (sWidget.Source eq 0)) then begin
			sWidget.Acquire_Files(*)=''
			sWidget.Acquire_Channels(*)=-1
		endif
		; Get the run number from the user
		WIDGET_CONTROL, sWidget.wRunNumText, GET_VALUE=temp
		sWidget.RunNum = temp
		if (strlen(sWidget.RunNum) eq 1) then begin
			sWidget.RunNum='0'+temp
		endif
		; Determine filenames for collected data using run number
		sWidget.file1 = 'fmri'+sWidget.RunNum+'_image_'+'raw.cvio'
		sWidget.file2 = 'fmri'+sWidget.RunNum+'_card_'+'raw.cvio'
		sWidget.file3 = 'fmri'+sWidget.RunNum+'_resp_'+'raw.cvio'
		sWidget.file4 = 'fmri'+sWidget.RunNum+'_stim_'+'raw.cvio'
		sWidget.file5 = 'fmri'+sWidget.RunNum+'_tag_'+'raw.cvio'
		sWidget.file6 = 'fmri'+sWidget.RunNum+'_Rwavesine.cvio'
		sWidget.file7 = 'fmri'+sWidget.RunNum+'_Rwavecosine.cvio'
		sWidget.file8 = 'fmri'+sWidget.RunNum+'_acqorder.cvio'
		sWidget.file9 = 'fmri'+sWidget.RunNum+'_resp.cvio'
		sWidget.file10 = 'fmri'+sWidget.RunNum+'_global.cvio'
		; Reset the names of the input/output files to reflect the change in run number
		sWidget.InputFile = 'fmri'+sWidget.RunNum+'_summary.cvio'
		if ((sWidget.ResultTypeOutput ge 5) AND (sWidget.ResultTypeOutput le 7)) then begin
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_ss_raw.cvio'
		endif else if ((sWidget.ResultTypeOutput ge 8) $
		    AND (sWidget.ResultTypeOutput le 10)) then begin
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_coef_raw.cvio'
		endif else if ((sWidget.ResultTypeOutput ge 11) $
		    AND (sWidget.ResultTypeOutput le 13)) then begin
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_effect_raw.cvio'
		endif else if ((sWidget.ResultTypeOutput ge 14) $
		    AND (sWidget.ResultTypeOutput le 16)) then begin
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_tstat_raw.cvio'
		endif else if ((sWidget.ResultTypeOutput ge 17) $
		    AND (sWidget.ResultTypeOutput le 19)) then begin
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_fstat_raw.cvio'
		endif else if (sWidget.ResultTypeOutput eq 21) then begin
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_lagsincos_raw.cvio'
		endif else if (sWidget.ResultTypeOutput eq 22) then begin
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_sincosfstat_raw.cvio'
		endif else begin
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_magsincos_raw.cvio'
		endelse
		; If a physiological collection button selected, re-activate/name appropriate 
		; regression button
		IF (sWidget.CardiacCheck EQ 1) THEN BEGIN
			WIDGET_CONTROL, sWidget.wCardiacButton, SET_VALUE=sWidget.file2
			WIDGET_CONTROL, sWidget.wCardiacButton, SENSITIVE=1
			; Activate tagged images regression button
			WIDGET_CONTROL, sWidget.wTaggedImgButton, SET_VALUE=sWidget.file5
			WIDGET_CONTROL, sWidget.wTaggedImgButton, SENSITIVE=1
			sWidget.Acquire_Channels[1] = 1
			sWidget.Acquire_Files[1] = sWidget.file2			
		ENDIF
		IF (sWidget.RespiratoryCheck EQ 1) THEN BEGIN
			WIDGET_CONTROL, sWidget.wRespiratoryButton, SET_VALUE=sWidget.file3
			WIDGET_CONTROL, sWidget.wRespiratoryButton, SENSITIVE=1
			; Activate tagged images regression button
			WIDGET_CONTROL, sWidget.wTaggedImgButton, SET_VALUE=sWidget.file5
			WIDGET_CONTROL, sWidget.wTaggedImgButton, SENSITIVE=1	
			sWidget.Acquire_Channels[2] = 2
			sWidget.Acquire_Files[2] = sWidget.file3		
		ENDIF
		IF (sWidget.StimulusCheck EQ 1) THEN BEGIN
			WIDGET_CONTROL, sWidget.wStimulusButton, SET_VALUE=sWidget.file4
			WIDGET_CONTROL, sWidget.wStimulusButton, SENSITIVE=1
			; Activate tagged images regression button
			WIDGET_CONTROL, sWidget.wTaggedImgButton, SET_VALUE=sWidget.file5
			WIDGET_CONTROL, sWidget.wTaggedImgButton, SENSITIVE=1
			sWidget.Acquire_Channels[3] = 3
			sWidget.Acquire_Files[3] = sWidget.file4
		ENDIF
		if (sWidget.ImagAcqCheck eq 1) then begin
			sWidget.Acquire_Channels[0] = 0
			sWidget.Acquire_Files[0] = sWidget.file1
		endif
		; Verify that a run number has been entered for future purposes
		sWidget.Number_Entered = 1
		; Update the user on status
		WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Run Number = '+ sWidget.RunNum
	END

	sWidget.wDateText: begin
		; Get study date from user
		WIDGET_CONTROL, sWidget.wDateText, GET_VALUE=temp
		sWidget.Date = temp
		; Update the user on status
		WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Date = '+ sWidget.Date
	END

	sWidget.wRunLabText: begin
		; Get run label from user
		WIDGET_CONTROL, sWidget.wRunLabText, GET_VALUE=temp
		sWidget.RunLabel = temp
		; Update the user on status
		WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Run Label = '+ sWidget.RunLabel
	END

	sWidget.wStimTypeList: begin
		; Get type of stimulus from user, either block or event related
		currentStimTypeList = WIDGET_INFO(sWidget.wStimTypeList, /DropList_Select)
		WIDGET_CONTROL, sWidget.wStimTypeList, GET_UVALUE=stimTypeListValues
		sWidget.StimType = stimTypeListValues[currentStimTypeList]
		; If block design category selected, make stimulus type choices available
		IF (sWidget.StimType EQ 'block design') THEN BEGIN
			WIDGET_CONTROL, sWidget.wStimLiButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wStimVisualButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wStimNamingButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wPolarButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wNewPolar30Button, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wNewPolar20Button, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wEccentricButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wNewEccentric30Button, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wNewEccentric20Button, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wCombinedButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wNewCombinedr30e20Button, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wNewCombinedr20e30Button, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wNewCombinedr20e302plus8Button, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wTomSpecialButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.w2HzHemiButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.w4HzHemiButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.w8HzHemiButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wFeedbackStimButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wEccFeedbackStimButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wRandomButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wBlockButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wThreeButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wFourButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wStimWordButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wStemsButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wLettersButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wWords1Button, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wWords2Button, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wWords3Button, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wWords4Button, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wBulbButton, SENSITIVE=1
			sWidget.block_click=1
			; Make event related stim unavailable
			if (sWidget.event_click EQ 1) THEN BEGIN
				WIDGET_CONTROL, sWidget.wISIText, SET_VALUE=''
				WIDGET_CONTROL, sWidget.wJitterList, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wISIText, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wISILabel, SENSITIVE=0
			ENDIF
		ENDIF
		; If event related category selected, make stim type choices available
		IF (sWidget.StimType EQ 'event related') THEN BEGIN
			WIDGET_CONTROL, sWidget.wJitterList, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wISIText, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wISILabel, SENSITIVE=1
			sWidget.event_click=1
			; Make block design stim unavailable
			IF (sWidget.block_click EQ 1) THEN BEGIN
				WIDGET_CONTROL, sWidget.wStimLiButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wStimVisualButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wStimNamingButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wPolarButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wNewPolar30Button, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wNewPolar20Button, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wEccentricButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wNewEccentric30Button, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wNewEccentric20Button, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wCombinedButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wNewCombinedr30e20Button, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wNewCombinedr20e30Button, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wNewCombinedr20e302plus8Button, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wTomSpecialButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wTomSpecialButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.w2HzHemiButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.w4HzHemiButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.w8HzHemiButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wFeedbackStimButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wEccFeedbackStimButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wRandomButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wBlockButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wThreeButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wFourButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wStimWordButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wStemsButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wLettersButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wWords1Button, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wWords2Button, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wWords3Button, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wWords4Button, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wBulbButton, SENSITIVE=0
				resetStimChoice, sWidget
			ENDIF
		ENDIF
	END

	sWidget.wPolarButton : begin
		; Prepare application to use rotating stimulus, change appropriate filename and
		; variable values. Make interface reflect selection.
		sWidget.StimSelect = 1
		sWidget.StimulusName = 'rotating.yuv'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wPolarButton, SET_VALUE = '*Old Polar*'
		sWidget.StimSizex = 480
		sWidget.StimSizey = 480
	END

	sWidget.wNewPolar30Button : begin
		; Prepare application to use rotating stimulus, change appropriate filename and
		; variable values. Make interface reflect selection.
		sWidget.StimSelect = 1
		sWidget.StimulusName = 'new_rad_30.yuv'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wNewPolar30Button, SET_VALUE = '*New Polar 30*'
		sWidget.StimSizex = 480
		sWidget.StimSizey = 480
	END

	sWidget.wNewPolar20Button : begin
		; Prepare application to use rotating stimulus, change appropriate filename and
		; variable values. Make interface reflect selection.
		sWidget.StimSelect = 1
		sWidget.StimulusName = 'new_rad_20.yuv'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wNewPolar20Button, SET_VALUE = '*New Polar 20*'
		sWidget.StimSizex = 480
		sWidget.StimSizey = 480
	END
	
	sWidget.wEccentricButton : begin
		; Prepare application to use expanding stimulus, change appropriate filename and
		; variable values. Make interface reflect selection.
		sWidget.StimSelect = 1
		sWidget.StimulusName = 'expanding.yuv'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wEccentricButton, SET_VALUE = '*Old Ecc*'
		sWidget.StimSizex = 480
		sWidget.StimSizey = 480
	END
	
	sWidget.wNewEccentric30Button : begin
		; Prepare application to use expanding stimulus, change appropriate filename and
		; variable values. Make interface reflect selection.
		sWidget.StimSelect = 1
		sWidget.StimulusName = 'new_ecc_30.yuv'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wNewEccentric30Button, SET_VALUE = '*New Ecc 30*'
		sWidget.StimSizex = 480
		sWidget.StimSizey = 480
	END
	
	sWidget.wNewEccentric20Button : begin
		; Prepare application to use expanding stimulus, change appropriate filename and
		; variable values. Make interface reflect selection.
		sWidget.StimSelect = 1
		sWidget.StimulusName = 'new_ecc_20.yuv'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wNewEccentric20Button, SET_VALUE = '*New Ecc 20*'
		sWidget.StimSizex = 480
		sWidget.StimSizey = 480
	END

	sWidget.wCombinedButton : begin
		; Prepare application to use combined stimulus, change appropriate filename and
		; variable values. Make interface reflect selection.
		sWidget.StimSelect = 1
		sWidget.StimulusName = 'combined.yuv'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wCombinedButton, SET_VALUE='*Old Combined*'
		sWidget.StimSizex = 480
		sWidget.StimSizey = 480
	END

	sWidget.wNewCombinedr30e20Button : begin
		; Prepare application to use combined stimulus, change appropriate filename and
		; variable values. Make interface reflect selection.
		sWidget.StimSelect = 1
		sWidget.StimulusName = 'new_combo_r30e20.yuv'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wNewCombinedr30e20Button, SET_VALUE='*Combined r30 e20 - 4 plus 8*'
		sWidget.StimSizex = 480
		sWidget.StimSizey = 480
	END

	sWidget.wNewCombinedr20e30Button : begin
		; Prepare application to use combined stimulus, change appropriate filename and
		; variable values. Make interface reflect selection.
		sWidget.StimSelect = 1
		sWidget.StimulusName = 'new_combo_r20e30.yuv'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wNewCombinedr20e30Button, SET_VALUE='*Combined r20 e30 - 4 plus 8*'
		sWidget.StimSizex = 480
		sWidget.StimSizey = 480
	END

	sWidget.wNewCombinedr20e302plus8Button : begin
		; Prepare application to use combined stimulus, change appropriate filename and
		; variable values. Make interface reflect selection.
		sWidget.StimSelect = 1
		sWidget.StimulusName = 'new_combo_r20e302plus8.yuv'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wNewCombinedr20e302plus8Button, SET_VALUE='*Combined r20 e30 - 2 plus 8*'
		sWidget.StimSizex = 480
		sWidget.StimSizey = 480
	END

	sWidget.wTomSpecialButton : begin
		; Prepare application to use combined stimulus, change appropriate filename and
		; variable values. Make interface reflect selection.
		sWidget.StimSelect = 5
		sWidget.StimulusName = 'tomspecial.yuv'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wTomSpecialButton, SET_VALUE='*Tom Special*'
		sWidget.StimSizex = 480
		sWidget.StimSizey = 480
	END

	sWidget.w2HzHemiButton : begin
		; Prepare application to use combined stimulus, change appropriate filename and
		; variable values. Make interface reflect selection.
		sWidget.StimSelect = 1
		sWidget.StimulusName = 'right_hemi_2.yuv'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.w2HzHemiButton, SET_VALUE='*2 Hz Rt. Hemifield*'
		sWidget.StimSizex = 480
		sWidget.StimSizey = 480
	END

	sWidget.w4HzHemiButton : begin
		; Prepare application to use combined stimulus, change appropriate filename and
		; variable values. Make interface reflect selection.
		sWidget.StimSelect = 1
		sWidget.StimulusName = 'right_hemi_4.yuv'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.w4HzHemiButton, SET_VALUE='*4 Hz Rt. Hemifield*'
		sWidget.StimSizex = 480
		sWidget.StimSizey = 480
	END

	sWidget.w8HzHemiButton : begin
		; Prepare application to use combined stimulus, change appropriate filename and
		; variable values. Make interface reflect selection.
		sWidget.StimSelect = 1
		sWidget.StimulusName = 'right_hemi_8.yuv'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.w8HzHemiButton, SET_VALUE='*8 Hz Rt. Hemifield*'
		sWidget.StimSizex = 480
		sWidget.StimSizey = 480
	END

	sWidget.wFeedbackStimButton : begin
		; Prepare application to use combined stimulus, change appropriate filename and
		; variable values. Make interface reflect selection.
		sWidget.StimSelect = 2
		sWidget.StimulusName = 'stim_feedback.yuv'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wFeedbackStimButton, SET_VALUE='*Feedback*'
		sWidget.StimSizex = 480
		sWidget.StimSizey = 480
	END

	sWidget.wEccFeedbackStimButton : begin
		; Prepare application to use combined stimulus, change appropriate filename and
		; variable values. Make interface reflect selection.
		sWidget.StimSelect = 1
		sWidget.StimulusName = 'stim_ecc_feedback.yuv'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wEccFeedbackStimButton, SET_VALUE='*Ecc Feedback*'
		sWidget.StimSizex = 480
		sWidget.StimSizey = 480
	END

	sWidget.wRandomButton : begin
		; Prepare application to use combined stimulus, change appropriate filename and
		; variable values. Make interface reflect selection.
		sWidget.StimSelect = 10
		sWidget.StimulusName = 'random_stim2'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wRandomButton, SET_VALUE='*Random*'
		sWidget.StimSizex = 480
		sWidget.StimSizey = 480
		WIDGET_CONTROL, sWidget.wRandomText, SENSITIVE=1
		WIDGET_CONTROL, sWidget.wRandomLabel, SENSITIVE=1
	END

	sWidget.wBlockButton : begin
		; Prepare application to use block visual naming stimulus, change appropriate 
		; filename and variable values. Make interface reflect selection.
		sWidget.StimSelect = 3
		sWidget.StimulusName = 'call_block'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wBlockButton, SET_VALUE = '*FTN (block)*'
		sWidget.StimSizex = 720
		sWidget.StimSizey = 486
	END

	sWidget.wThreeButton : begin
		; Prepare application to use three type visual naming stimulus, change appropriate 
		; filename and variable values. Make interface reflect selection.
		sWidget.StimSelect = 3
		sWidget.StimulusName = 'call_three'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wThreeButton, SET_VALUE = '*FTN (random)*'
		sWidget.StimSizex = 720
		sWidget.StimSizey = 486
	END

	sWidget.wFourButton : begin
		; Prepare application to use four type visual naming stimulus, change appropriate 
		; filename and variable values. Make interface reflect selection.
		sWidget.StimSelect = 3
		sWidget.StimulusName = 'call_four'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wFourButton, SET_VALUE = '*FTAN (random)*'
		sWidget.StimSizex = 720
		sWidget.StimSizey = 486
	END

	sWidget.wStemsButton : begin
		; Prepare application to use four type visual naming stimulus, change appropriate 
		; filename and variable values. Make interface reflect selection.
		sWidget.StimSelect = 4
		sWidget.StimulusName = 'stems/call_stems'
		sWidget.StimulusName2 = 'stems/'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wStemsButton, SET_VALUE = '*Stems*'
		sWidget.StimSizex = 720
		sWidget.StimSizey = 486
	END

	sWidget.wLettersButton : begin
		; Prepare application to use four type visual naming stimulus, change appropriate 
		; filename and variable values. Make interface reflect selection.
		sWidget.StimSelect = 4
		sWidget.StimulusName = 'letters/call_letters'
		sWidget.StimulusName2 = 'letters/'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wLettersButton, SET_VALUE = '*Letters*'
		sWidget.StimSizex = 720
		sWidget.StimSizey = 486
	END

	sWidget.wWords1Button : begin
		; Prepare application to use four type visual naming stimulus, change appropriate 
		; filename and variable values. Make interface reflect selection.
		sWidget.StimSelect = 4
		sWidget.StimulusName = 'words1/call_words'
		sWidget.StimulusName2 = 'words1/'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wWords1Button, SET_VALUE = '*Words 1*'
		sWidget.StimSizex = 720
		sWidget.StimSizey = 486
	END

	sWidget.wWords2Button : begin
		; Prepare application to use four type visual naming stimulus, change appropriate 
		; filename and variable values. Make interface reflect selection.
		sWidget.StimSelect = 4
		sWidget.StimulusName = 'words2/call_words'
		sWidget.StimulusName2 = 'words2/'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wWords2Button, SET_VALUE = '*Words 2*'
		sWidget.StimSizex = 720
		sWidget.StimSizey = 486
	END

	sWidget.wWords3Button : begin
		; Prepare application to use four type visual naming stimulus, change appropriate 
		; filename and variable values. Make interface reflect selection.
		sWidget.StimSelect = 4
		sWidget.StimulusName = 'words3/call_words'
		sWidget.StimulusName2 = 'words3/'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wWords3Button, SET_VALUE = '*Words 3*'
		sWidget.StimSizex = 720
		sWidget.StimSizey = 486
	END

	sWidget.wWords4Button : begin
		; Prepare application to use four type visual naming stimulus, change appropriate 
		; filename and variable values. Make interface reflect selection.
		sWidget.StimSelect = 4
		sWidget.StimulusName = 'words4/call_words'
		sWidget.StimulusName2 = 'words4/'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wWords4Button, SET_VALUE = '*Words 4*'
		sWidget.StimSizex = 720
		sWidget.StimSizey = 486
	END

	sWidget.wBulbButton : begin
		; Prepare application to use four type visual naming stimulus, change appropriate 
		; filename and variable values. Make interface reflect selection.
		sWidget.StimSelect = 4
		sWidget.StimulusName = 'bulb/call_bulb'
		sWidget.StimulusName2 = 'bulb/'
		resetStimChoice, sWidget
		WIDGET_CONTROL, sWidget.wBulbButton, SET_VALUE = '*Bulb*'
		sWidget.StimSizex = 720
		sWidget.StimSizey = 486
	END

	sWidget.wISIText: begin
		; Get ISI from user if necessary
		WIDGET_CONTROL, sWidget.wISIText, GET_VALUE=temp
		sWidget.ISI = temp
		; Update the user on status
		WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'ISI Successfully Entered '
	END

	sWidget.wRandomText: begin
		; Get random CVIO file from user if necessary
		WIDGET_CONTROL, sWidget.wRandomText, GET_VALUE=temp
		sWidget.Random_File = temp
		; Update the user on status
		WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'CVIO File Successfully Entered '
	END

	sWidget.wJitterList: begin
		; Get whether stimulus jittered or not from user if necessary
		currentJitterList = WIDGET_INFO(sWidget.wJitterList, /DropList_Select)
		WIDGET_CONTROL, sWidget.wJitterList, GET_UVALUE=jitterListValues
		sWidget.Jitter = jitterListValues[currentJitterList]
	END

	sWidget.wDurationText: begin
		; Get duration of experiment from user if necessary
		WIDGET_CONTROL, sWidget.wDurationText, GET_VALUE=temp
		sWidget.Duration = temp
		; Update the user on status
		WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Duration Successfully Entered'
	END

	sWidget.wPeriodText: begin
		; Get period of stimulus from user if necessary
		WIDGET_CONTROL, sWidget.wPeriodText, GET_VALUE=temp
		sWidget.Period = temp
		; Update the user on status
		WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Period Successfully Entered'
	END

	sWidget.wSourceList: begin
		; Get whether data will be coming from shared memory (real-time) or disk
		; Store data source in variable for future purposes
		currentSourceList = WIDGET_INFO(sWidget.wSourceList, /DropList_Select)
		WIDGET_CONTROL, sWidget.wSourceList, GET_UVALUE=sourceListValues
		temp = sourceListValues[currentSourceList]
		; Set appropriate program variables to reflect change
		if (temp EQ 'Shared Memory') then begin
			sWidget.Source = 0
		endif
		if (temp EQ 'Disk') then begin
			sWidget.Source = 1
		endif
	END

	sWidget.wNumImgText: begin
		; Get number of images from user
		WIDGET_CONTROL, sWidget.wNumImgText, GET_VALUE=temp
		sWidget.NumImg = temp
		; Update the user on status
		WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Number of Images = '+STRING(sWidget.NumImg)
	END

	sWidget.wNumSliText: begin
		; Get number of slices from user 
		WIDGET_CONTROL, sWidget.wNumSliText, GET_VALUE=temp
		sWidget.NumSli = temp
		; Update the user on status
		WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Number of Slices = '+STRING(sWidget.NumSli)
	END
	
	sWidget.wRateText : begin
		; Get collection rate from user - it applies to all channels
		WIDGET_CONTROL, sWidget.wRateText, GET_VALUE=temp
		sWidget.Rate = temp
		; Update the user on status
		WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Collection Rate = '+STRING(sWidget.Rate)
	END

	sWidget.wCVIOCardiacButton : begin
		; If user hasn't entered run number then can't provide regression file options
		IF (sWidget.Number_Entered EQ -1)  then begin
			; Update the user on status
			WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Please Enter Run Number First'
		endif
		; User has selcted to collect cardiac signal, provide regression/collection options
		; Change appropriate values for acquisition, make interface reflect the change
		IF sWidget.CardiacCheck NE 1 THEN BEGIN
			WIDGET_CONTROL, sWidget.wCVIOCardiacButton, SET_VALUE='*Cardiac*'
			sWidget.Channels = sWidget.Channels + 1
			WIDGET_CONTROL, sWidget.wCardiacButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wCardiacButton, SET_VALUE=sWidget.file2
			WIDGET_CONTROL, sWidget.wTaggedImgButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wTaggedImgButton, SET_VALUE=sWidget.file5
			sWidget.Acquire_Channels[1] = 1
			sWidget.Acquire_Files[1] = sWidget.file2
			sWidget.CardiacCheck = 1
		; User has selected to NOT collect cardiac signal, remove collection/regression options
		; Change appropriate values for acquisition, make interface reflect the change
		ENDIF ELSE BEGIN
			WIDGET_CONTROL, sWidget.wCVIOCardiacButton, SET_VALUE='Cardiac'
			sWidget.Channels = sWidget.Channels - 1
			; If not collecting any other channel, can't access images for variable
			if ((sWidget.RespiratoryCheck eq -1) AND (sWidget.StimulusCheck eq -1) and $
			    (sWidget.ImagAcqCheck eq -1)) then begin
				WIDGET_CONTROL, sWidget.wTaggedImgButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wTaggedImgButton, SET_VALUE='Tagged Img Data'
			endif
			WIDGET_CONTROL, sWidget.wCardiacButton, SET_VALUE='Cardiac'
			WIDGET_CONTROL, sWidget.wCardiacButton, SENSITIVE=0
			sWidget.Acquire_Channels[1] = -1
			sWidget.Acquire_Files[1] = ''
			sWidget.CardiacCheck = -1
		ENDELSE
	END

	sWidget.wCVIORespiratoryButton : begin
		; If user hasn't entered run number then can't provide regression file options
		IF (sWidget.Number_Entered EQ -1)  then begin
			; Update the user on status
			WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Please Enter Run Number First'
		endif
		; User has selcted to collect respiratory signal, provide regression/collection options
		; Change appropriate values for acquisition, make interface reflect the change
		IF sWidget.RespiratoryCheck EQ -1 THEN BEGIN
			WIDGET_CONTROL, sWidget.wCVIORespiratoryButton, SET_VALUE='*Respiratory*'
			sWidget.Channels = sWidget.Channels + 1
			WIDGET_CONTROL, sWidget.wRespiratoryButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wRespiratoryButton, SET_VALUE=sWidget.file3
			WIDGET_CONTROL, sWidget.wTaggedImgButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wTaggedImgButton, SET_VALUE=sWidget.file5
			sWidget.Acquire_Channels[2] = 2
			sWidget.Acquire_Files[2] = sWidget.file3			
			sWidget.RespiratoryCheck = 1
		; User has selected to NOT collect respiratory signal, remove collection/regression
		; options.  Change appropriate values for acquisition, make interface reflect the change
		ENDIF ELSE BEGIN
			WIDGET_CONTROL, sWidget.wCVIORespiratoryButton, SET_VALUE='Respiratory'
			sWidget.Channels = sWidget.Channels - 1
			; If not collecting any other channel, can't access images for variable
			if ((sWidget.CardiacCheck eq -1) AND (sWidget.StimulusCheck eq -1) and $
			    (sWidget.ImagAcqCheck eq -1)) then begin
				WIDGET_CONTROL, sWidget.wTaggedImgButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wTaggedImgButton, SET_VALUE='Tagged Img Data'
			endif
			WIDGET_CONTROL, sWidget.wRespiratoryButton, SET_VALUE='Respiratory'
			WIDGET_CONTROL, sWidget.wRespiratoryButton, SENSITIVE=0
			sWidget.Acquire_Channels[2] = -1
			sWidget.Acquire_Files[2] = ''
			sWidget.RespiratoryCheck = -1
		ENDELSE
	END

	sWidget.wCVIOImagAcqButton : begin
		; If user hasn't entered run number then can't provide regression file options
		IF (sWidget.Number_Entered EQ -1) then begin
			; Update the user on status
			WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Please Enter Run Number First'
		endif
		; User has selcted to collect image acq signal, provide regression/collection options
		; Change appropriate values for acquisition, make interface reflect the change
		IF (sWidget.ImagAcqCheck EQ -1) THEN BEGIN
			WIDGET_CONTROL, sWidget.wCVIOImagAcqButton, SET_VALUE='*Image Acquisition*'
			sWidget.Channels = sWidget.Channels + 1
			WIDGET_CONTROL, sWidget.wTaggedImgButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wTaggedImgButton, SET_VALUE=sWidget.file5
			sWidget.Acquire_Channels[0] = 0
			sWidget.Acquire_Files[0] = sWidget.file1
			sWidget.ImagAcqCheck = 1
		; User has selected to NOT collect image acq signal, remove collection/regression options
		; Change appropriate values for acquisition, make interface reflect the change
		ENDIF ELSE BEGIN
			WIDGET_CONTROL, sWidget.wCVIOImagAcqButton, SET_VALUE='Image Acquisition'
			sWidget.Channels = sWidget.Channels - 1
			; If not collecting any other channel, can't access images for variable
			if ((sWidget.RespiratoryCheck eq -1) AND (sWidget.StimulusCheck eq -1) and $
			    (sWidget.CardiacCheck eq -1)) then begin
				WIDGET_CONTROL, sWidget.wTaggedImgButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wTaggedImgButton, SET_VALUE='Tagged Img Data'
			endif
			sWidget.Acquire_Channels[0] = -1
			sWidget.Acquire_Files[0] = ''
			sWidget.ImagAcqCheck = -1
		ENDELSE
	END

	sWidget.wCVIOStimulusButton : begin
		; If user hasn't entered run number then can't provide regression file options
		IF (sWidget.Number_Entered EQ -1) then begin
			; Update the user on status
			WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Please Enter Run Number First'
		endif
		; User has selcted to collect stimulus signal, provide regression/collection options
		; Change appropriate values for acquisition, make interface reflect the change	
		IF (sWidget.StimulusCheck EQ -1) THEN BEGIN
			WIDGET_CONTROL, sWidget.wCVIOStimulusButton, SET_VALUE='*Stimulus*'
			sWidget.Channels = sWidget.Channels + 1
			WIDGET_CONTROL, sWidget.wStimulusButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wStimulusButton, SET_VALUE=sWidget.file4
			WIDGET_CONTROL, sWidget.wTaggedImgButton, SET_VALUE=sWidget.file5
			sWidget.Acquire_Channels[3] = 3
			sWidget.Acquire_Files[3] = sWidget.file4
			sWidget.StimulusCheck = 1
		; User has selected to NOT collect stimulus signal, remove collection/regression options
		; Change appropriate values for acquisition, make interface reflect the change
		ENDIF ELSE BEGIN
			WIDGET_CONTROL, sWidget.wCVIOStimulusButton, SET_VALUE='Stimulus'
			sWidget.Channels = sWidget.Channels - 1
			; If not collecting any other channel, can't access images for variable
			if ((sWidget.RespiratoryCheck eq -1) AND (sWidget.StimulusCheck eq -1) and $
			    (sWidget.ImagAcqCheck eq -1)) then begin
				WIDGET_CONTROL, sWidget.wTaggedImgButton, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wTaggedImgButton, SET_VALUE='Tagged Img Data'
			endif
			WIDGET_CONTROL, sWidget.wStimulusButton, SENSITIVE=0
			WIDGET_CONTROL, sWidget.wStimulusButton, SET_VALUE='Stimulus'
			sWidget.Acquire_Channels[3] = -1
			sWidget.Acquire_Files[3] = ''
			sWidget.StimulusCheck = -1
		ENDELSE
	END

	sWidget.wTaggedImgButton : begin
		; User has selected independent variable for regression to be raw tagged images
		; Make interface reflect the change
		if (sWidget.IndependentCheck eq -1) then begin
			WIDGET_CONTROL, sWidget.wTaggedImgButton,SET_VALUE="*"+sWidget.file5+"*"
			sWidget.IndependentCheck = 1
		endif else begin
			WIDGET_CONTROL, sWidget.wTaggedImgButton,SET_VALUE=sWidget.file5
			sWidget.IndependentCheck = -1
		endelse
	END

	sWidget.wOrderList: begin
		; Get whether images are interleaved or sequential
		; Store image collection type in variable for future purposes
		currentOrderList = WIDGET_INFO(sWidget.wOrderList, /DropList_Select)
		WIDGET_CONTROL, sWidget.wOrderList, GET_UVALUE=orderListValues
		temp = orderListValues[currentOrderList]
		if (temp eq 'Sequential') then begin
			sWidget.Order = 0
		endif else begin
			sWidget.Order = 1
		endelse
		; Re-determine which slice to regress based on change in collection type
		if (sWidget.RegressSlice ne -1) then begin
			sWidget.GetPicture = sWidget.RegressSlice
			; Because collection is interleaved, img acq cvio data is interleaved, need to sort
			if (sWidget.RegressSlice eq 1) then begin
				; do nothing - 1 is still 1 when sorted
			endif else begin
				calc1 = (sWidget.NumSli MOD 2)
				if (calc1 eq 0) then begin
					start_even=sWidget.NumSli
				endif else begin
					start_even=sWidget.NumSli + 1
				endelse
				calc2 = (sWidget.RegressSlice MOD 2)
				if (calc2 eq 0) then begin
					actual_slice=(start_even/2)+1+(sWidget.RegressSlice-2)/2
				endif else begin
					actual_slice=(sWidget.RegressSlice+1)/2
				endelse
				sWidget.RegressSlice=actual_slice
			endelse
		endif
	END

	sWidget.wWinText : begin
		; If necessary, get sliding window length for regression
		WIDGET_CONTROL, sWidget.wWinText, GET_VALUE = temp
		sWidget.SlidingWindow = temp
		; Update the user on status
		WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Sliding Window Successfully Entered'
	END

	;sWidget.wNoWindowButton : begin
	;	; If necessary, get sliding window length for regression
	;	if (sWidget.SlidingWindow eq -1) then begin
	;		; Do nothing - there is nothing to remove
	;	endif else begin
	;		sWidget.SlidingWindow = -1
	;	endelse			
	;END

	sWidget.wRegressSliceText : begin
		; Get which slice to regress and determine position of slice in image 
		; based on order of collection and number of slices
		WIDGET_CONTROL, sWidget.wRegressSliceText, GET_VALUE = temp
		sWidget.RegressSlice = temp
		; If sequential storage (getting images from cross-mounted hard drive), save 
		; which slice want to get pictures of
		if (sWidget.Order ne -1) then begin
			sWidget.GetPicture = sWidget.RegressSlice
			; Because collection is interleaved, img acq cvio data is interleaved, need to sort
			if (sWidget.RegressSlice eq 1) then begin
				; do nothing - 1 is still 1 when sorted
			endif else begin
				calc1 = (sWidget.NumSli MOD 2)
				if (calc1 eq 0) then begin
					start_even = sWidget.NumSli
				endif else begin
					start_even = sWidget.NumSli + 1
				endelse
				calc2 = (sWidget.RegressSlice MOD 2)
				if (calc2 eq 0) then begin
					actual_slice = (start_even/2) + 1 + (sWidget.RegressSlice-2)/2
				endif else begin
					actual_slice = (sWidget.RegressSlice+1)/2
				endelse
				sWidget.RegressSlice = actual_slice
			endelse
		endif

		; If user enters number greater than number of slices, set value to 1
		if (sWidget.RegressSlice gt sWidget.NumSli) then begin
			WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Invalid Entry'
			sWidget.RegressSlice = 1
		endif
		; Update the user on status
		WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Regression Slice = ' $
			+STRING(sWidget.RegressSlice)
	END

	sWidget.wCardNoneButton : begin
		; User has decided to regress against unfiltered cardiac signal, make appropriate
		; changes to regression variables, make interface reflect the change	
		if (sWidget.Card_None eq -1) then begin
			WIDGET_CONTROL, sWidget.wCardNoneButton, SET_VALUE='*None*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Card_None = 1
			sWidget.Multi_Card_Regress = sWidget.Multi_Card_Regress + 1
			sWidget.Cardiac_Regress = 1
			cardActive, sWidget
		endif else begin
			; User has decided NOT to regress against unfiltered cardiac signal, make
			; appropriate changes to regression variables, make interface reflect change
			WIDGET_CONTROL, sWidget.wCardNoneButton, SET_VALUE='None'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Card_None = -1
			sWidget.Cardiac_Regress = -1
			cardInactive, sWidget
			cardReset, sWidget
		endelse
	END

	sWidget.wCardPostEventButton : begin 
		; User has decided to regress against post-event filtered cardiac signal, make
		; appropriate changes to regression variables, make interface reflect the change
		if (sWidget.Card_Post eq -1) then begin
			WIDGET_CONTROL, sWidget.wCardPostEventButton, SET_VALUE='*Post-Event Interval*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Card_Post = 1
			sWidget.Multi_Card_Regress = sWidget.Multi_Card_Regress + 1
			sWidget.Cardiac_Regress = 1
			cardActive, sWidget
		endif else begin
			; User has decided NOT to regress against post-event filtered cardiac signal,
			; make appropriate changes to regression variables, make interface reflect change
			WIDGET_CONTROL, sWidget.wCardPostEventButton,SET_VALUE='Post-Event Interval'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Card_Post = -1
			sWidget.Cardiac_Regress = -1
			cardInactive, sWidget
			cardReset, sWidget
		endelse
	END

	sWidget.wCardPreEventButton : begin
		; User has decided to regress against pre-event filtered cardiac signal, make 
		; appropriate changes to regression variables, make interface reflect change
		if (sWidget.Card_Pre eq -1) then begin
			WIDGET_CONTROL, sWidget.wCardPreEventButton, SET_VALUE='*Pre-Event Interval*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Card_Pre = 1
			sWidget.Multi_Card_Regress = sWidget.Multi_Card_Regress + 1
			sWidget.Cardiac_Regress = 1
			cardActive, sWidget
		endif else begin
			; User has decided NOT to regress against pre-event filtered cardiac signal,
			; make appropriate changes to regression variables, make interface reflect change
			WIDGET_CONTROL, sWidget.wCardPreEventButton,SET_VALUE='Pre-Event Interval'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Card_Pre = -1
			sWidget.Cardiac_Regress = -1
			cardInactive, sWidget
			cardReset, sWidget
		endelse
	END

	sWidget.wCardRelativeCycleButton : begin
		; User has decided to regress against relative cycle filtered cardiac signal, make
		; appropriate changes to regression variables, make interface reflect change
		if (sWidget.Card_Rel eq -1) then begin
			WIDGET_CONTROL, sWidget.wCardRelativeCycleButton, $
				SET_VALUE='*Relative Cycle Position*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Card_Rel = 1
			sWidget.Multi_Card_Regress = sWidget.Multi_Card_Regress + 1
			sWidget.Cardiac_Regress = 1
			cardActive, sWidget
		endif else begin
			; User has decided NOT to regress against relative cycle filtered cardiac signal
			; make appropriate changes to regression variables, make interface reflect change
			WIDGET_CONTROL, sWidget.wCardRelativeCycleButton, $
				SET_VALUE='Relative Cycle Position'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Card_Rel = -1
			sWidget.Cardiac_Regress = -1
			cardInactive, sWidget
			cardReset, sWidget
		endelse
	END

	sWidget.wCardConvolutionButton : begin
		; User has decided to regress against convolved cardiac signal, make appropriate
		; changes to regression variables, make interface reflect change
		if (sWidget.Card_Conv eq -1) then begin
			WIDGET_CONTROL, sWidget.wCardConvolutionButton, $
				SET_VALUE='*Convolution with HRF*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Card_Conv = 1
			sWidget.Multi_Card_Regress = sWidget.Multi_Card_Regress + 1
			sWidget.Cardiac_Regress = 1
			cardActive, sWidget
		endif else begin
			; User has decided NOT to regress against convolved cardiac signal, make
			; appropriate changes to regression variables, make interface reflect change
			WIDGET_CONTROL, sWidget.wCardConvolutionButton, $
				SET_VALUE='Convolution with HRF'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Card_Conv = -1
			sWidget.Cardiac_Regress = -1
			cardInactive, sWidget
			cardReset, sWidget
		endelse
	END

	sWidget.wCardSineButton : begin
		; User has decided to regress against sine filtered cardiac signal, make
		; appropriate changes to regression variables, make interface reflect change
		if (sWidget.Card_Sin eq -1) then begin
			WIDGET_CONTROL, sWidget.wCardSineButton, SET_VALUE='*Sine*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Card_Sin = 1
			sWidget.Multi_Card_Regress = sWidget.Multi_Card_Regress + 1
			sWidget.Cardiac_Regress = 1
			cardActive, sWidget
		endif else begin
			; User has decided NOT to regress against sine filtered cardiac signal, make
			; appropriate changes to regression variables, make interface reflect change
			WIDGET_CONTROL, sWidget.wCardSineButton, SET_VALUE='Sine'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Card_Sin = -1
			sWidget.Cardiac_Regress = -1
			cardInactive, sWidget
			cardReset, sWidget
		endelse
	END

	sWidget.wCardCosineButton : begin
		; User has decided to regress against cosine filtered cardiac signal, make
		; appropriate changes to regression variables, make interface reflect change
		if (sWidget.Card_Cos eq -1) then begin
			WIDGET_CONTROL, sWidget.wCardCosineButton, SET_VALUE='*Cosine*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Card_Cos = 1
			sWidget.Multi_Card_Regress = sWidget.Multi_Card_Regress + 1
			sWidget.Cardiac_Regress = 1
			cardActive, sWidget
		endif else begin
			; User has decided NOT to regress against cosine filtered cardiac signal, make
			; appropriate changes to regression variables, make interface reflect change
			WIDGET_CONTROL, sWidget.wCardCosineButton, SET_VALUE='Cosine'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Card_Cos = -1
			sWidget.Cardiac_Regress = -1
			cardInactive, sWidget
			cardReset, sWidget
		endelse
	END

	sWidget.wCardLookupButton : begin
		; User has decided to regress against lookup table filtered cardiac signal, make
		; appropriate changes to regression variables, make interface reflect change
		if (sWidget.Card_Look eq -1) then begin
			WIDGET_CONTROL, sWidget.wCardLookupButton, SET_VALUE='*Lookup Value*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Card_Look = 1
			sWidget.Multi_Card_Regress = sWidget.Multi_Card_Regress + 1
			sWidget.Cardiac_Regress = 1
			cardActive, sWidget
		endif else begin
			; User has decided NOT to regress against lookup table filtered cardiac signal,
			; make appropriate changes to regression variables, make interface reflect change
			WIDGET_CONTROL, sWidget.wCardLookupButton, SET_VALUE='Lookup Value'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Card_Look = -1
			sWidget.Cardiac_Regress = -1
			cardInactive, sWidget
			cardReset, sWidget
		endelse
	END

	sWidget.wRespNoneButton : begin	
		; User has decided to regress against unfiltered respiratory signal, make
		; appropriate changes to regression variables, make interface reflect change
		if (sWidget.Resp_None eq -1) then begin
			WIDGET_CONTROL, sWidget.wRespNoneButton, SET_VALUE='*None*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Resp_None = 1
			sWidget.Multi_Resp_Regress = sWidget.Multi_Resp_Regress + 1
			sWidget.Respiratory_Regress = 1
			respActive, sWidget
		endif else begin
			; User has decided NOT to regress against unfiltered respiratory signal,
			; make appropriate changes to regression variables, make interface reflect change
			WIDGET_CONTROL, sWidget.wRespNoneButton, SET_VALUE='None'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Resp_None = -1
			sWidget.Respiratory_Regress = -1
			respInactive, sWidget
			respReset, sWidget
		endelse
	END

	sWidget.wRespPostEventButton : begin 
		; User has decided to regress against post-event filtered respiratory signal, make
		; appropriate changes to regression variables, make interface reflect change
		if (sWidget.Resp_Post eq -1) then begin
			WIDGET_CONTROL, sWidget.wRespPostEventButton, SET_VALUE='*Post-Event Interval*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Resp_Post = 1
			sWidget.Multi_Resp_Regress = sWidget.Multi_Resp_Regress + 1
			sWidget.Respiratory_Regress = 1
			respActive, sWidget
		endif else begin
			; User has decided NOT to regress against post-event filtered respiratory signal,
			; make appropriate changes to regression variables, make interface reflect change
			WIDGET_CONTROL, sWidget.wRespPostEventButton,SET_VALUE='Post-Event Interval'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Resp_Post = -1
			sWidget.Respiratory_Regress = -1
			respInactive, sWidget
			respReset, sWidget
		endelse
	END

	sWidget.wRespPreEventButton : begin
		; User has decided to regress against pre-event filtered respiratory signal, make
		; appropriate changes to regression variables, make interface reflect change
		if (sWidget.Resp_Pre eq -1) then begin
			WIDGET_CONTROL, sWidget.wRespPreEventButton, SET_VALUE='*Pre-Event Interval*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Resp_Pre = 1
			sWidget.Multi_Resp_Regress = sWidget.Multi_Resp_Regress + 1
			sWidget.Respiratory_Regress = 1
			respActive, sWidget
		endif else begin
			; User has decided NOT to regress against pre-event filtered respiratory signal,
			; make appropriate changes to regression variables, make interface reflect change
			WIDGET_CONTROL, sWidget.wRespPreEventButton,SET_VALUE='Pre-Event Interval'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Resp_Pre = -1
			sWidget.Respiratory_Regress = -1
			respInactive, sWidget
			respReset, sWidget
		endelse
	END

	sWidget.wRespRelativeCycleButton : begin
		; User has decided to regress against relative cycle filtered respiratory signal,
		; make appropriate changes to regression variables, make interface reflect change
		if (sWidget.Resp_Rel eq -1) then begin
			WIDGET_CONTROL, sWidget.wRespRelativeCycleButton, $
				SET_VALUE='*Relative Cycle Position*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Resp_Rel = 1
			sWidget.Multi_Resp_Regress = sWidget.Multi_Resp_Regress + 1
			sWidget.Respiratory_Regress = 1
			respActive, sWidget
		endif else begin
			; User has decided NOT to regress against relative cycle filtered respiratory
			; signal, make appropriate changes to regression variables,
			; make interface reflect change
			WIDGET_CONTROL, sWidget.wRespRelativeCycleButton,SET_VALUE='Relative Cycle Position'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Resp_Rel = -1
			sWidget.Respiratory_Regress = -1
			respInactive, sWidget
			respReset, sWidget
		endelse
	END

	sWidget.wRespConvolutionButton : begin
		; User has decided to regress against HRF convolution filtered respiratory signal,
		; make appropriate changes to regression variables, make interface reflect change
		if (sWidget.Resp_Conv eq -1) then begin
			WIDGET_CONTROL, sWidget.wRespConvolutionButton, $
				SET_VALUE='*Convolution with HRF*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Resp_Conv = 1
			sWidget.Multi_Resp_Regress = sWidget.Multi_Resp_Regress + 1
			sWidget.Respiratory_Regress = 1
			respActive, sWidget
		endif else begin
			; User has decided NOT to regress against HRF convolution filtered 
			; respiratory signal, make appropriate changes to regression variables, 
			; make interface reflect change
			WIDGET_CONTROL, sWidget.wRespConvolutionButton,SET_VALUE='Convolution with HRF'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Resp_Conv = -1
			sWidget.Respiratory_Regress = -1
			respInactive, sWidget
			respReset, sWidget
		endelse
	END

	sWidget.wRespSineButton : begin
		; User has decided to regress against sine filtered respiratory signal,
		; make appropriate changes to regression variables, make interface reflect change
		if (sWidget.Resp_Sin eq -1) then begin
			WIDGET_CONTROL, sWidget.wRespSineButton, SET_VALUE='*Sine*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Resp_Sin = 1
			sWidget.Multi_Resp_Regress = sWidget.Multi_Resp_Regress + 1
			sWidget.Respiratory_Regress = 1
			respActive, sWidget
		endif else begin
			; User has decided NOT to regress against sine filtered respiratory signal
			; make appropriate changes to regression variables, make interface reflect change
			WIDGET_CONTROL, sWidget.wRespSineButton, SET_VALUE='Sine'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Resp_Sin = -1
			sWidget.Respiratory_Regress = -1
			respInactive, sWidget
			respReset, sWidget
		endelse
	END

	sWidget.wRespCosineButton : begin
		; User has decided to regress against cosine filtered respiratory signal,
		; make appropriate changes to regression variables, make interface reflect change
		if (sWidget.Resp_Cos eq -1) then begin
			WIDGET_CONTROL, sWidget.wRespCosineButton, SET_VALUE='*Cosine*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Resp_Cos = 1
			sWidget.Multi_Resp_Regress = sWidget.Multi_Resp_Regress + 1
			sWidget.Respiratory_Regress = 1
			respActive, sWidget
		endif else begin
			; User has decided NOT to regress against cosine filtered respiratory signal,
			; make appropriate changes to regression variables, make interface reflect change
			WIDGET_CONTROL, sWidget.wRespCosineButton, SET_VALUE='Cosine'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Resp_Cos = -1
			sWidget.Respiratory_Regress = -1
			respInactive, sWidget
			respReset, sWidget
		endelse
	END

	sWidget.wRespLookupButton : begin
		; User has decided to regress against lookup table filtered respiratory signal,
		; make appropriate changes to regression variables, make interface reflect change
		if (sWidget.Resp_Look eq -1) then begin
			WIDGET_CONTROL, sWidget.wRespLookupButton, SET_VALUE='*Lookup Value*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Resp_Look = 1
			sWidget.Multi_Resp_Regress = sWidget.Multi_Resp_Regress + 1
			sWidget.Respiratory_Regress = 1
			respActive, sWidget
		endif else begin
			; User has decided NOT to regress against lookup table filtered respiratory
			; signal, make appropriate changes to regression variables, make interface 
			; reflect change
			WIDGET_CONTROL, sWidget.wRespLookupButton, SET_VALUE='Lookup Value'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Resp_Look = -1
			sWidget.Respiratory_Regress = -1
			respInactive, sWidget
			respReset, sWidget
		endelse
	END

	sWidget.wStimNoneButton : begin	
		; User has decided to regress against unfiltered stimulus signal, make
		; appropriate changes to regression variables, make interface reflect change
		if (sWidget.Stim_None eq -1) then begin
			WIDGET_CONTROL, sWidget.wStimNoneButton, SET_VALUE='*None*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Stim_None = 1
			sWidget.Multi_Stim_Regress = sWidget.Multi_Stim_Regress + 1
			sWidget.Stimulus_Regress = 1
			stimActive, sWidget
		endif else begin
			; User has decided NOT to regress against unfiltered stimulus signal,
			; make appropriate changes to regression variables, make interface reflect change
			WIDGET_CONTROL, sWidget.wStimNoneButton, SET_VALUE='None'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Stim_None = -1
			sWidget.Stimulus_Regress = -1
			stimInactive, sWidget
			stimReset, sWidget
		endelse
	END

	sWidget.wStimPostEventButton : begin 
		; User has decided to regress against post-event filtered stimulus signal,
		; make appropriate changes to regression variables, make interface reflect change
		if (sWidget.Stim_Post eq -1) then begin
			WIDGET_CONTROL, sWidget.wStimPostEventButton, SET_VALUE='*Post-Event Interval*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Stim_Post = 1
			sWidget.Multi_Stim_Regress = sWidget.Multi_Stim_Regress + 1
			sWidget.Stimulus_Regress = 1
			stimActive, sWidget
		endif else begin
			; User has decided NOT to regress against post-event filtered stimulus signal,
			; make appropriate changes to regression variables, make interface reflect change
			WIDGET_CONTROL, sWidget.wStimPostEventButton,SET_VALUE='Post-Event Interval'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Stim_Post = -1
			sWidget.Stimulus_Regress = -1
			stimInactive, sWidget
			stimReset, sWidget
		endelse
	END

	sWidget.wStimPreEventButton : begin
		; User has decided to regress against pre-event filtered stimulus signal,
		; make appropriate changes to regression variables, make interface reflect change
		if (sWidget.Stim_Pre eq -1) then begin
			WIDGET_CONTROL, sWidget.wStimPreEventButton, SET_VALUE='*Pre-Event Interval*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Stim_Pre = 1
			sWidget.Multi_Stim_Regress = sWidget.Multi_Stim_Regress + 1
			sWidget.Stimulus_Regress = 1
			stimActive, sWidget
		endif else begin
			; User has decided NOT to regress against pre-event filtered stimulus signal,
			; make appropriate changes to regression variables, make interface reflect change
			WIDGET_CONTROL, sWidget.wStimPreEventButton,SET_VALUE='Pre-Event Interval'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Stim_Pre = -1
			sWidget.Stimulus_Regress = -1
			stimInactive, sWidget
			stimReset, sWidget
		endelse
	END

	sWidget.wStimRelativeCycleButton : begin
		; User has decided to regress against relative cycle filtered stimulus signal,
		; make appropriate changes to regression variables, make interface reflect change
		if (sWidget.Stim_Rel eq -1) then begin
			WIDGET_CONTROL, sWidget.wStimRelativeCycleButton, $
				SET_VALUE='*Relative Cycle Position*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Stim_Rel = 1
			sWidget.Multi_Stim_Regress = sWidget.Multi_Stim_Regress + 1
			sWidget.Stimulus_Regress = 1
			stimActive, sWidget
		endif else begin
			; User has decided NOT to regress against relative cycle filtered stimulus
			; signal, make appropriate changes to regression variables, make interface 
			; reflect change
			WIDGET_CONTROL, sWidget.wStimRelativeCycleButton, $
				SET_VALUE='Relative Cycle Position'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Stim_Rel = -1
			sWidget.Stimulus_Regress = -1
			stimInactive, sWidget
			stimReset, sWidget
		endelse
	END

	sWidget.wStimConvolutionButton : begin
		; User has decided to regress against HRF convolution filtered stimulus signal,
		; make appropriate changes to regression variables, make interface reflect change
		if (sWidget.Stim_Conv eq -1) then begin
			WIDGET_CONTROL, sWidget.wStimConvolutionButton, $
				SET_VALUE='*Convolution with HRF*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Stim_Conv = 1
			sWidget.Multi_Stim_Regress = sWidget.Multi_Stim_Regress + 1
			sWidget.Stimulus_Regress = 1
			stimActive, sWidget
		endif else begin
			; User has decided NOT to regress against HRF convolution filtered stimulus signal,
			; make appropriate changes to regression variables, make interface reflect change
			WIDGET_CONTROL, sWidget.wStimConvolutionButton, $
				SET_VALUE='Convolution with HRF'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Stim_Conv = -1
			sWidget.Stimulus_Regress = -1
			stimInactive, sWidget
			stimReset, sWidget
		endelse
	END

	sWidget.wStimSineButton : begin
		; User has decided to regress against sine filtered stimulus signal,
		; make appropriate changes to regression variables, make interface reflect change
		if (sWidget.Stim_Sin eq -1) then begin
			WIDGET_CONTROL, sWidget.wStimSineButton, SET_VALUE='*Sine*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Stim_Sin = 1
			sWidget.Multi_Stim_Regress = sWidget.Multi_Stim_Regress + 1
			sWidget.Stimulus_Regress = 1
			stimActive, sWidget
		endif else begin
			; User has decided NOT to regress against sine filtered stimulus signal,
			; make appropriate changes to regression variables, make interface reflect change
			WIDGET_CONTROL, sWidget.wStimSineButton, SET_VALUE='Sine'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Stim_Sin = -1
			sWidget.Stimulus_Regress = -1
			stimInactive, sWidget
			stimReset, sWidget
		endelse
	END

	sWidget.wStimCosineButton : begin
		; User has decided to regress against cosine filtered stimulus signal, make
		; appropriate changes to regression variables, make interface reflect change
		if (sWidget.Stim_Cos eq -1) then begin
			WIDGET_CONTROL, sWidget.wStimCosineButton, SET_VALUE='*Cosine*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Stim_Cos = 1
			sWidget.Multi_Stim_Regress = sWidget.Multi_Stim_Regress + 1
			sWidget.Stimulus_Regress = 1
			stimActive, sWidget
		endif else begin
			; User has decided NOT to regress against cosine filtered stimulus signal,
			; make appropriate changes to regression variables, make interface reflect change
			WIDGET_CONTROL, sWidget.wStimCosineButton, SET_VALUE='Cosine'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Stim_Cos = -1
			sWidget.Stimulus_Regress = -1
			stimInactive, sWidget
			stimReset, sWidget
		endelse
	END

	sWidget.wStimLookupButton : begin
		; User has decided to regress against lookup table filtered stimulus signal,
		; make appropriate changes to regression variables, make interface reflect change
		if (sWidget.Stim_Look eq -1) then begin
			WIDGET_CONTROL, sWidget.wStimLookupButton, SET_VALUE='*Lookup Value*'
			sWidget.Num_Vars = sWidget.Num_Vars + 1
			sWidget.Stim_Look = 1
			sWidget.Multi_Stim_Regress = sWidget.Multi_Stim_Regress + 1
			sWidget.Stimulus_Regress = 1
			stimActive, sWidget
		endif else begin
			; User has decided NOT to regress against lookup table filtered stimulus signal,
			; make appropriate changes to regression variables, make interface reflect change
			WIDGET_CONTROL, sWidget.wStimLookupButton, SET_VALUE='Lookup Value'
			sWidget.Num_Vars = sWidget.Num_Vars - 1
			sWidget.Stim_Look = -1
			sWidget.Stimulus_Regress = -1
			stimInactive, sWidget
			stimReset, sWidget
		endelse
	END

	sWidget.wConstantButton : begin
		; User has decided not to include constant term in regression, change
		; appropriate variables, make interface reflect change
		if (sWidget.Constant EQ 1) then begin
			WIDGET_CONTROL, sWidget.wConstantButton, SET_VALUE='*Constant Removed*'
			sWidget.Constant = -1
			sWidget.Num_Vars = sWidget.Num_Vars - 1
		; User has decided to include constant term in regression, change 
		; appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wConstantButton, SET_VALUE='Remove Constant'
			sWidget.Constant = 1
			sWidget.Num_Vars = sWidget.Num_Vars + 1
		endelse
	END

	sWidget.wTrendingButton : begin
		; User has decided not to include constant term in regression, change
		; appropriate variables, make interface reflect change
		if (sWidget.Trending EQ -1) then begin
			WIDGET_CONTROL, sWidget.wTrendingButton, SET_VALUE='*Linear Trend Included*'
			sWidget.Trending = 1
			sWidget.Num_Vars = sWidget.Num_Vars + 1
		; User has decided to include constant term in regression, change 
		; appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wTrendingButton, SET_VALUE='Include Linear Trend'
			sWidget.Trending = -1
			sWidget.Num_Vars = sWidget.Num_Vars - 1
		endelse
	END

	sWidget.wGlobalButton : begin
		; User has decided not to include constant term in regression, change
		; appropriate variables, make interface reflect change
		if (sWidget.Global EQ -1) then begin
			WIDGET_CONTROL, sWidget.wGlobalButton, SET_VALUE='*Global Average Included*'
			sWidget.Global = 1
			sWidget.Num_Vars = sWidget.Num_Vars + 1
		; User has decided to include constant term in regression, change 
		; appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wGlobalButton, SET_VALUE='Include Global Average'
			sWidget.Global = -1
			sWidget.Num_Vars = sWidget.Num_Vars - 1
		endelse
	END

	sWidget.wAutoRegressionButton : begin
		;sWidget. ... 		Assign variable values
	END

	sWidget.wOSOSCardButton : begin
		if (sWidget.ResultTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the output display 
			; show the sum of squares of the cardiac signal, change the appropriate 
			; variables, make interface reflect change
			if (sWidget.ResultTypeOutput eq 5) then begin
				sWidget.ResultTypeOutput = -1
				WIDGET_CONTROL, sWidget.wOSOSCardButton, SET_VALUE='Cardiac'
			; The user wants to change previously selected option to make the output display 
			; show the sum of squares of the cardiac signal, change the appropriate 
			; variables, make interface reflect change
			endif else begin
				OutReset, sWidget
				sWidget.ResultTypeOutput = 5
				WIDGET_CONTROL, sWidget.wOSOSCardButton, SET_VALUE='*Cardiac*'
				sWidget.OutputFile='fmri'+sWidget.RunNum+'_ss_raw.cvio'
			endelse
		; The user wants the output display to show the sum of squares of the cardiac
		; signal, change the appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wOSOSCardButton, SET_VALUE='*Cardiac*'
			sWidget.ResultTypeOutput = 5
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_ss_raw.cvio'
		endelse
	END

	sWidget.wOSOSRespButton : begin
		if (sWidget.ResultTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the output display 
			; show the sum of squares of the respiratory signal, change the appropriate 
			; variables, make interface reflect change
			if (sWidget.ResultTypeOutput eq 6) then begin
				sWidget.ResultTypeOutput = -1
				WIDGET_CONTROL, sWidget.wOSOSRespButton, SET_VALUE='Respiratory'
			; The user wants to change previously selected option to make the output display 
			; show the sum of squares of the respiratory signal, change the appropriate 
			; variables, make interface reflect change
			endif else begin
				OutReset, sWidget
				sWidget.ResultTypeOutput = 6
				WIDGET_CONTROL, sWidget.wOSOSRespButton, SET_VALUE='*Respiratory*'
				sWidget.OutputFile='fmri'+sWidget.RunNum+'_ss_raw.cvio'
			endelse
		; The user wants the output display to show the sum of squares of the respiratory
		; signal, change the appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wOSOSRespButton, SET_VALUE='*Respiratory*'
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_ss_raw.cvio'
			sWidget.ResultTypeOutput = 6
		endelse
	END

	sWidget.wOSOSStimButton : begin
		if (sWidget.ResultTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the output display 
			; show the sum of squares of the stimulus signal, change the appropriate 
			; variables, make interface reflect change
			if (sWidget.ResultTypeOutput eq 7) then begin
				sWidget.ResultTypeOutput = -1
				WIDGET_CONTROL, sWidget.wOSOSStimButton, SET_VALUE='Stimulus'
			; The user wants to change previously selected option to make the output display 
			; show the sum of squares of the stimulus signal, change the appropriate 
			; variables, make interface reflect change
			endif else begin
				OutReset, sWidget
				sWidget.ResultTypeOutput = 7
				WIDGET_CONTROL, sWidget.wOSOSStimButton, SET_VALUE='*Stimulus*'
				sWidget.OutputFile='fmri'+sWidget.RunNum+'_ss_raw.cvio'
			endelse
		; The user wants the output display to show the sum of squares of the stimulus
		; signal, change the appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wOSOSStimButton, SET_VALUE='*Stimulus*'
			sWidget.ResultTypeOutput = 7
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_ss_raw.cvio'
		endelse
	END

	sWidget.wORegCoefCardButton : begin
		if (sWidget.ResultTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the output display 
			; show the regression coefficients of the cardiac signal, change the appropriate 
			; variables, make interface reflect change
			if (sWidget.ResultTypeOutput eq 8) then begin
				sWidget.ResultTypeOutput = -1
				WIDGET_CONTROL, sWidget.wORegCoefCardButton, SET_VALUE='Cardiac'
			; The user wants to change previously selected option to make the output display 
			; show the regression coefficients of the cardiac signal, change the appropriate 
			; variables, make interface reflect change
			endif else begin
				OutReset, sWidget
				sWidget.ResultTypeOutput = 8
				WIDGET_CONTROL, sWidget.wORegCoefCardButton, SET_VALUE='*Cardiac*'
				sWidget.OutputFile='fmri'+sWidget.RunNum+'_coef_raw.cvio'
			endelse
		; The user wants the output display to show the regression coefficients 
		; of the cardiac signal, change the appropriate variables,
		; make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wORegCoefCardButton, SET_VALUE='*Cardiac*'
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_coef_raw.cvio'
			sWidget.ResultTypeOutput = 8
		endelse
	END

	sWidget.wORegCoefRespButton : begin
		if (sWidget.ResultTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the output display 
			; show the regression coefficients of the respiratory signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.ResultTypeOutput eq 9) then begin
				sWidget.ResultTypeOutput = -1
				WIDGET_CONTROL, sWidget.wORegCoefRespButton, SET_VALUE='Respiratory'
			; The user wants to change previously selected option to make the output display 
			; show the regression coefficients of the respiratory signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				OutReset, sWidget
				sWidget.ResultTypeOutput = 9
				WIDGET_CONTROL, sWidget.wORegCoefRespButton, SET_VALUE='*Respiratory*'
				sWidget.OutputFile='fmri'+sWidget.RunNum+'_coef_raw.cvio'
			endelse
		; The user wants the output display to show the regression coefficients 
		; of the respiratory signal, change the appropriate variables,
		; make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wORegCoefRespButton, SET_VALUE='*Respiratory*'
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_coef_raw.cvio'
			sWidget.ResultTypeOutput = 9
		endelse
	END

	sWidget.wORegCoefStimButton : begin
		if (sWidget.ResultTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the output display 
			; show the regression coefficients of the stimulus signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.ResultTypeOutput eq 10) then begin
				sWidget.ResultTypeOutput = -1
				WIDGET_CONTROL, sWidget.wORegCoefStimButton, SET_VALUE='Stimulus'
			; The user wants to change previously selected option to make the output display 
			; show the regression coefficients of the stimulus signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				OutReset, sWidget
				sWidget.ResultTypeOutput = 10
				WIDGET_CONTROL, sWidget.wORegCoefStimButton, SET_VALUE='*Stimulus*'
				sWidget.OutputFile='fmri'+sWidget.RunNum+'_coef_raw.cvio'
			endelse
		; The user wants the output display to show the regression coefficients 
		; of the stimulus signal, change the appropriate variables,
		; make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wORegCoefStimButton, SET_VALUE='*Stimulus*'
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_coef_raw.cvio'
			sWidget.ResultTypeOutput = 10
		endelse
	END

	sWidget.wOEffectCardButton : begin
		if (sWidget.ResultTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the output display 
			; show the effect size of the cardiac signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.ResultTypeOutput eq 11) then begin
				sWidget.ResultTypeOutput = -1
				WIDGET_CONTROL, sWidget.wOEffectCardButton, SET_VALUE='Cardiac'
			; The user wants to change previously selected option to make the output display 
			; show the effect size of the cardiac signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				OutReset, sWidget
				sWidget.ResultTypeOutput = 11
				WIDGET_CONTROL, sWidget.wOEffectCardButton, SET_VALUE='*Cardiac*'
				sWidget.OutputFile='fmri'+sWidget.RunNum+'_effect_raw.cvio'
			endelse
		; The user wants the output display to show the effect size of the cardiac  
		; signal, change the appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wOEffectCardButton, SET_VALUE='*Cardiac*'
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_effect_raw.cvio'
			sWidget.ResultTypeOutput = 11
		endelse
	END

	sWidget.wOEffectRespButton : begin
		if (sWidget.ResultTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the output display 
			; show the effect size of the respiratory signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.ResultTypeOutput eq 12) then begin
				sWidget.ResultTypeOutput = -1
				WIDGET_CONTROL, sWidget.wOEffectRespButton, SET_VALUE='Respiratory'
			; The user wants to change previously selected option to make the output display 
			; show the effect size of the respiratory signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				OutReset, sWidget
				sWidget.ResultTypeOutput = 12
				WIDGET_CONTROL, sWidget.wOEffectRespButton, SET_VALUE='*Respiratory*'
				sWidget.OutputFile='fmri'+sWidget.RunNum+'_effect_raw.cvio'
			endelse
		; The user wants the output display to show the effect size of the respiratory
		; signal, change the appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wOEffectRespButton, SET_VALUE='*Respiratory*'
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_effect_raw.cvio'
			sWidget.ResultTypeOutput = 12
		endelse
	END

	sWidget.wOEffectStimButton : begin
		if (sWidget.ResultTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the output display 
			; show the effect size of the stimulus signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.ResultTypeOutput eq 13) then begin
				sWidget.ResultTypeOutput = -1
				WIDGET_CONTROL, sWidget.wOEffectStimButton, SET_VALUE='Stimulus'
			; The user wants to change previously selected option to make the output display 
			; show the effect size of the stimulus signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				OutReset, sWidget
				sWidget.ResultTypeOutput = 13
				WIDGET_CONTROL, sWidget.wOEffectStimButton, SET_VALUE='*Stimulus*'
				sWidget.OutputFile='fmri'+sWidget.RunNum+'_effect_raw.cvio'
			endelse
		; The user wants the output display to show the effect size of the stimulus
		; signal, change the appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wOEffectStimButton, SET_VALUE='*Stimulus*'
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_effect_raw.cvio'
			sWidget.ResultTypeOutput = 13
		endelse
	END

	sWidget.wOFstatCardButton : begin
		if (sWidget.ResultTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the output display 
			; show the F-statistic of the cardiac signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.ResultTypeOutput eq 17) then begin
				sWidget.ResultTypeOutput = -1
				WIDGET_CONTROL, sWidget.wOFstatCardButton, SET_VALUE='Cardiac'
			; The user wants to change previously selected option to make the output display 
			; show the F-statistic of the cardiac signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				OutReset, sWidget
				sWidget.ResultTypeOutput = 17
				WIDGET_CONTROL, sWidget.wOFstatCardButton, SET_VALUE='*Cardiac*'
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_fstat_raw.cvio'
			endelse
		; The user wants the output display to show the F-statistic of the cardiac 
		; signal, change the appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wOFstatCardButton, SET_VALUE='*Cardiac*'
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_fstat_raw.cvio'
			sWidget.ResultTypeOutput = 17
		endelse
	END

	sWidget.wOFstatRespButton : begin
		if (sWidget.ResultTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the output display 
			; show the F-statistic of the respiratory signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.ResultTypeOutput eq 18) then begin
				sWidget.ResultTypeOutput = -1
				WIDGET_CONTROL, sWidget.wOFstatRespButton, SET_VALUE='Respiratory'
			; The user wants to change previously selected option to make the output display 
			; show the F-statistic of the respiratory signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.ResulttTypeOutput = 18
				WIDGET_CONTROL, sWidget.wOFstatRespButton, SET_VALUE='*Respiratory*'
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_fstat_raw.cvio'
			endelse
		; The user wants the output display to show the F-statistic of the respiratory 
		; signal, change the appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wOFstatRespButton, SET_VALUE='*Respiratory*'
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_fstat_raw.cvio'
			sWidget.ResultTypeOutput = 18
		endelse
	END

	sWidget.wOFstatStimButton : begin
		if (sWidget.ResultTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the output display 
			; show the F-statistic of the stimulus signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.ResultTypeOutput eq 19) then begin
				sWidget.ResultTypeOutput = -1
				WIDGET_CONTROL, sWidget.wOFstatStimButton, SET_VALUE='Stimulus'
			; The user wants to change previously selected option to make the output display 
			; show the F-statistic of the stimulus signal, change the 
			; appropriate variables
			endif else begin
				OutReset, sWidget
				sWidget.ResultTypeOutput = 19
				WIDGET_CONTROL, sWidget.wOFstatStimButton, SET_VALUE='*Stimulus*'
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_fstat_raw.cvio'
			endelse
		; The user wants the output display to show the F-statistic of the stimulus
		; signal, change the appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wOFstatStimButton, SET_VALUE='*Stimulus*'
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_fstat_raw.cvio'
			sWidget.ResultTypeOutput = 19
		endelse
	END

	sWidget.wOTstatCardButton : begin
		if (sWidget.ResultTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the output display 
			; show the t-statistic of the cardiac signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.ResultTypeOutput eq 14) then begin
				sWidget.ResultTypeOutput = -1
				WIDGET_CONTROL, sWidget.wOTstatCardButton, SET_VALUE='Cardiac'
			; The user wants to change previously selected option to make the output display 
			; show the t-statistic of the stimulus signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				OutReset, sWidget
				sWidget.ResultTypeOutput = 14
				WIDGET_CONTROL, sWidget.wOTstatCardButton, SET_VALUE='*Cardiac*'
				sWidget.OutputFile='fmri'+sWidget.RunNum+'_tstat_raw.cvio'
			endelse
		; The user wants the output display to show the t-statistic of the stimulus 
		; signal, change the appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wOTstatCardButton, SET_VALUE='*Cardiac*'
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_tstat_raw.cvio'
			sWidget.ResultTypeOutput = 14
		endelse
	END

	sWidget.wOTstatRespButton : begin
		if (sWidget.ResultTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the output display 
			; show the t-statistic of the respiratory signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.ResultTypeOutput eq 15) then begin
				sWidget.ResultTypeOutput = -1
				WIDGET_CONTROL, sWidget.wOTstatRespButton, SET_VALUE='Respiratory'
			; The user wants to change previously selected option to make the output display 
			; show the t-statistic of the respiratory signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				OutReset, sWidget
				sWidget.ResultTypeOutput = 15
				WIDGET_CONTROL, sWidget.wOTstatRespButton, SET_VALUE='*Respiratory*'
				sWidget.OutputFile='fmri'+sWidget.RunNum+'_tstat_raw.cvio'
			endelse
		; The user wants the output display to show the t-statistic of the respiratory 
		; signal, change the appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wOTstatRespButton, SET_VALUE='*Respiratory*'
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_tstat_raw.cvio'
			sWidget.ResultTypeOutput = 15
		endelse
	END

	sWidget.wOTstatStimButton : begin
		if (sWidget.ResultTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the output display 
			; show the t-statistic of the stimulus signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.ResultTypeOutput eq 16) then begin
				sWidget.ResultTypeOutput = -1
				WIDGET_CONTROL, sWidget.wOTstatStimButton, SET_VALUE='Stimulus'
			; The user wants to change previously selected option to make the output display 
			; show the t-statistic of the stimulus signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				OutReset, sWidget
				sWidget.ResultTypeOutput = 16
				WIDGET_CONTROL, sWidget.wOTstatStimButton, SET_VALUE='*Stimulus*'
				sWidget.OutputFile='fmri'+sWidget.RunNum+'_tstat_raw.cvio'
			endelse
		; The user wants the output display to show the t-statistic of the stimulus 
		; signal, change the appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wOTstatStimButton, SET_VALUE='*Stimulus*'
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_tstat_raw.cvio'
			sWidget.ResultTypeOutput = 16
		endelse
	END

	sWidget.wOErrorCoefCardButton : begin
		if (sWidget.ResultTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the output display 
			; show the t-statistic of the cardiac signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.ResultTypeOutput eq 24) then begin
				sWidget.ResultTypeOutput = -1
				WIDGET_CONTROL, sWidget.wOErrorCoefCardButton, SET_VALUE='Cardiac'
			; The user wants to change previously selected option to make the output display 
			; show the t-statistic of the stimulus signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				OutReset, sWidget
				sWidget.ResultTypeOutput = 24
				WIDGET_CONTROL, sWidget.wOErrorCoefCardButton, SET_VALUE='*Cardiac*'
				sWidget.OutputFile='fmri'+sWidget.RunNum+'_tstat_raw.cvio'
			endelse
		; The user wants the output display to show the t-statistic of the stimulus 
		; signal, change the appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wOErrorCoefCardButton, SET_VALUE='*Cardiac*'
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_tstat_raw.cvio'
			sWidget.ResultTypeOutput = 24
		endelse
	END

	sWidget.wOErrorCoefRespButton : begin
		if (sWidget.ResultTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the output display 
			; show the t-statistic of the respiratory signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.ResultTypeOutput eq 25) then begin
				sWidget.ResultTypeOutput = -1
				WIDGET_CONTROL, sWidget.wOErrorCoefRespButton, SET_VALUE='Respiratory'
			; The user wants to change previously selected option to make the output display 
			; show the t-statistic of the respiratory signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				OutReset, sWidget
				sWidget.ResultTypeOutput = 25
				WIDGET_CONTROL, sWidget.wOErrorCoefRespButton, SET_VALUE='*Respiratory*'
				sWidget.OutputFile='fmri'+sWidget.RunNum+'_tstat_raw.cvio'
			endelse
		; The user wants the output display to show the t-statistic of the respiratory 
		; signal, change the appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wOErrorCoefRespButton, SET_VALUE='*Respiratory*'
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_tstat_raw.cvio'
			sWidget.ResultTypeOutput = 25
		endelse
	END

	sWidget.wOErrorCoefStimButton : begin
		if (sWidget.ResultTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the output display 
			; show the t-statistic of the stimulus signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.ResultTypeOutput eq 26) then begin
				sWidget.ResultTypeOutput = -1
				WIDGET_CONTROL, sWidget.wOErrorCoefStimButton, SET_VALUE='Stimulus'
			; The user wants to change previously selected option to make the output display 
			; show the t-statistic of the stimulus signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				OutReset, sWidget
				sWidget.ResultTypeOutput = 26
				WIDGET_CONTROL, sWidget.wOErrorCoefStimButton, SET_VALUE='*Stimulus*'
				sWidget.OutputFile='fmri'+sWidget.RunNum+'_tstat_raw.cvio'
			endelse
		; The user wants the output display to show the t-statistic of the stimulus 
		; signal, change the appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wOErrorCoefStimButton, SET_VALUE='*Stimulus*'
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_tstat_raw.cvio'
			sWidget.ResultTypeOutput = 26
		endelse
	END

	sWidget.wOCombocoefButton : begin
		if (sWidget.ResultTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the output display 
			; show the magnitude of the sin and cosine of the stimulus, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.ResultTypeOutput eq 23) then begin
				sWidget.ResultTypeOutput = -1
				WIDGET_CONTROL, sWidget.wOCombocoefButton, SET_VALUE='Sin and Cos Magnitude'
			; The user wants to change previously selected option to make the output display 
			; show the magnitude of the sin and cosine of the stimulus, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				OutReset, sWidget
				sWidget.ResultTypeOutput = 23
				WIDGET_CONTROL, sWidget.wOCombocoefButton, SET_VALUE='*Sin and Cos Magnitude*'
				sWidget.OutputFile='fmri'+sWidget.RunNum+'_magsincos_raw.cvio'
			endelse
		; The user wants the output display to show the magnitude of the sin and cosine $
		; of the stimulus  signal, change the appropriate variables, make interface $
		; reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wOCombocoefButton, SET_VALUE='*Sin and Cos Magnitude*'
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_magsincos_raw.cvio'
			sWidget.ResultTypeOutput = 23
		endelse
	END

	sWidget.wOComboangleButton : begin
		if (sWidget.ResultTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the output display 
			; show the phase lag of sin and cos of the stimulus signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.ResultTypeOutput eq 21) then begin
				sWidget.ResultTypeOutput = -1
				WIDGET_CONTROL,sWidget.wOComboangleButton,SET_VALUE='Sin and Cos Phase Lag'
			; The user wants to change previously selected option to make the output display 
			; show the phase lag of sin and cos of the stimulus signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				OutReset, sWidget
				sWidget.ResultTypeOutput = 21
				WIDGET_CONTROL,sWidget.wOComboangleButton,SET_VALUE='*Sin and Cos Phase Lag*'
				sWidget.OutputFile='fmri'+sWidget.RunNum+'_lagsincos_raw.cvio'
			endelse
		; The user wants the output display to show the phase lag of sin and cos of the $
		; stimulus signal signal, change the appropriate variables, make interface  $
		; reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wOComboangleButton, SET_VALUE='*Sin and Cos Phase Lag*'
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_lagsincos_raw.cvio'
			sWidget.ResultTypeOutput = 21
		endelse
	END

	sWidget.wOCombofstatButton : begin
		if (sWidget.ResultTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the output display 
			; show the t-statistic of the sin and cos of the stimulus signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.ResultTypeOutput eq 22) then begin
				sWidget.ResultTypeOutput = -1
				WIDGET_CONTROL,sWidget.wOCombotstatButton,SET_VALUE='Sin and Cos F-stat'
			; The user wants to change previously selected option to make the output display 
			; show the t-statistic of the sin and cos of the stimulus signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				OutReset, sWidget
				sWidget.ResultTypeOutput = 22
				WIDGET_CONTROL,sWidget.wOCombotstatButton,SET_VALUE='*Sin and Cos F-stat*'
				sWidget.OutputFile='fmri'+sWidget.RunNum+'_sincosfstat_raw.cvio'
			endelse
		; The user wants the output display to show the t-statistic of the sin and $
		; cos of the stimulus signal, change the appropriate variables, make interface $
		; reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wOCombotstatButton, SET_VALUE='*Sin and Cos F-stat*'
			sWidget.OutputFile='fmri'+sWidget.RunNum+'_sincosfstat_raw.cvio'
			sWidget.ResultTypeOutput = 22
		endelse
	END

	sWidget.wFeedbackList : begin
		; User enters the type of patient feedback desired, set appropriate variables
		currentFeedbackList = WIDGET_INFO(sWidget.wFeedbackList, /DropList_Select)
		WIDGET_CONTROL, sWidget.wFeedbackList, GET_UVALUE = feedbackListValues
		sWidget.FeedbackType = feedbackListValues[currentFeedbackList]
		;IF (sWidget.FeedbackType NE 'None') THEN BEGIN
			; give appropriate form of feedback
			;IF (sWidget.FeedbackType EQ 'Image - Visual') THEN BEGIN
			;	do nothing
			;ENDIF
			;IF (sWidget.FeedbackType EQ 'Image - Auditory') THEN BEGIN
			;	do nothing
			;ENDIF
			;IF (sWidget.FeedbackType EQ 'Stripchart') THEN BEGIN
			;	do nothing
			;ENDIF
		;ENDIF
	END

	sWidget.wCCardiacButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the cardiac signal, change the appropriate variables,
			; make interface reflect change
			if (sWidget.StripchartTypeOutput eq 1) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCCardiacButton, SET_VALUE='Cardiac'
			; The user wants to change previously selected option to make the stripchart
			; display show the cardiac signal, change the appropriate variables,
			; make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 1
				WIDGET_CONTROL, sWidget.wCCardiacButton, SET_VALUE='*Cardiac*'
			endelse
		; The user wants the stripchart display to show the cardiac signal, change the 
		; appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCCardiacButton, SET_VALUE='*Cardiac*'
			sWidget.StripchartTypeOutput = 1
		endelse
	END

	sWidget.wCRespiratoryButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the respiratory signal, change the appropriate variables,
			; make interface reflect change
			if (sWidget.StripchartTypeOutput eq 2) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCRespiratoryButton, SET_VALUE='Respiratory'
			; The user wants to change previously selected option to make the stripchart
			; display show the respiratory signal, change the appropriate variables,
			; make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 2
				WIDGET_CONTROL, sWidget.wCRespiratoryButton, SET_VALUE='*Respiratory*'
			endelse
		; The user wants the stripchart display to show the respiratory signal, change the 
		;  appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCRespiratoryButton, SET_VALUE='*Respiratory*'
			sWidget.StripchartTypeOutput = 2
		endelse
	END

	sWidget.wCStimulusButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the stimulus signal, change the appropriate variables,
			; make interface reflect change
			if (sWidget.StripchartTypeOutput eq 3) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCStimulusButton, SET_VALUE='Stimulus'
			; The user wants to change previously selected option to make the stripchart
			; display show the stimulus signal, change the appropriate variables,
			; make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 3
				WIDGET_CONTROL, sWidget.wCStimulusButton, SET_VALUE='*Stimulus*'
			endelse
		; The user wants the stripchart display to show the stimulus signal, change the 
		;  appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCStimulusButton, SET_VALUE='*Stimulus*'
			sWidget.StripchartTypeOutput = 3
		endelse
	END

	sWidget.wCImgAcqButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the image acquisition signal, change the appropriate variables,
			; make interface reflect change
			if (sWidget.StripchartTypeOutput eq 4) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCImgAcqButton, SET_VALUE='Image Acquisition'
			; The user wants to change previously selected option to make the stripchart
			; display show the image acquisition signal, change the appropriate variables,
			; make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 4
				WIDGET_CONTROL, sWidget.wCImgAcqButton, SET_VALUE='*Image Acquisition*'
			endelse
		; The user wants the stripchart display to show the image acquisition signal, change the 
		; appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCImgAcqButton, SET_VALUE='*Image Acquisition*'
			sWidget.StripchartTypeOutput = 4
		endelse
	END

	sWidget.wCSelectRawButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the image acquisition signal, change the appropriate variables,
			; make interface reflect change
			if (sWidget.StripchartTypeOutput eq 20) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCSelectRawButton, SET_VALUE='Mean of ROI Pixels'
				WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=0
			; The user wants to change previously selected option to make the stripchart
			; display show the image acquisition signal, change the appropriate variables,
			; make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 20
				WIDGET_CONTROL, sWidget.wCSelectRawButton, SET_VALUE='*Mean of ROI Pixels*'
				WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=0
			endelse
		; The user wants the stripchart display to show the image acquisition signal, change the 
		; appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCSelectRawButton, SET_VALUE='*Mean of ROI Pixels*'
				WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=0
			sWidget.StripchartTypeOutput = 20
		endelse
	END

	sWidget.wCMotionButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the image acquisition signal, change the appropriate variables,
			; make interface reflect change
			if (sWidget.StripchartTypeOutput eq 21) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCMotionButton, SET_VALUE='Motion Detection'
				WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=0
			; The user wants to change previously selected option to make the stripchart
			; display show the image acquisition signal, change the appropriate variables,
			; make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 21
				WIDGET_CONTROL, sWidget.wCMotionButton, SET_VALUE='*Motion Detection*'
				WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=0
			endelse
		; The user wants the stripchart display to show the image acquisition signal, change the 
		;  appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCMotionButton, SET_VALUE='*Motion Detection*'
				WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=0
			sWidget.StripchartTypeOutput = 21
		endelse
	END

	sWidget.wCCombofstatButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the image acquisition signal, change the appropriate variables,
			; make interface reflect change
			if (sWidget.StripchartTypeOutput eq 22) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCCombotstatButton, SET_VALUE='Sin and Cos F-stat'
				WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=0
			; The user wants to change previously selected option to make the stripchart
			; display show the image acquisition signal, change the appropriate variables,
			; make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 22
				WIDGET_CONTROL, sWidget.wCCombotstatButton, SET_VALUE='*Sin and Cos F-stat*'
				WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=0
			endelse
		; The user wants the stripchart display to show the image acquisition signal, change 
		; the appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCCombotstatButton, SET_VALUE='*Sin and Cos F-stat*'
				WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=0
			sWidget.StripchartTypeOutput = 22
		endelse
	END

	sWidget.wCCombocoefButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the image acquisition signal, change the appropriate variables,
			; make interface reflect change
			if (sWidget.StripchartTypeOutput eq 23) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCCombocoefButton, SET_VALUE='Sin and Cos Magnitude'
			; The user wants to change previously selected option to make the stripchart
			; display show the image acquisition signal, change the appropriate variables,
			; make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 23
				WIDGET_CONTROL, sWidget.wCCombocoefButton, SET_VALUE='*Sin and Cos Magnitude*'
				WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=0
			endelse
		; The user wants the stripchart display to show the image acquisition signal, change 
		; the appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCCombocoefButton, SET_VALUE='*Sin and Cos Magnitude*'
				WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=0
			sWidget.StripchartTypeOutput = 23
		endelse
	END

	sWidget.wCSOSCardButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the sum of squares of the cardiac signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.StripchartTypeOutput eq 5) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCSOSCardButton, SET_VALUE='Cardiac'
			; The user wants to change previously selected option to make the stripchart  
			; display show the sum of squares of the cardiac signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 5
				WIDGET_CONTROL, sWidget.wCSOSCardButton, SET_VALUE='*Cardiac*'
			endelse
		; The user wants the stripchart display to show the sum of squares of the 
		; cardiac signal, change the appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCSOSCardButton, SET_VALUE='*Cardiac*'
			sWidget.StripchartTypeOutput = 5
		endelse
	END

	sWidget.wCSOSRespButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the sum of squares of the respiratory signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.StripchartTypeOutput eq 6) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCSOSRespButton, SET_VALUE='Respiratory'
			; The user wants to change previously selected option to make the stripchart  
			; display show the sum of squares of the respiratory signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 6
				WIDGET_CONTROL, sWidget.wCSOSRespButton, SET_VALUE='*Respiratory*'
			endelse
		; The user wants the stripchart display to show the sum of squares
		; of the respiratory signal, change the appropriate variables,
		; make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCSOSRespButton, SET_VALUE='*Respiratory*'
			sWidget.StripchartTypeOutput = 6
		endelse
	END

	sWidget.wCSOSStimButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the sum of squares of the stimulus signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.StripchartTypeOutput eq 7) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCSOSStimButton, SET_VALUE='Stimulus'
			; The user wants to change previously selected option to make the stripchart  
			; display show the sum of squares of the stimulus signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 7
				WIDGET_CONTROL, sWidget.wCSOSStimButton, SET_VALUE='*Stimulus*'
			endelse
		; The user wants the stripchart display to show the sum of squares of the stimulus 
		; signal, change the appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCSOSStimButton, SET_VALUE='*Stimulus*'
			sWidget.StripchartTypeOutput = 7
		endelse
	END

	sWidget.wCRegCoefCardButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the regression coefficient of the cardiac signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.StripchartTypeOutput eq 8) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCRegCoefCardButton, SET_VALUE='Cardiac'
			; The user wants to change previously selected option to make the stripchart  
			; display show the regression coefficient of the cardiac signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 8
				WIDGET_CONTROL, sWidget.wCRegCoefCardButton, SET_VALUE='*Cardiac*'
			endelse
		; The user wants the stripchart display to show the regression coefficient
		; of the cardiac signal, change the appropriate variables,
		; make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCRegCoefCardButton, SET_VALUE='*Cardiac*'
			sWidget.StripchartTypeOutput = 8
		endelse
	END

	sWidget.wCRegCoefRespButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the regression coefficient of the respiratory signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.StripchartTypeOutput eq 9) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCRegCoefRespButton, SET_VALUE='Respiratory'
			; The user wants to change previously selected option to make the stripchart  
			; display show the regression coefficient of the respiratory signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 9
				WIDGET_CONTROL, sWidget.wCRegCoefRespButton, SET_VALUE='*Respiratory*'
			endelse
		; The user wants the stripchart display to show the regression coefficient
		; of the respiratory signal, change the appropriate variables,
		; make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCRegCoefRespButton, SET_VALUE='*Respiratory*'
			sWidget.StripchartTypeOutput = 9
		endelse
	END

	sWidget.wCRegCoefStimButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the regression coefficient of the stimulus signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.StripchartTypeOutput eq 10) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCRegCoefStimButton, SET_VALUE='Stimulus'
			; The user wants to change previously selected option to make the stripchart  
			; display show the regression coefficient of the stimulus signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 10
				WIDGET_CONTROL, sWidget.wCRegCoefStimButton, SET_VALUE='*Stimulus*'
			endelse
		; The user wants the stripchart display to show the regression coefficient
		; of the stimulus signal, change the appropriate variables,
		; make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCRegCoefStimButton, SET_VALUE='*Stimulus*'
			sWidget.StripchartTypeOutput = 10
		endelse
	END

	sWidget.wCEffectCardButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the effect size of the cardiac signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.StripchartTypeOutput eq 11) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCEffectCardButton, SET_VALUE='Cardiac'
			; The user wants to change previously selected option to make the stripchart  
			; display show the effect size of the cardiac signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 11
				WIDGET_CONTROL, sWidget.wCEffectCardButton, SET_VALUE='*Cardiac*'
			endelse
		; The user wants the stripchart display to show the effect size
		; of the cardiac signal, change the appropriate variables,
		; make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCEffectCardButton, SET_VALUE='*Cardiac*'
			sWidget.StripchartTypeOutput = 11
		endelse
	END

	sWidget.wCEffectRespButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the effect size of the respiratory signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.StripchartTypeOutput eq 12) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCEffectRespButton, SET_VALUE='Respiratory'
			; The user wants to change previously selected option to make the stripchart  
			; display show the effect size of the respiratory signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 12
				WIDGET_CONTROL, sWidget.wCEffectRespButton, SET_VALUE='*Respiratory*'
			endelse
		; The user wants the stripchart display to show the effect size
		; of the respiratory signal, change the appropriate variables,
		; make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCEffectRespButton, SET_VALUE='*Respiratory*'
			sWidget.StripchartTypeOutput = 12
		endelse
	END

	sWidget.wCEffectStimButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the effect size of the stimulus signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.StripchartTypeOutput eq 13) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCEffectStimButton, SET_VALUE='Stimulus'
			; The user wants to change previously selected option to make the stripchart  
			; display show the effect size of the stimulus signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 13
				WIDGET_CONTROL, sWidget.wCEffectStimButton, SET_VALUE='*Stimulus*'
			endelse
		; The user wants the stripchart display to show the effect size
		; of the stimulus signal, change the appropriate variables,
		; make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCEffectStimButton, SET_VALUE='*Stimulus*'
			sWidget.StripchartTypeOutput = 13
		endelse
	END

	sWidget.wCFstatCardButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the F-statistic of the cardiac signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.StripchartTypeOutput eq 17) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCFstatCardButton, SET_VALUE='Cardiac'
			; The user wants to change previously selected option to make the stripchart  
			; display show the F-statistic of the cardiac signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 17
				WIDGET_CONTROL, sWidget.wCFstatCardButton, SET_VALUE='*Cardiac*'
			endelse
		; The user wants the stripchart display to show the F-statistic
		; of the cardiac signal, change the appropriate variables,
		; make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCFstatCardButton, SET_VALUE='*Cardiac*'
			sWidget.StripchartTypeOutput = 17
		endelse
	END

	sWidget.wCFstatRespButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the F-statistic of the respiratory signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.StripchartTypeOutput eq 18) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCFstatRespButton, SET_VALUE='Respiratory'
			; The user wants to change previously selected option to make the stripchart  
			; display show the F-statistic of the respiratory signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 18
				WIDGET_CONTROL, sWidget.wCFstatRespButton, SET_VALUE='*Respiratory*'
			endelse
		; The user wants the stripchart display to show the F-statistic
		; of the respiratory signal, change the appropriate variables,
		; make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCFstatRespButton, SET_VALUE='*Respiratory*'
			sWidget.StripchartTypeOutput = 18
		endelse
	END

	sWidget.wCFstatStimButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the F-statistic of the stimulus signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.StripchartTypeOutput eq 19) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCFstatStimButton, SET_VALUE='Stimulus'
			; The user wants to change previously selected option to make the stripchart  
			; display show the F-statistic of the stimulus signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 19
				WIDGET_CONTROL, sWidget.wCFstatStimButton, SET_VALUE='*Stimulus*'
			endelse
		; The user wants the stripchart display to show the F-statistic
		; of the stimulus signal, change the appropriate variables,
		; make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCFstatStimButton, SET_VALUE='*Stimulus*'
			sWidget.StripchartTypeOutput = 19
		endelse
	END

	sWidget.wCTstatCardButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the t-statistic of the cardiac signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.StripchartTypeOutput eq 14) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCTstatCardButton, SET_VALUE='Cardiac'
			; The user wants to change previously selected option to make the stripchart  
			; display show the t-statistic of the cardiac signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 14
				WIDGET_CONTROL, sWidget.wCTstatCardButton, SET_VALUE='*Cardiac*'
			endelse
		; The user wants the stripchart display to show the t-statistic
		; of the cardiac signal, change the appropriate variables,
		; make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCTstatCardButton, SET_VALUE='*Cardiac*'
			sWidget.StripchartTypeOutput = 14
		endelse
	END

	sWidget.wCTstatRespButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the t-statistic of the respiratory signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.StripchartTypeOutput eq 15) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCTstatRespButton, SET_VALUE='Respiratory'
			; The user wants to change previously selected option to make the stripchart  
			; display show the t-statistic of the respiratory signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 15
				WIDGET_CONTROL, sWidget.wCTstatRespButton, SET_VALUE='*Respiratory*'
			endelse
		; The user wants the stripchart display to show the t-statistic
		; of the respiratory signal, change the appropriate variables,
		; make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCTstatRespButton, SET_VALUE='*Respiratory*'
			sWidget.StripchartTypeOutput = 15
		endelse
	END

	sWidget.wCTstatStimButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the t-statistic of the stimulus signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.StripchartTypeOutput eq 16) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCTstatStimButton, SET_VALUE='Stimulus'
			; The user wants to change previously selected option to make the stripchart  
			; display show the t-statistic of the stimulus signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 16
				WIDGET_CONTROL, sWidget.wCTstatStimButton, SET_VALUE='*Stimulus*'
			endelse
		; The user wants the stripchart display to show the t-statistic of the stimulus
		; signal, change the appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCTstatStimButton, SET_VALUE='*Stimulus*'
			sWidget.StripchartTypeOutput = 16
		endelse
	END

	sWidget.wCErrorCoefCardButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the t-statistic of the cardiac signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.StripchartTypeOutput eq 24) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCErrorCoefCardButton, SET_VALUE='Cardiac'
				WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=0
			; The user wants to change previously selected option to make the stripchart  
			; display show the t-statistic of the cardiac signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 24
				WIDGET_CONTROL, sWidget.wCErrorCoefCardButton, SET_VALUE='*Cardiac*'
				WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=1
				WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=1
			endelse
		; The user wants the stripchart display to show the t-statistic
		; of the cardiac signal, change the appropriate variables,
		; make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCErrorCoefCardButton, SET_VALUE='*Cardiac*'
				WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=1
				WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=1
			sWidget.StripchartTypeOutput = 24
		endelse
	END

	sWidget.wCErrorCoefRespButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the t-statistic of the respiratory signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.StripchartTypeOutput eq 25) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCErrorCoefRespButton, SET_VALUE='Respiratory'
				WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=0
			; The user wants to change previously selected option to make the stripchart  
			; display show the t-statistic of the respiratory signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 25
				WIDGET_CONTROL, sWidget.wCErrorCoefRespButton, SET_VALUE='*Respiratory*'
				WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=1
				WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=1
			endelse
		; The user wants the stripchart display to show the t-statistic
		; of the respiratory signal, change the appropriate variables,
		; make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCErrorCoefRespButton, SET_VALUE='*Respiratory*'
				WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=1
				WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=1
			sWidget.StripchartTypeOutput = 25
		endelse
	END

	sWidget.wCErrorCoefStimButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the t-statistic of the stimulus signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.StripchartTypeOutput eq 26) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCErrorCoefStimButton, SET_VALUE='Stimulus'
				WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=0
				WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=0
			; The user wants to change previously selected option to make the stripchart  
			; display show the t-statistic of the stimulus signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 26
				WIDGET_CONTROL, sWidget.wCErrorCoefStimButton, SET_VALUE='*Stimulus*'
				WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=1
				WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=1
			endelse
		; The user wants the stripchart display to show the t-statistic of the stimulus
		; signal, change the appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCErrorCoefStimButton, SET_VALUE='*Stimulus*'
				WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=1
				WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=1
			sWidget.StripchartTypeOutput = 26
		endelse
	END

	sWidget.wCErrorTstatCardButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the t-statistic of the cardiac signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.StripchartTypeOutput eq 27) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCErrorTstatCardButton, SET_VALUE='Cardiac'
			; The user wants to change previously selected option to make the stripchart  
			; display show the t-statistic of the cardiac signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 27
				WIDGET_CONTROL, sWidget.wCErrorTstatCardButton, SET_VALUE='*Cardiac*'
			endelse
		; The user wants the stripchart display to show the t-statistic
		; of the cardiac signal, change the appropriate variables,
		; make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCErrorTstatCardButton, SET_VALUE='*Cardiac*'
			sWidget.StripchartTypeOutput = 27
		endelse
	END

	sWidget.wCErrorTstatRespButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the t-statistic of the respiratory signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.StripchartTypeOutput eq 28) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCErrorTstatRespButton, SET_VALUE='Respiratory'
			; The user wants to change previously selected option to make the stripchart  
			; display show the t-statistic of the respiratory signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 28
				WIDGET_CONTROL, sWidget.wCErrorTstatRespButton, SET_VALUE='*Respiratory*'
			endelse
		; The user wants the stripchart display to show the t-statistic
		; of the respiratory signal, change the appropriate variables,
		; make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCErrorTstatRespButton, SET_VALUE='*Respiratory*'
			sWidget.StripchartTypeOutput = 28
		endelse
	END

	sWidget.wCErrorTstatStimButton : begin
		if (sWidget.StripchartTypeOutput gt 0) then begin
			; The user wants to eliminate their choice of having the stripchart display 
			; show the t-statistic of the stimulus signal, change the  
			; appropriate variables, make interface reflect change
			if (sWidget.StripchartTypeOutput eq 29) then begin
				sWidget.StripchartTypeOutput = -1
				WIDGET_CONTROL, sWidget.wCErrorTstatStimButton, SET_VALUE='Stimulus'
			; The user wants to change previously selected option to make the stripchart  
			; display show the t-statistic of the stimulus signal, change the 
			; appropriate variables, make interface reflect change
			endif else begin
				ChartReset, sWidget
				sWidget.StripchartTypeOutput = 29
				WIDGET_CONTROL, sWidget.wCErrorTstatStimButton, SET_VALUE='*Stimulus*'
			endelse
		; The user wants the stripchart display to show the t-statistic of the stimulus
		; signal, change the appropriate variables, make interface reflect change
		endif else begin
			WIDGET_CONTROL, sWidget.wCErrorTstatStimButton, SET_VALUE='*Stimulus*'
			sWidget.StripchartTypeOutput = 29
		endelse
	END

	sWidget.wOutputFileList : begin
		; User inputs selects whether the output of the regression as chosen previously will
		; be placed in a cvio file or a "regular" disk file
		currentOutputFileList = WIDGET_INFO(sWidget.wOutputFileList, /DropList_Select)
		WIDGET_CONTROL, sWidget.wOutputFileList, GET_UVALUE=outputFileListValues
		sWidget.OutputFileType = outputFileListValues[currentOutputFileList]
	END

	sWidget.wSaveButton : begin
		; If user has yet to enter output file name or input file name, then cannot proceed
		if ((sWidget.RunNum EQ '') or (sWidget.OutputFileType eq '')) then begin
			WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Must Enter Output File' + $
			' Type and Run Number First'
		endif else begin
			; Determine whether the output will be saved or not saved
			if (sWidget.Save eq -1) then begin
				sWidget.Save=1
				WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Output will be saved'
			endif else begin
				sWidget.Save=-1
				WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Output will not be saved'
			endelse
		endelse
	END

	sWidget.wConIntervalText : begin
		; Get the name of the file containing the image on which the ROI will be selected
		WIDGET_CONTROL, sWidget.wConIntervalText, GET_VALUE=temp
		sWidget.ConfidenceInterval = temp
	END

	sWidget.wOverlayText : begin
		; Get the name of the file containing the image on which the ROI will be selected
		WIDGET_CONTROL, sWidget.wOverlayText, GET_VALUE=temp
		sWidget.Overlay = temp
	END

	sWidget.wROITypeList : begin
		; User input selects whether ROI will be arbitrary or box shape
		currentROITypeList = WIDGET_INFO(sWidget.wROITypeList, /DropList_Select)
		WIDGET_CONTROL, sWidget.wROITypeList, GET_UVALUE=ROITypeValues
		sWidget.ROIType = ROITypeValues[currentROITypeList]
		resetROIStuff,sWidget
		if (sWidget.ROIType eq 'User Defined - Arbitrary') then begin
			WIDGET_CONTROL, sWidget.wCompROI1Label, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wCompROI1Text, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROISliceText, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROISliceLabel, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wOptionsList, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROIButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROILabel, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROISliceText, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROISliceLabel, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wNoROIButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wNoROILabel, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wExpandROIText, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wExpandROILabel, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wUpdateText,SET_VALUE='Click and hold left button' + $
				' to draw. Click right button when done.'
		endif else if (sWidget.ROIType eq 'User Defined - Box') then begin
			WIDGET_CONTROL, sWidget.wCompROI1Label, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wCompROI1Text, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROISliceLabel, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROISliceText, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wOptionsList, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROIButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROILabel, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROISliceText, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROISliceLabel, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wNoROIButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wNoROILabel, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wExpandROIText, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wExpandROILabel, SENSITIVE=1
		endif else if (sWidget.ROIType eq 'Threshold - One Run') then begin
			WIDGET_CONTROL, sWidget.wCompROI1Text, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wCompROI1Label, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROISliceText, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROISliceLabel, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wOptionsList, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROIButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROILabel, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wNoROIButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wNoROILabel, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROISliceText, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROISliceLabel, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wPositionList, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wCutoffText, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wCutoffLabel, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wExpandROIText, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wExpandROILabel, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wUpdateText,SET_VALUE='Enter run number, ' + $
				'slice, file type, rel. position, and cutoff.'
		endif else begin
			WIDGET_CONTROL, sWidget.wCompROI1Text, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wCompROI1Label, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROISliceText, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROISliceLabel, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wOptionsList, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROISliceText, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROISliceLabel, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wNoROIButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wNoROILabel, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROIButton, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wROILabel, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wPositionList, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wCutoffText, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wCutoffLabel, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wCompROI2Text, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wCompROI2Label, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wExpandROIText, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wExpandROILabel, SENSITIVE=1
			WIDGET_CONTROL, sWidget.wUpdateText,SET_VALUE='Enter both run numbers, ' + $
				'slice, file type, rel. position, and cutoff.'
		end			
	END

	sWidget.wCompROI1Text : begin
		; Get the run number for the output which will be used to determine the ROI
		WIDGET_CONTROL, sWidget.wCompROI1Text, GET_VALUE=temp
		sWidget.CompROI1 = temp
		if (strlen(sWidget.CompROI1) eq 1) then begin
			sWidget.CompROI1='0'+temp
		endif
	end

	sWidget.wCompROI2Text : begin
		; Get the run number for the output which will be used to determine the ROI
		WIDGET_CONTROL, sWidget.wCompROI2Text, GET_VALUE=temp
		sWidget.CompROI2 = temp
		if (strlen(sWidget.CompROI2) eq 1) then begin
			sWidget.CompROI2='0'+temp
		endif
	end

	sWidget.wOptionsList : begin
		; User enters the type of patient feedback desired, set appropriate variables
		currentOptionsList = WIDGET_INFO(sWidget.wOptionsList, /DropList_Select)
		WIDGET_CONTROL, sWidget.wOptionsList, GET_UVALUE = OptionsListValues
		sWidget.OptionsType = OptionsListValues[currentOptionsList]
	END
	
	sWidget.wROISliceText : begin
		; Get the slice of the image on which the ROI will be selected
		WIDGET_CONTROL, sWidget.wROISliceText, GET_VALUE=temp
		sWidget.ROISlice = temp
	END

	sWidget.wPositionList : begin
		; User enters the type of patient feedback desired, set appropriate variables
		currentPositionList = WIDGET_INFO(sWidget.wPositionList, /DropList_Select)
		WIDGET_CONTROL, sWidget.wPositionList, GET_UVALUE = PositionListValues
		sWidget.PositionType = PositionListValues[currentPositionList]
	END

	sWidget.wCutoffText : begin
		; Get the cutoff point to determine the ROI
		WIDGET_CONTROL, sWidget.wCutoffText, GET_VALUE=temp
		sWidget.CompROICut = temp
	end
		
	sWidget.wROIButton : begin
		device, pseudo=8
		if (sWidget.Mask eq -1) then begin
			sWidget.Mask = 1
		endif
		proceed = -1
		if ((sWidget.ROIType eq 'User Defined - Arbitrary') or $
			(sWidget.ROIType eq 'User Defined - Box')) then begin
			if ((sWidget.OptionsType eq '') and (proceed eq -1)) then begin
				WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter ROI File Type'	
				proceed = 1
			endif else if ((sWidget.CompROI1 eq '') and (proceed eq -1)) then begin
				WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter ROI Run Number'
				proceed = 1
			endif else if ((sWidget.ROISlice le 0) and (proceed eq -1)) then begin
				WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter ROI Slice'
				proceed = 1
			endif else begin
				; do nothing
			endelse

			if (proceed eq -1) then begin
				; Open the cvio file containing the image
				if (sWidget.OptionsType eq 'Raw Image') then begin
					open_file1='fmri'+sWidget.CompROI1+'_tag_raw.cvio'
				endif else if (sWidget.OptionsType eq 'Sum of Squares') then begin
					open_file1='fmri'+sWidget.CompROI1+'_ss_raw.cvio.slice'+sWidget.ROISlice
				endif else if (sWidget.OptionsType eq 'Regression Coefficient') then begin
					open_file1='fmri'+sWidget.CompROI1+'_coef_raw.cvio.slice'+sWidget.ROISlice
				endif else if (sWidget.OptionsType eq 'Effect Size') then begin
					open_file1='fmri'+sWidget.CompROI1+'_effect_raw.cvio.slice'+sWidget.ROISlice
				endif else if (sWidget.OptionsType eq 't-Statistic') then begin
					open_file1='fmri'+sWidget.CompROI1+'_tstat_raw.cvio.slice'+sWidget.ROISlice
				endif else if (sWidget.OptionsType eq 'F-Statistic') then begin
					open_file1='fmri'+sWidget.CompROI1+'_fstat_raw.cvio.slice'+sWidget.ROISlice
				endif else if (sWidget.OptionsType eq 'Phase Lag Sin and Cos') then begin
					open_file1='fmri'+sWidget.CompROI1+'_lagsincos_raw.cvio.slice'+ $
						sWidget.ROISlice
				endif else if (sWidget.OptionsType eq 'Sin and Cos F-Statistic') then begin
					open_file1='fmri'+sWidget.CompROI1+'_sincosfstat_raw.cvio.slice'+ $
						sWidget.ROISlice
				endif else begin
					open_file1='fmri'+sWidget.CompROI1+'_magsincos_raw.cvio.slice'+ $
						sWidget.ROISlice
				endelse
				sWidget.ROIFile = open_file1

				junk = -1
				if (sWidget.OptionsType eq 'Raw Image') then begin
					err=cvio_open(sWidget.cvioData_dir+sWidget.ROIFile,1,roiFile)
					if (err ne 0) then begin
						WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='File Does Not Exist'
						junk = 1
					endif
		
					if (junk eq -1) then begin
						; Calculate distance into file must travel and retrieve specified image
						err=cvio_read_next_samples(roiFile,store_t,store_d,sWidget.ROISlice)
						slice1 = store_d(*,*,sWidget.ROISlice-1)
				
	  					; Scale images into grayscale and increase size
						slice1 = reverse(slice1,2)
	  					big_slice = REBIN(slice1, sWidget.disp_sizex, sWidget.disp_sizey)
						; Display the image
	  					wset, sWidget.orig_win_id
						tv,bytscl(big_slice)/2
					endif
				endif else begin
					err=cvio_open(sWidget.output_dir+sWidget.ROIFile,1,roiFile)
					if (err ne 0) then begin
						WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='File Does Not Exist'
						junk = 1
					endif
	
					if (junk eq -1) then begin
						; Calculate distance into file must travel and retrieve specified image
						err=cvio_read_next_samples(roiFile,store_t,store_d, $
							sWidget.NumImg/sWidget.NumSli)
						slice1 = store_d(*,*,sWidget.NumImg/sWidget.NumSli-1)
		
						; Scale images into grayscale and increase size
						slice1 = reverse(slice1,2)
		  				big_slice = REBIN(slice1, sWidget.disp_sizex, sWidget.disp_sizey)
						number=max(abs(big_slice))
						hold=bytscl(big_slice,-number,number)/2+128
						; Display the image
		  				wset, sWidget.result_win_id
						tv,hold
					endif
				endelse
	
				if (junk eq -1) then begin
					; Close the cvio file
	  				err=cvio_close(roiFile)
	
					; Call program allowing user to draw box indicating ROI on specified image
					; and store results in appropriate variables
					if (sWidget.ROIType eq 'User Defined - Box') then begin
						movbox,hold1,hold2,hold3,hold4,color=255
						sWidget.ROIx=hold1/(sWidget.disp_sizex/sWidget.orig_sizex)
						sWidget.ROIy=hold2/(sWidget.disp_sizex/sWidget.orig_sizex)
						sWidget.ROIdx=hold3/(sWidget.disp_sizex/sWidget.orig_sizex)
						sWidget.ROIdy=hold4/(sWidget.disp_sizex/sWidget.orig_sizex)
					; Call program allowing user to draw arbritrary ROI on specified image and 
					; store results in appropriate variables
					endif else begin
						sWidget.Store_ROI(*)=0
						drawpoly,x,y,/CURVE,/NOPLOT,COLOR=64,THICK=2
						sWidget.Store_ROI = polyfillv(x/(sWidget.disp_sizex/sWidget.orig_sizex), $
							(sWidget.orig_sizey-1)-y/(sWidget.disp_sizey/sWidget.orig_sizex), $
							sWidget.orig_sizex,sWidget.orig_sizey)
						print, n_elements(where(sWidget.Store_ROI gt 0))
					endelse
				endif
			endif
		endif else begin
			if (sWidget.ROIType eq 'Threshold - Two Runs') then begin
				; If user has not entered file name, image number, or slice number cannot display image
				if ((sWidget.OptionsType eq '') and (proceed eq -1)) then begin
					WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter ROI File Type'	
					proceed = 1
				endif else if ((sWidget.CompROI1 eq '') and (proceed eq -1)) then begin
					WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter ROI Run Number'
					proceed = 1
				endif else if ((sWidget.CompROI2 eq '') and (proceed eq -1)) then begin
					WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter Both ROI Run Numbers'
					proceed = 1
				endif else if ((sWidget.ROISlice le 0) and (proceed eq -1)) then begin
					WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter ROI Slice'
					proceed = 1
				endif else if ((sWidget.CompROICut eq -1) and (proceed eq -1)) then begin
					WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter ROI Threshold Cutoff'
					proceed = 1
				endif else if ((sWidget.PositionType eq '') and (proceed eq -1)) then begin
					WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter ROI Threshold Position'
					proceed = 1
				endif else begin
					; do nothing
				endelse
				if (proceed eq -1) then begin
					; If two run numbers are entered want to determine common areas of activation greater than
					; the cuttoff to create ROI
					; Determine the name of the output files for the runs based on the contents of the run
					; summary file
					if (sWidget.OptionsType eq 'Sum of Squares') then begin
						open_file1='fmri'+sWidget.CompROI1+'_ss_raw.cvio.slice'+sWidget.ROISlice
					endif else if (sWidget.OptionsType eq 'Regression Coefficient') then begin
						open_file1='fmri'+sWidget.CompROI1+'_coef_raw.cvio.slice'+sWidget.ROISlice
					endif else if (sWidget.OptionsType eq 'Effect Size') then begin
						open_file1='fmri'+sWidget.CompROI1+'_effect_raw.cvio.slice'+sWidget.ROISlice
					endif else if (sWidget.OptionsType eq 't-Statistic') then begin
						open_file1='fmri'+sWidget.CompROI1+'_tstat_raw.cvio.slice'+sWidget.ROISlice
					endif else if (sWidget.OptionsType eq 'F-Statistic') then begin
						open_file1='fmri'+sWidget.CompROI1+'_fstat_raw.cvio.slice'+sWidget.ROISlice
					endif else if (sWidget.OptionsType eq 'Phase Lag Sin and Cos') then begin
						open_file1='fmri'+sWidget.CompROI1+'_lagsincos_raw.cvio.slice'+ $
							sWidget.ROISlice
					endif else if (sWidget.OptionsType eq 'Sin and Cos F-Statistic') then begin
						open_file1='fmri'+sWidget.CompROI1+'_sincosfstat_raw.cvio.slice'+ $
							sWidget.ROISlice
					endif else if (sWidget.OptionsType eq 'Raw Image') then begin
						WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='File type unavailable. ' + $
							'Please choose another.'
					endif else begin
						open_file1='fmri'+sWidget.CompROI1+'_magsincos_raw.cvio.slice'+ $
							sWidget.ROISlice
					endelse
	
					if (sWidget.OptionsType eq 'Sum of Squares') then begin
						open_file2='fmri'+sWidget.CompROI2+'_ss_raw.cvio.slice'+sWidget.ROISlice
					endif else if (sWidget.OptionsType eq 'Regression Coefficient') then begin
						open_file2='fmri'+sWidget.CompROI2+'_coef_raw.cvio.slice'+sWidget.ROISlice
					endif else if (sWidget.OptionsType eq 'Effect Size') then begin
						open_file2='fmri'+sWidget.CompROI2+'_effect_raw.cvio.slice'+sWidget.ROISlice
					endif else if (sWidget.OptionsType eq 't-Statistic') then begin
						open_file2='fmri'+sWidget.CompROI2+'_tstat_raw.cvio.slice'+sWidget.ROISlice
					endif else if (sWidget.OptionsType eq 'F-Statistic') then begin
						open_file2='fmri'+sWidget.CompROI2+'_fstat_raw.cvio.slice'+sWidget.ROISlice
					endif else if (sWidget.OptionsType eq 'Phase Lag Sin and Cos') then begin
						open_file2='fmri'+sWidget.CompROI2+'_lagsincos_raw.cvio.slice'+ $
							sWidget.ROISlice
					endif else if (sWidget.OptionsType eq 'Sin and Cos F-Statistic') then begin
						open_file2='fmri'+sWidget.CompROI2+'_sincosfstat_raw.cvio.slice'+ $
							sWidget.ROISlice
					endif else if (sWidget.OptionsType eq 'Raw Image') then begin
						WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='File type unavailable. ' + $
							'Please choose another.'
					endif else begin
						open_file2='fmri'+sWidget.CompROI2+'_magsincos_raw.cvio.slice'+ $
							sWidget.ROISlice
					endelse
					sWidget.ROIFile = open_file1
			
					junk = -1
					; Open the corresponding output files based on determined names
					err=cvio_open(sWidget.output_dir+open_file1,1,comp1)
					if (err ne 0) then begin
						WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='File 1 Does Not Exist'
						junk = 1
					endif
					err=cvio_open(sWidget.output_dir+open_file2,1,comp2)
					if (err ne 0) then begin
						WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='File 2 Does Not Exist'
						junk = 1
					endif
		
					if (junk eq -1) then begin
						; Determine the number of images so can move to last one
						err=CVIO_GETATTRIBUTE(comp1,'REQ_ATTR_NSAMP_UL',num_samp1)
						if (err ne 0) then print, "get attribute problem 1"
						err=CVIO_GETATTRIBUTE(comp2,'REQ_ATTR_NSAMP_UL',num_samp2)
						if (err ne 0) then print, "get attribute problem 2"
		
						err=cvio_read_next_samples(comp1,junk_time1,result_data1,fix(num_samp1))
						err=cvio_read_next_samples(comp2,junk_time2,result_data2,fix(num_samp2))
				
						err=cvio_close(comp1)
						err=cvio_close(comp2)
		
						; Determine where in each output file value is greater than the cutoff
						if (sWidget.PositionType eq 'Above') then begin
							print,"above ",sWidget.CompROICut
							valid1=where(result_data1(*,*,num_samp1-1) gt sWidget.CompROICut)
							valid2=where(result_data2(*,*,num_samp2-1) gt sWidget.CompROICut)
						endif else begin
							print,"below ",sWidget.CompROICut
							valid1=where(result_data1(*,*,num_samp1-1) lt sWidget.CompROICut)
							valid2=where(result_data2(*,*,num_samp2-1) lt sWidget.CompROICut)
						endelse
		
						; Determine where the two areas overlap
						if (n_elements(valid1) lt n_elements(valid2)) then begin
							output = lonarr(n_elements(valid1))-1
							counter=0
							for b=0,n_elements(valid1)-1 do begin
								works_both=where(valid1(b) eq valid2)
								if (works_both(0) ne -1) then begin
									output(counter) = valid2(works_both)
									counter = counter + 1
								endif
							endfor
						endif else begin
							output = lonarr(n_elements(valid2))-1
							counter=0
							for b=0,n_elements(valid2)-1 do begin
								works_both=where(valid2(b) eq valid1)
								if (works_both(0) ne -1) then begin
									output(counter) = valid1(works_both)
									counter = counter + 1
								endif
							endfor
						endelse
	
						; Create ROI array indicating results
						first = where(output eq -1)
						if (first(0) eq -1) then begin
							; matches at every point
							sWidget.Comp_Store_ROI=output
						endif else if (first(0) eq 0) then begin
							; don't do anything then doesn't match anywhere
						endif else begin
							sWidget.Comp_Store_ROI=output(0:first(0)-1)
						endelse
			
						print, n_elements(where(sWidget.Comp_Store_ROI gt 0))
					endif
				endif
			endif else begin
				; If user has not entered file name, image number, or slice number cannot display image
				if ((sWidget.OptionsType eq '') and (proceed eq -1)) then begin
					WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter ROI File Type'	
					proceed = 1
				endif else if ((sWidget.CompROI1 eq '') and (proceed eq -1)) then begin
					WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter ROI Run Number'
					proceed = 1
				endif else if ((sWidget.ROISlice le 0) and (proceed eq -1)) then begin
					WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter ROI Slice'
					proceed = 1
				endif else if ((sWidget.CompROICut eq -1) and (proceed eq -1)) then begin
					WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter ROI Threshold Cutoff'
					proceed = 1
				endif else if ((sWidget.PositionType eq '') and (proceed eq -1)) then begin
					WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter ROI Threshold Position'
					proceed = 1
				endif else begin
					; do nothing
				endelse
				; Determine the name of the output files for the runs based on the contents of the run
				; summary file
				if (proceed eq -1) then begin
					if (sWidget.OptionsType eq 'Sum of Squares') then begin
						open_file1='fmri'+sWidget.CompROI1+'_ss_raw.cvio.slice'+sWidget.ROISlice
					endif else if (sWidget.OptionsType eq 'Regression Coefficient') then begin
						open_file1='fmri'+sWidget.CompROI1+'_coef_raw.cvio.slice'+ $
							sWidget.ROISlice
					endif else if (sWidget.OptionsType eq 'Effect Size') then begin
						open_file1='fmri'+sWidget.CompROI1+'_effect_raw.cvio.slice'+ $
							sWidget.ROISlice
					endif else if (sWidget.OptionsType eq 't-Statistic') then begin
						open_file1='fmri'+sWidget.CompROI1+'_tstat_raw.cvio.slice'+ $
							sWidget.ROISlice
					endif else if (sWidget.OptionsType eq 'F-Statistic') then begin
						open_file1='fmri'+sWidget.CompROI1+'_fstat_raw.cvio.slice'+ $
							strtrim(sWidget.GetPicture,2)
					endif else if (sWidget.OptionsType eq 'Phase Lag Sin and Cos') then begin
						open_file1='fmri'+sWidget.CompROI1+'_lagsincos_raw.cvio.slice'+ $
							sWidget.ROISlice
					endif else if (sWidget.OptionsType eq 'Sin and Cos F-Statistic') then begin
						open_file1='fmri'+sWidget.CompROI1+'_sincosfstat_raw.cvio.slice'+ $
							sWidget.ROISlice
					endif else begin
						open_file1='fmri'+sWidget.CompROI1+'_magsincos_raw.cvio.slice'+ $
							sWidget.ROISlice
					endelse
					sWidget.ROIFile = open_file1

					junk = -1
					; Open the corresponding output files based on determined names
					err=cvio_open(sWidget.output_dir+open_file1,1,comp1)
					if (err ne 0) then begin
						WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='File Does Not Exist'
						junk = 1
					endif

					if (junk eq -1) then begin
						; Determine the number of images so can move to last one
						err=CVIO_GETATTRIBUTE(comp1,'REQ_ATTR_NSAMP_UL',num_samp1)
						if (err ne 0) then print, "get attribute err = ",fix(num_samp1)
	
						; Read in the desired number of samples
						err=cvio_read_next_samples(comp1,junk_time1,result_data1,fix(num_samp1))
						err=cvio_close(comp1)
		
						; Determine where in each output file value is greater than the cutoff
						; Create ROI based on results
						if (sWidget.PositionType eq 'Above') then begin
							print,"above ",sWidget.CompROICut
							sWidget.Comp_Store_ROI=where(result_data1(*,*,num_samp1-1) gt sWidget.CompROICut)
						endif else begin
							print,"below ",sWidget.CompROICut
							sWidget.Comp_Store_ROI=where(result_data1(*,*,num_samp1-1) lt sWidget.CompROICut)
						endelse
			
						print, n_elements(where(sWidget.Comp_Store_ROI gt 0))
					endif
				endif
			endelse
		endelse
	end

	sWidget.wExpandROIText : begin
		; User has indicated desire to expand ROI by specified number of pixels
		WIDGET_CONTROL, sWidget.wExpandROIText, GET_VALUE=temp
		sWidget.Expand=temp
		; Update the user on status
		WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='ROI Dilation Factor' + $
			' Successfully Entered'
	end

	sWidget.wNoROIButton : begin
		; User has indicated desire to remove existing ROI from calculations
		if (sWidget.Mask eq 1) then begin
			sWidget.Mask = -1
			; Reset ROI variable values
			sWidget.Comp_Store_ROI(*) = 0
			sWidget.Store_ROI(*) = 0
		    sWidget.ROIx = 0
		    sWidget.ROIy = 0
		    sWidget.ROIdx = sWidget.orig_sizex
		    sWidget.ROIdy = sWidget.orig_sizey	
		endif
	end

	sWidget.wScaleMaxText : begin
		; Get the base, or first, number from the user for real time acquisition indicating
		; the name of the first image to be collected/regressed
		WIDGET_CONTROL, sWidget.wScaleMaxText, GET_VALUE=temp
		sWidget.ScaleMax = temp
	end

	sWidget.wXDimensionText : begin
		; Get the base, or first, number from the user for real time acquisition indicating
		; the name of the first image to be collected/regressed
		WIDGET_CONTROL, sWidget.wXDimensionText, GET_VALUE=temp
		sWidget.orig_sizex = temp
		sWidget.ROIdx = sWidget.orig_sizex
	end

	sWidget.wYDimensionText : begin
		; Get the base, or first, number from the user for real time acquisition indicating
		; the name of the first image to be collected/regressed
		WIDGET_CONTROL, sWidget.wYDimensionText, GET_VALUE=temp
		sWidget.orig_sizey = temp
		sWidget.ROIdy = sWidget.orig_sizey
	end

	sWidget.wOutputDirText : begin
		; Get the directory where the regression output is to be channeled to
		WIDGET_CONTROL, sWidget.wOutputDirText, GET_VALUE=temp
		sWidget.output_dir = temp
	end

	sWidget.wLookupDirText : begin
		; Get the directory where the lookup table is to be found/created
		WIDGET_CONTROL, sWidget.wLookupDirText, GET_VALUE=temp
		sWidget.lookuptable_dir = temp
	end

	sWidget.wRandomDirText : begin
		; Get the directory where the lookup table is to be found/created
		WIDGET_CONTROL, sWidget.wRandomDirText, GET_VALUE=temp
		sWidget.random_dir = temp
	end

	sWidget.wStimulusDirText : begin
		; Get the directory where the stimulus can be found
		WIDGET_CONTROL, sWidget.wStimulusDirText, GET_VALUE=temp
		sWidget.stimulus1_dir = temp
	end

	sWidget.wDataDirText : begin
		; Get the directory where the data can be found for disk mode
		WIDGET_CONTROL, sWidget.wDataDirText, GET_VALUE=temp
		sWidget.cvioData_dir = temp
	end

	sWidget.wPrepareButton : begin
		; Make sure all user input directories end in "/" for proper file creation/storage
		if (strmid(sWidget.stimulus1_dir,0,1,/REVERSE_OFFSET) ne '/') then begin
			sWidget.stimulus1_dir = sWidget.stimulus1_dir + '/'
		endif
		if (strmid(sWidget.output_dir,0,1,/REVERSE_OFFSET) ne '/') then begin
			sWidget.output_dir = sWidget.output_dir + '/'
		endif
		if (strmid(sWidget.lookuptable_dir,0,1,/REVERSE_OFFSET) ne '/') then begin
			sWidget.lookuptable_dir = sWidget.lookuptable_dir + '/'
		endif
		if (strmid(sWidget.cvioData_dir,0,1,/REVERSE_OFFSET) ne '/') then begin
			sWidget.cvioData_dir = sWidget.cvioData_dir + '/'
		endif

		; Determine whether each piece of necessary information has been entered
		proceed = -1
		if ((sWidget.RunNum eq '') and (proceed eq -1)) then begin
	  		WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter The'+ $
				' Run Number Before Hitting GO'
			proceed = 1
		endif
		if ((sWidget.Period eq '-1') and (sWidget.Stimulus_Regress eq 1) and $
			(proceed eq -1)) then begin
	  			WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter The'+ $
					' Stimulus Period Before Hitting GO'
			proceed = 1
		endif
		if ((sWidget.Source eq -1) and (proceed eq -1)) then begin
	  		WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter The'+ $
				' Data Source Before Hitting GO'
			proceed = 1
		endif
		if ((sWidget.Order eq -1) and (proceed eq -1)) then begin
	  		WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter The'+ $
				' Collection Order Before Hitting GO'
			proceed = 1
		endif
		if ((sWidget.NumImg eq -1) and (proceed eq -1)) then begin
	  		WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter The'+ $
				' Total Number Of Images Before Hitting GO'
			proceed = 1
		endif
		if ((sWidget.NumSli eq -1) and (proceed eq -1)) then begin
	  		WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter The'+ $
				' Total Number Of Slices Before Hitting GO'
			proceed = 1
		endif
		if ((sWidget.Rate eq -1) and (proceed eq -1)) then begin
	  		WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter The'+ $
				' Collection Rate Before Hitting GO'
			proceed = 1
		endif
		if ((sWidget.RegressSlice eq -1) and (proceed eq -1)) then begin
	  		WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter The'+ $
				' Slice To Be Regressed Before Hitting GO'
			proceed = 1
		endif
		if ((sWidget.Channels eq 0) and (proceed eq -1)) then begin
	  		WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter'+ $
				' CVIO Channels To Be Opened/Collected Before Hitting GO'
			proceed = 1
		endif
		if ((sWidget.Num_Vars eq 0) and (proceed eq -1)) then begin
	  		WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter'+ $
				' Variables To Be Regressed Against Before Hitting GO'
			proceed = 1
		endif

		; Check for a couple of things required only in real-time mode
		if ((sWidget.Source eq 0) and (proceed eq -1)) then begin
			if ((sWidget.Duration eq -1) and (proceed eq -1)) then begin
	  			WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Enter A'+ $
					' Duration Before Hitting GO'
				proceed = 1
			endif
		endif
		
		if (proceed eq -1) then begin		
			; Determine whether disk files with same run number already exist and take
			; appropriate actions
			err=cvio_open(sWidget.cvioData_dir+sWidget.file5,1,check_exists)
			if (err eq 0) then begin
				; if in real-time mode determine if files are there
				if (sWidget.Source eq 0) then begin
					; Update the user on status
	  				WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Files Already'+ $
						' Exist - Change Run Number Before Hitting GO'
					proceed = 1
					err=cvio_close(check_exists)
				endif
			endif else begin
				; if in disk mode determine if files are not there
				if (sWidget.Source eq 1) then begin
					; Update the user on status
	  				WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Files Do Not'+ $
						' Exist - Change Run Number Before Hitting GO'
					proceed = 1
				endif
			endelse
		endif

		if (proceed eq -1) then begin
			; Determine if lookup tables exists based on user entered period
			if (sWidget.Stim_Conv eq 1) then begin
				err=cvio_open(sWidget.lookuptable_dir+'table'+strtrim(sWidget.Period,2), $
					1,junk_table)

				if (err ne 0) then begin
					; Does not exist, so create
					; Update the user on status
	  				WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Please Hold While'+ $
						' Lookup Tables Are Generated'

					lookup_table,sWidget.lookuptable_dir,sWidget.Period
					err=cvio_open(sWidget.lookuptable_dir+'table'+strtrim(sWidget.Period,2), $
						1,junk_table2)
					if (err eq 0) then begin
	  					WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Lookup Table'+ $
							' Successfully Generated'
						err=cvio_close(junk_table2)
					endif 
				endif else begin
					; It does exist so move on
					err=cvio_close(junk_table)
				endelse
			endif
			if (sWidget.Stim_Sin eq 1) then begin
				err=cvio_open(sWidget.lookuptable_dir+'sin_table'+strtrim(sWidget.Period,2), $
					1,junk_table)

				if (err ne 0) then begin
					; Does not exist, so create
					; Update the user on status
	  				WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Please Hold While Lookup Tables'+ $
						' Are Generated'

					sin_lookup_table,sWidget.lookuptable_dir,sWidget.Period
					err=cvio_open(sWidget.lookuptable_dir+'sin_table'+strtrim(sWidget.Period,2), $
						1,junk_table2)
					if (err eq 0) then begin
	  					WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Lookup Table'+ $
							' Successfully Generated'
						err=cvio_close(junk_table2)
					endif
				endif else begin
					; It does exist so move on
					err=cvio_close(junk_table)
				endelse
			endif
			
			if (sWidget.Stim_Cos eq 1) then begin
				err=cvio_open(sWidget.lookuptable_dir+'cos_table'+strtrim(sWidget.Period,2), $
					1,junk_table)

				if (err ne 0) then begin
					; Does not exist, so create
					; Update the user on status
	  				WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Please Hold While Lookup Tables'+ $
						' Are Generated'
	
					cos_lookup_table,sWidget.lookuptable_dir,sWidget.Period
					err=cvio_open(sWidget.lookuptable_dir+'cos_table'+strtrim(sWidget.Period,2), $
						1,junk_table2)
					if (err eq 0) then begin
		  				WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Lookup Table'+ $
							' Successfully Generated'
						err=cvio_close(junk_table2)
					endif
				endif else begin
					; It does exist so move on
					err=cvio_close(junk_table)
				endelse
			endif
		endif

		if (proceed eq -1) then begin
			; If in real-time mode determine file number for images from cross-mounted hard drive
			if (sWidget.Source eq 0) then begin
				spawn,"get_file_name2",junk
				output=strmid(junk,0,strpos(junk,'.'))
				chris=strmid(junk,strpos(junk,'.')+1,strlen(junk))
				chris=strmid(chris,0,strpos(chris,'.'))
				sWidget.BaseNum=chris
				sWidget.BaseNum2=output
				
			endif
			; Update the user on status
	  		WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='All Necessary Data Entered, ' + $
				'You May Hit GO'
			
			; Generate pop up menu so user inputs can be checked
			sWidget.OriginalOutputFile = sWidget.OutputFile+".slice"+strtrim(sWidget.GetPicture,2)
			writeInputs,sWidget
			xdisplayfile,sWidget.output_dir+sWidget.InputFile, /BLOCK, $
				font='-*-times-*-r-*-*-15-*-*-*-*-*-*-*',HEIGHT=40,TITLE='User Inputs'
			remove_file="rm "+sWidget.output_dir+sWidget.InputFile
			spawn,remove_file

			; Activate the go button
			WIDGET_CONTROL,sWidget.wGoButton,SENSITIVE=1
		endif
	END
				
	sWidget.wGoButton : begin
		device, pseudo=8
		zero = intarr(sWidget.NumImg/sWidget.NumSli)
		; Determine whether regression will need to be performed
		if ((sWidget.Cardiac_Regress eq 1) OR (sWidget.Stimulus_Regress eq 1) OR $
		    (sWidget.Respiratory_Regress eq 1)) then begin
			; Based on number of variables determine where each type of signal lies
			; in the multi-dimensional arrays
			determinePosition, sWidget
			; Generate arrays to sort data once generated
			if (sWidget.StripchartTypeOutput gt 4) then begin
				if ((sWidget.StripchartTypeOutput eq 5) or (sWidget.StripchartTypeOutput eq 8) $
				  	or (sWidget.StripchartTypeOutput eq 11) or (sWidget.StripchartTypeOutput eq 14) $
				  	or (sWidget.StripchartTypeOutput eq 17) or (sWidget.StripchartTypeOutput eq 24) $
					or (sWidget.StripchartTypeOutput eq 27)) then begin
					nice=sWidget.Num_Vars*lindgen(sWidget.orig_sizex*sWidget.orig_sizey)+ $
					     sWidget.CardShift
				endif else if ((sWidget.StripchartTypeOutput eq 6) or $
				    (sWidget.StripchartTypeOutput eq 9) or (sWidget.StripchartTypeOutput eq 12) $
				    or (sWidget.StripchartTypeOutput eq 15) or (sWidget.StripchartTypeOutput eq 18) or $
					(sWidget.StripchartTypeOutput eq 25) or (sWidget.StripchartTypeOutput eq 28)) $
				    then begin
					nice=sWidget.Num_Vars*lindgen(sWidget.orig_sizex*sWidget.orig_sizey)+ $
					     sWidget.RespShift
				endif else if ((sWidget.StripchartTypeOutput eq 7) or $
				    (sWidget.StripchartTypeOutput eq 10) or (sWidget.StripchartTypeOutput eq 13) $
				    or (sWidget.StripchartTypeOutput eq 16) or (sWidget.StripchartTypeOutput eq 19) $
					or (sWidget.StripchartTypeOutput eq 21) or (sWidget.StripchartTypeOutput eq 26) $
					or (sWidget.StripchartTypeOutput eq 29)) then begin
					nice=sWidget.Num_Vars*lindgen(sWidget.orig_sizex*sWidget.orig_sizey)+ $
					     sWidget.StimShift
				endif else if ((sWidget.StripchartTypeOutput eq 22) or $
					(sWidget.StripchartTypeOutput eq 23)) then begin
					if (sWidget.Multi_Card_Regress gt 1) then begin
						nice=sWidget.Num_Vars*lindgen(sWidget.orig_sizex*sWidget.orig_sizey)+ $
						     sWidget.CardShift
						sugar=sWidget.Num_Vars*lindgen(sWidget.orig_sizex*sWidget.orig_sizey)+ $
							(sWidget.CardShift+1)	
					endif
					if (sWidget.Multi_Resp_Regress gt 1) then begin
						nice=sWidget.Num_Vars*lindgen(sWidget.orig_sizex*sWidget.orig_sizey)+ $
						     sWidget.RespShift
						sugar=sWidget.Num_Vars*lindgen(sWidget.orig_sizex*sWidget.orig_sizey)+ $
							(sWidget.RespShift+1)
					endif
					if (sWidget.Multi_Stim_Regress gt 1) then begin
						nice=sWidget.Num_Vars*lindgen(sWidget.orig_sizex*sWidget.orig_sizey)+ $
						     sWidget.StimShift
						sugar=sWidget.Num_Vars*lindgen(sWidget.orig_sizex*sWidget.orig_sizey)+ $
							(sWidget.StimShift+1)
					endif
				endif else begin
					; do nothing
				endelse
			endif
			if ((sWidget.ResultTypeOutput eq 5) or (sWidget.ResultTypeOutput eq 8) $
			    or (sWidget.ResultTypeOutput eq 11) or (sWidget.ResultTypeOutput eq 14) $
			    or (sWidget.ResultTypeOutput eq 17) or (sWidget.ResultTypeOutput eq 24)) $
				then begin
				lemon=sWidget.Num_Vars*lindgen(sWidget.orig_sizex*sWidget.orig_sizey)+ $
				     sWidget.CardShift
			endif else if ((sWidget.ResultTypeOutput eq 6) or $
			    (sWidget.ResultTypeOutput eq 9) or (sWidget.ResultTypeOutput eq 12) $
			    or (sWidget.ResultTypeOutput eq 15) or (sWidget.ResultTypeOutput eq 18) $
				or (sWidget.ResultTypeOutput eq 25)) then begin
				lemon=sWidget.Num_Vars*lindgen(sWidget.orig_sizex*sWidget.orig_sizey)+ $
				     sWidget.RespShift
			endif else if ((sWidget.ResultTypeOutput eq 7) or $
			    (sWidget.ResultTypeOutput eq 10) or (sWidget.ResultTypeOutput eq 13) or $
			    (sWidget.ResultTypeOutput eq 16) or (sWidget.ResultTypeOutput eq 19) or $
				(sWidget.ResultTypeOutput eq 26)) then begin
				lemon=sWidget.Num_Vars*lindgen(sWidget.orig_sizex*sWidget.orig_sizey)+ $
				     sWidget.StimShift
			endif else if ((sWidget.ResultTypeOutput eq 22) or (sWidget.ResultTypeOutput eq 21) $
				or (sWidget.ResultTypeOutput eq 23)) then begin
				if (sWidget.Multi_Card_Regress gt 1) then begin
					lemon=sWidget.Num_Vars*lindgen(sWidget.orig_sizex*sWidget.orig_sizey)+ $
					     sWidget.CardShift
					lime=sWidget.Num_Vars*lindgen(sWidget.orig_sizex*sWidget.orig_sizey)+ $
						(sWidget.CardShift+1)	
				endif
				if (sWidget.Multi_Resp_Regress gt 1) then begin
					lemon=sWidget.Num_Vars*lindgen(sWidget.orig_sizex*sWidget.orig_sizey)+ $
					     sWidget.RespShift
					lime=sWidget.Num_Vars*lindgen(sWidget.orig_sizex*sWidget.orig_sizey)+ $
						(sWidget.RespShift+1)
				endif
				if (sWidget.Multi_Stim_Regress gt 1) then begin
					lemon=sWidget.Num_Vars*lindgen(sWidget.orig_sizex*sWidget.orig_sizey)+ $
					     sWidget.StimShift
					lime=sWidget.Num_Vars*lindgen(sWidget.orig_sizex*sWidget.orig_sizey)+ $
						(sWidget.StimShift+1)
				endif
			endif else begin
				; do nothing
			endelse
			
			; Set up array to pass into regression structure determining variable type
			pass_array = intarr(sWidget.Num_Vars)
			if (sWidget.Constant eq 1) then begin	
				pass_array(0) = -1
			endif

			; Declare arrays to hold output
			hold_independent = fltarr(sWidget.orig_sizex,sWidget.orig_sizey)
			hold_dependent = fltarr(sWidget.orig_sizex,sWidget.orig_sizey,sWidget.Num_Vars)
			if (sWidget.SlidingWindow ne -1) then begin
				store_independent = fltarr(sWidget.orig_sizex,sWidget.orig_sizey,sWidget.SlidingWindow)
				store_dependent = fltarr(sWidget.orig_sizex,sWidget.orig_sizey,sWidget.Num_Vars, $
					sWidget.SlidingWindow)
				WindowIndex=0
			endif

			; Set up regression with or without mask
			if (sWidget.Mask eq 1) then begin
				mask=bytarr(sWidget.orig_sizex, sWidget.orig_sizey)

				; Based on previous ROI determinations, create a mask
				if (sWidget.ROIType eq 'User Defined - Box') then begin
					mask(sWidget.ROIx:(sWidget.ROIx+sWidget.ROIdx-1), $
						((sWidget.orig_sizey-1)-sWidget.ROIy-sWidget.ROIdy):((sWidget.orig_sizey-1)-sWidget.ROIy-1))=1
				endif else if (sWidget.ROIType eq 'User Defined - Arbitrary') then begin
					help, sWidget.Store_ROI
					mask(sWidget.Store_ROI)=1
				endif else begin
					mask(sWidget.Comp_Store_ROI)=1
				endelse

				if (sWidget.Expand ne -1) then begin
					s=replicate(1,3,3)
					for i=1,sWidget.Expand do mask=dilate(mask,s)
				endif
	
				reg = reg_create(pass_array,sWidget.orig_sizex*sWidget.orig_sizey,mask)
			endif else begin	
				reg = reg_create(pass_array,sWidget.orig_sizex*sWidget.orig_sizey)
			endelse
		endif

		; If operating from shared memory set-up/begin collection of data
		if (sWidget.Source eq 0) then begin
			acquire_start, sWidget.Duration, sWidget.Rate, sWidget.Acquire_Channels, $
			  sWidget.Acquire_Files, 0, sWidget.file5, sWidget.NumImg, sWidget.BaseNum, $
			  sWidget.orig_sizex, sWidget.orig_sizey, sWidget.BaseNum2
		; If operating from disk open image acquisition file to determine when events occur
		endif else begin
			err=cvio_open(sWidget.cvioData_dir+sWidget.file1,1,img_acq)
		endelse

		; If regressing against cardiac data open appropriate file based on source
		if (sWidget.Cardiac_Regress eq 1) then begin
			;if (sWidget.Source EQ 0) then begin
			;	err=cvio_open(':shmem:'+sWidget.file2,1,cardiac)
			;endif
			;if (sWidget.Source EQ 1) then begin
			;	err=cvio_open(sWidget.cvioData_dir+sWidget.file2,1,cardiac)
			;endif
			if (sWidget.Card_Cos eq 1) then begin
				err=cvio_open(sWidget.cvioData_dir+sWidget.file7,1,card_table2)
			endif
			if (sWidget.Card_Sin eq 1) then begin
				err=cvio_open(sWidget.cvioData_dir+sWidget.file6,1,card_table3)
			endif
			print,"card open err = ",err
		endif
		; If regressing against respiratory data open appropriate file based on source
		if (sWidget.Respiratory_Regress eq 1) then begin
			;junk_time = ulonarr(10)
			;next = ulong('ffffffff'x)
			;if (sWidget.Source EQ 0) then begin
			;	err=cvio_open(':shmem:'+sWidget.file3,1,respiratory)
			;endif
			;if (sWidget.Source EQ 1) then begin
			;	err=cvio_open(sWidget.cvioData_dir+sWidget.file3,1,respiratory)
			;endif
			if (sWidget.Resp_None eq 1) then begin
				err=cvio_open(sWidget.cvioData_dir+sWidget.file9,1,resp_table1)
			endif
			print,"resp open err = ",err
		endif
		; If regressing against stimulus data open appropriate files based on source
		if (sWidget.Stimulus_Regress eq 1) then begin
			; Open up lookup table
			; Determine if lookup tables exists based on user entered period
			if (sWidget.Stim_Conv eq 1) then begin
				err=cvio_open(sWidget.lookuptable_dir+'table'+strtrim(sWidget.Period,2), $
					1,l_table1)
			endif
			if (sWidget.Stim_Sin eq 1) then begin
				err=cvio_open(sWidget.lookuptable_dir+'sin_table'+strtrim(sWidget.Period,2), $
					1,l_table2)
			endif
			if (sWidget.Stim_Cos eq 1) then begin
				err=cvio_open(sWidget.lookuptable_dir+'cos_table'+strtrim(sWidget.Period,2), $
					1,l_table3)
			endif
			; Open up stimulus cvio files
			if (sWidget.Source EQ 0) then begin
				err=cvio_open(':shmem:'+sWidget.file4,1,stimulus)
				print,"shmem stim open err = ",err
				sWidget.StimStart = -1
			endif
			if (sWidget.Source EQ 1) then begin
				err=cvio_open(sWidget.cvioData_dir+sWidget.file4,1,stimulus)
				print, "stim_open = ",err
				sWidget.StimStart = -1
				; Determine time-stamp of first imaging event because in disk mode
				while (sWidget.StimStart eq -1) do begin
					; Determine timestamp of first stimulus event for later use if in disk mode
					err1=cvio_read_next_samples(stimulus,stim_t,stim_d,sWidget.Rate/2)
					hit=where(stim_d gt 32000)
					if (hit(0) ne -1) then begin
						first_time=stim_t(hit(0))
						print, "first_time = ",first_time
						sWidget.StimStart=1
					endif
				endwhile
				err=cvio_close(stimulus)
			endif
		endif
		if (sWidget.Trending eq 1) then begin
			err=cvio_open(sWidget.cvioData_dir+sWidget.file8,1,trend_table)
			print, "trend table open = ",err
		endif
		if (sWidget.Global eq 1) then begin
			err=cvio_open(sWidget.cvioData_dir+sWidget.file10,1,global_table)
			print, "global table open = ",err
		endif

		card_overlap = -1
		resp_overlap = -1
		img_overlap = -1
		; Based on what stripchart will display open appropriate files if not already opened,
		; determine title for stripchart
		if (sWidget.StripchartTypeOutput EQ 4) then begin
			if (sWidget.Source EQ 0) then begin
				err=cvio_open(':shmem:'+sWidget.file1,1,chart_type)
			endif
			if (sWidget.Source EQ 1) then begin
				img_overlap = 1
			endif
			label = "Image Acquisition"
			print,"acq chart err = ",err
		endif else if (sWidget.StripchartTypeOutput EQ 1) then begin
			if (sWidget.Cardiac_Regress eq 1) then begin
				card_overlap = 1
			endif else begin
				if (sWidget.Source EQ 0) then begin
					err=cvio_open(':shmem:'+sWidget.file2,1,chart_type)
				endif
				if (sWidget.Source EQ 1) then begin
					err=cvio_open(sWidget.cvioData_dir+sWidget.file2,1,chart_type)
				endif
			endelse
			label = "Cardiac"
			print,"card chart err = ",err
		endif else if (sWidget.StripchartTypeOutput EQ 2) then begin
			if (sWidget.Respiratory_Regress eq 1) then begin
				resp_overlap = 1
			endif else begin
				if (sWidget.Source EQ 0) then begin
					err=cvio_open(':shmem:'+sWidget.file3,1,chart_type)
				endif
				if (sWidget.Source EQ 1) then begin
					err=cvio_open(sWidget.cvioData_dir+sWidget.file3,1,chart_type)
				endif
			endelse
			label = "Respiratory"
			print,"resp chart err = ",err
		endif else if (sWidget.StripchartTypeOutput EQ 3) then begin
			if (sWidget.Source EQ 0) then begin
				err=cvio_open(':shmem:'+sWidget.file4,1,chart_type)
			endif
			if (sWidget.Source EQ 1) then begin
				err=cvio_open(sWidget.cvioData_dir+sWidget.file4,1,chart_type)
			endif
			label = "Stimulus"
			print,"stim chart err = ",err
		; Set up array to contain outputs of regression which will be graphed
		endif else begin
			if (sWidget.StripchartTypeOutput eq 20) then begin
				mean_chart_results=fltarr(sWidget.NumImg/sWidget.NumSli)
				plot_hrf=fltarr(sWidget.NumImg/sWidget.NumSli)
				chart_title='Mean of ROI Pixels'
			endif else if ((sWidget.StripchartTypeOutput lt 20) or $
				(sWidget.StripchartTypeOutput gt 21)) then begin
				chart_results=fltarr(sWidget.NumImg/sWidget.NumSli)
				min_chart_results=fltarr(sWidget.NumImg/sWidget.NumSli)
				zero = min_chart_results
				if ((sWidget.StripchartTypeOutput ge 24) or $
					(sWidget.StripchartTypeOutput lt 26)) then begin
					max_plus_error=fltarr(sWidget.NumImg/sWidget.NumSli)
					max_minus_error=fltarr(sWidget.NumImg/sWidget.NumSli)
					min_plus_error=fltarr(sWidget.NumImg/sWidget.NumSli)
					min_minus_error=fltarr(sWidget.NumImg/sWidget.NumSli)	
				endif
			endif else begin
				total_chart_results=fltarr(sWidget.NumImg/sWidget.NumSli)
				chart_title='Motion Detection'
			endelse
		endelse

		; If performing a regression, set up array to contain outputs which will be displayed
		if (sWidget.ResultTypeOutput ne -1) then begin
			reg_results=fltarr(sWidget.orig_sizex,sWidget.orig_sizex,(sWidget.NumImg/sWidget.NumSli))
			output_time=ulonarr(sWidget.NumImg/sWidget.NumSli)
		endif 
		
		; Open tagged image file
		if (sWidget.Source EQ 0) then begin
			err=cvio_open(':shmem:'+sWidget.file5,1,tagged_image)
		endif
		if (sWidget.Source EQ 1) then begin
			err=cvio_open(sWidget.cvioData_dir+sWidget.file5,1,tagged_image)
		endif
		print,"tag open err = ",err

		; Run stimulus if in real-time mode only
		if ((sWidget.Source eq 0) and (sWidget.StimulusName ne '')) then begin
			if (sWidget.StimSelect eq 1) then begin
				cmd=sWidget.stimulus1_dir+"call_bigbandw "+sWidget.stimulus1_dir+ $
					sWidget.StimulusName+" junk &"
			endif else if (sWidget.StimSelect eq 2) then begin
				cmd=sWidget.stimulus2_dir+"feedback_stim "+sWidget.stimulus2_dir+ $
					sWidget.StimulusName+" junk &"
			endif else if (sWidget.StimSelect eq 3) then begin
				cmd=sWidget.stimulus3_dir+sWidget.StimulusName+" "+sWidget.stimulus3_dir+ $
					" junk &"
			endif else if (sWidget.StimSelect eq 5) then begin
				cmd=sWidget.stimulus1_dir+"call_bigbandw2 "+sWidget.stimulus1_dir+ $
					sWidget.StimulusName+" junk &"
			endif else if (sWidget.StimSelect eq 10) then begin
				cmd=sWidget.random_dir+"random_stim2 "+sWidget.random_dir+sWidget.Random_File+" "+sWidget.random_dir
			endif else begin
				cmd=sWidget.stimulus4_dir+sWidget.StimulusName+" "+sWidget.stimulus4_dir+ $
					sWidget.StimulusName2+" &"
			endelse
			spawn,cmd,ret
			print, ret
 		endif

		; Shared memory case
		if (sWidget.Source eq 0) then begin
			; Delare constants for tracking purposes
			abort_time = DOUBLE(60)
			temp=0
			check=0
			slice=0
			bad_run=0
			time1 = systime(1)
			while (temp NE (sWidget.NumImg/sWidget.NumSli)) do begin
				; If haven't determined timestamp of first image acquisition, attempt to do so
				if (sWidget.StimStart eq -1) then begin
					err1=cvio_read_next_samples(stimulus,stim_t,stim_d,sWidget.Rate/2)
					if (err1 eq 0) then begin
						hit=where(stim_d gt 32000)
						if (hit(0) ne -1) then begin
							first_time=stim_t(hit(0))
							sWidget.StimStart=1
						endif
					endif
				endif

				; If stripchart display is raw cvio data, display it when enough data
				; written to shared memory to do so, otherwise do nothing
				if ((sWidget.StripchartTypeOutput ge 1) and $
				    (sWidget.StripchartTypeOutput le 4)) then begin
					if (card_overlap eq 1) then begin
						err1 = cvio_read_next_samples(cardiac,t1,d1,sWidget.Rate/5)
					endif else if (resp_overlap eq 1) then begin
						err1 = cvio_read_next_samples(respiratory,t1,d1,sWidget.Rate/5)
					endif else if (img_overlap eq 1) then begin
						t1 = check_t
						d1 = check_d
						err1 = 0
					endif else begin
						err1 = cvio_read_next_samples(chart_type,t1,d1,sWidget.Rate/5)
					endelse
	
					if (err1 eq 0) then begin
						wset,sWidget.chart_win_id
						if (check eq 2) then begin
							oplot,d4,color=0
							check = 1
						endif
						if (check eq 0) then begin
							plot,d1,xtitle='No. Images',POS=[0.2,0.2,0.85,0.85],title=label, $
								charsize=0.5,color=127,yrange=[-34000,34000],xstyle=4+1,ystyle=8
							check = 2
							d4 = d1
						endif
						if (check eq 1) then begin
							oplot,d1,color=127
							check = 2
							d4 = d1
						endif
					endif
				endif
					
				; Check to see if an image has been placed in shared memory so it can be
				; displayed, if not, do nothing
				read_err = 0
				err2 = cvio_read_next_samples(tagged_image,t2,d2,1)
				if (err2 EQ 0) then begin
					time1 = systime(1)
					while (read_err eq 0) do begin
						slice = slice + 1
						if ((slice eq sWidget.GetPicture) and (sWidget.Order eq 0)) then begin
							seq_picture = d2
							holder = d2
							wset,sWidget.orig_win_id
							Final_Raw_Image=bytscl(holder)/2
							holder = rebin(holder,sWidget.disp_sizex,sWidget.disp_sizey)
							holder=reverse(holder,2)
							if (sWidget.StripchartTypeOutput eq 20) then begin
								mean_chart_results(temp)=mean(d2(sWidget.Comp_Store_ROI))
							endif
							tv,bytscl(holder)/2
						endif
						; If this slice is the one to be regressed, display original, gather 
						; raw cvio data and place it in necessary arrays for regression
						if (slice eq sWidget.RegressSlice) then begin
							spot=0
							if (sWidget.Order eq 1) then begin
								seq_picture = d2
								holder = d2
								wset,sWidget.orig_win_id
								Final_Raw_Image=bytscl(holder)/2
								holder = rebin(holder,sWidget.disp_sizex,sWidget.disp_sizey)
								holder=reverse(holder,2)
								if (sWidget.StripchartTypeOutput eq 20) then begin
									mean_chart_results(temp)=mean(d2(sWidget.Comp_Store_ROI))
								endif
								tv,bytscl(holder)/2
							endif

							output_time(temp)=t2
							temp = temp + 1
							print, "temp = ", temp
							if (sWidget.Constant eq 1) then begin
								hold_dependent(*,*,0)=1.0
								spot = spot + 1
							endif
							if (sWidget.Cardiac_Regress eq 1) then begin
								err=cvio_read_samples(cardiac,t2,card_data,1)
								;process card_data if necessary
								hold_dependent(*,*,spot)=float(card_data)
								spot = spot + 1
							endif
							if (sWidget.Respiratory_Regress eq 1) then begin
								junk_time(0)=t2-(5*(20000L/sWidget.Rate))
								junk_time(1:*)=next
								err=cvio_read_samples(respiratory,junk_time, $
									resp_data,n_elements(junk_time))
								med_resp = median(resp_data)
								hold_dependent(*,*,spot)=med_resp
								spot = spot + 1
							endif
							if (sWidget.Stimulus_Regress eq 1) then begin
								distance = t2 - first_time
								position = ((distance MOD (sWidget.Period*20000L))+1)/20
								if (sWidget.Stim_Conv eq 1) then begin
									err=cvio_read_samples(l_table1,position,hrf_data,1)
									hold_dependent(*,*,spot)=hrf_data
									if (sWidget.StripchartTypeOutput eq 20) then begin
										plot_hrf(temp-1)=hrf_data
									endif
									spot = spot + 1
								endif
								if (sWidget.Stim_Sin eq 1) then begin
									err=cvio_read_samples(l_table2,position,hrf_data,1)
									hold_dependent(*,*,spot)=hrf_data
									if (sWidget.StripchartTypeOutput eq 20) then begin
										plot_hrf(temp-1)=hrf_data
									endif
									spot = spot + 1
								endif
								if (sWidget.Stim_Cos eq 1) then begin
									err=cvio_read_samples(l_table3,position,hrf_data,1)
									hold_dependent(*,*,spot)=hrf_data
									if (sWidget.StripchartTypeOutput eq 20) then begin
										plot_hrf(temp-1)=hrf_data
									endif
									spot = spot + 1
								endif
							endif
						endif

						if (((slice ge sWidget.GetPicture) and (slice eq sWidget.RegressSlice)) or $
						   ((slice eq sWidget.GetPicture) and (slice ge sWidget.RegressSlice))) then begin
							if (temp gt 1) then begin
							; Add data to regression arrays and perform regression calculations
							if ((sWidget.Cardiac_Regress eq 1) OR (sWidget.Stimulus_Regress eq 1) OR $
				    			(sWidget.Respiratory_Regress eq 1)) then begin
								hold_independent = float(seq_picture)
								if (sWidget.SlidingWindow ne -1) then begin
									if (WindowIndex lt sWidget.SlidingWindow) then begin
										status=reg_add_obs(reg,hold_independent,hold_dependent)
										store_independent(*,*,WindowIndex)=hold_independent
										store_dependent(*,*,*,WindowIndex)=hold_dependent
										WindowIndex=WindowIndex+1
									endif else begin
										status=reg_add_obs(reg,hold_independent,hold_dependent)
										lose_independent=store_independent(*,*,0)
										lose_dependent=store_dependent(*,*,*,0)
										status=reg_remove_obs(reg,lose_independent, $
											lose_dependent)
										store_independent(0,0,0)=store_independent(*,*, $
											1:sWidget.SlidingWindow-1)
										store_dependent(0,0,0,0)=store_dependent(*,*,*, $
											1:sWidget.SlidingWindow-1)
										store_independent(*,*,WindowIndex-1)=hold_independent
										store_dependent(*,*,*,WindowIndex-1)=hold_dependent
									endelse
								endif else begin
									status=reg_add_obs(reg,hold_independent,hold_dependent)
								endelse

								; If have ROI simplify regression
								if (sWidget.Mask eq 1) then begin
									status = reg_calc_ss(reg,mask,0,sst,sse,ssv)
									if (((sWidget.ResultTypeOutput ge 24) and $
										(sWidget.ResultTypeOutput le 26)) or $
										((sWidget.StripchartTypeOutput ge 24) and $
										(sWidget.StripchartTypeOutput le 29))) then begin
										status = reg_calc_coef(reg,mask,coef,STD_ERROR=se)
									endif else begin
										status = reg_calc_coef(reg,mask,coef)
									endelse												
								endif else begin
									status = reg_calc_ss(reg,0,sst,sse,ssv)
									if (((sWidget.ResultTypeOutput ge 24) and $
										(sWidget.ResultTypeOutput le 26)) or $
										((sWidget.StripchartTypeOutput ge 24) and $
										(sWidget.StripchartTypeOutput le 29))) then begin
										status = reg_calc_coef(reg,coef,STD_ERROR=se)
									endif else begin
										status = reg_calc_coef(reg,coef)
									endelse
								endelse

								; Based on user input gather the specified data
								; and perform the calculations for the stripchart
								if (sWidget.StripchartTypeOutput ge 5) then begin
								wset,sWidget.chart_win_id
								if ((sWidget.StripchartTypeOutput ge 5) $
								    AND (sWidget.StripchartTypeOutput le 7)) then begin
									very=ssv(nice)
									chart_results(temp-1)=max(very)
									min_chart_results(temp-1)=min(very)
									if (temp eq 2) then begin
										chart_title='Sum of Squares'
										y_range=[0,500000]
									endif
								endif else if ((sWidget.StripchartTypeOutput ge 8) $
								    AND (sWidget.StripchartTypeOutput le 10)) then begin
									very=coef(nice)
									chart_results(temp-1)=max(very)
									min_chart_results(temp-1)=min(very)

									if (temp eq 2) then begin
										chart_title = 'Regression Coefficient'
										y_range = [-600,600]
									endif
								endif else if ((sWidget.StripchartTypeOutput ge 11) $
								    AND (sWidget.StripchartTypeOutput le 13)) then begin
									;effect_size=
									;chart_results(temp-1)=effect_size
									chart_title='Effect Size'
								endif else if ((sWidget.StripchartTypeOutput ge 14) $
								    AND (sWidget.StripchartTypeOutput le 16) $
								    AND (temp gt 2)) then begin
									temp_sse=sse
									temp_coef=coef
									valid=where(sse ne 0)
									wrong=where(sse eq 0)
									bad_coef=where(coef(nice) eq 0)
									good_coef=where(coef(nice) ne 0)
									if (bad_coef(0) ne -1) then begin
										coef[bad_coef]=1.0
									endif
									if (wrong(0) ne -1) then begin
										sse[wrong]=1.0
									endif
									if (sWidget.SlidingWindow eq -1) then begin
										tstat=(sqrt((ssv(nice)/1.)/(sse/(float(temp-2)))) $
										     *(abs(coef(nice))/coef(nice)))
									endif else begin
										if (temp ge sWidget.SlidingWindow-1) then begin
											tstat=(sqrt((ssv(nice)/1.)/(sse/(float(sWidget.SlidingWindow-3)))) $
											     *(abs(coef(nice))/coef(nice)))
										endif else begin
											tstat=(sqrt((ssv(nice)/1.)/(sse/(float(temp-2)))) $
											     *(abs(coef(nice))/coef(nice)))
										endelse
									endelse
									if ((good_coef(0) ne -1) and (valid(0) ne -1)) then begin
										; Assuming t-statistic greater 0 for plotting
										if (bad_coef(0) ne -1) then begin
											tstat[bad_coef]=0
										endif
										if (wrong(0) ne -1) then begin
											tstat[wrong]=0
										endif
										max1=max(tstat[good_coef])
										min1=min(tstat[good_coef])
										max2=max(tstat[valid])
										min2=min(tstat[valid])
										if (max2 gt max1) then begin
											chart_results(temp-1)=max2
											print, max2
										endif else begin
											chart_results(temp-1)=max1
											print, max1
										endelse
										if (min1 lt min2) then begin
											min_chart_results(temp-1)=min1
										endif else begin
											min_chart_results(temp-1)=min2
										endelse
									endif else begin
										chart_results(temp-1)=0
										min_chart_results(temp-1)=0
									endelse
									if (temp eq 3) then begin
										chart_title='t-Statistic'
										y_range=[-12,12]
									endif
									sse=temp_sse
									coef=temp_coef
								endif else if ((sWidget.StripchartTypeOutput ge 24) $
								    AND (sWidget.StripchartTypeOutput le 26)) then begin

									; Save initial data for later
									temp_se = se

									err_arr = se(nice)
									very = coef(nice)
											
									; Eliminate "bad" data points
									valid = where(err_arr ne 0)
									wrong = where(err_arr eq 0)
									if (wrong(0) ne -1) then begin
										err_arr[wrong]=1.0
									endif

									;print, "max coef = ",max(very)
									;print, "max error = ",max(err_arr)											
									
									; Perform calculations
									tstat = very/err_arr
									if (wrong(0) ne -1) then begin
										tstat[wrong]=0
									endif

									big_t = max(tstat)
									;print, "high t = ",big_t
									little_t = min(tstat)
									;print, "low t = ", little_t
									big_spot = where(tstat eq big_t)
									little_spot = where(tstat eq little_t)

									if ((big_spot(0) ne -1) and (little_spot(0) ne -1)) then begin
										chart_results(temp-1)=very[big_spot(0)]
										;print, "high = ",chart_results(temp-1)
										min_chart_results(temp-1)=very[little_spot(0)]
										;print, "low = ",min_chart_results(temp-1)
										max_plus_error(temp-1)=very[big_spot(0)]+sWidget.ConfidenceInterval*err_arr[big_spot(0)]
										max_minus_error(temp-1)=very[big_spot(0)]-sWidget.ConfidenceInterval*err_arr[big_spot(0)]
										min_plus_error(temp-1)=very[little_spot(0)]+sWidget.ConfidenceInterval*err_arr[little_spot(0)]
										min_minus_error(temp-1)=very[little_spot(0)]-sWidget.ConfidenceInterval*err_arr[little_spot(0)]
										;print, "max error = ", err_arr[big_spot(0)]
										;print, "max plus error = ",max_plus_error(temp-1)
										;print, "max minus error = ",max_minus_error(temp-1)
										;print, "min error = ", err_arr[little_spot(0)]
										;print, "min plus error = ",min_plus_error(temp-1)
										;print, "min minus error = ",min_minus_error(temp-1)
									endif else begin
										max_plus_error(temp-1)=0
										max_minus_error(temp-1)=0
										min_plus_error(temp-1)=0
										min_minus_error(temp-1)=0													
									endelse

									if (temp eq 2) then begin
										chart_title = 'Regression Coefficient with Error'
										y_range=[-200,200]
									endif

									se = temp_se
								endif else if ((sWidget.StripchartTypeOutput ge 27) AND $
									(sWidget.StripchartTypeOutput le 29) AND $
									(temp gt 2)) then begin
									; Save initial data for later
									temp_se = se

									; Eliminate "bad" data points
									valid = where(se ne 0)
									wrong = where(se eq 0)
									if (wrong(0) ne -1) then begin
										se[wrong]=1.0
									endif

									; Perform calculations
									tstat = coef(nice)/se(nice)
									if (wrong(0) ne -1) then begin
										tstat[wrong]=0
									endif

									if (valid(0) ne -1) then begin
										chart_results(temp-1) = max(tstat[valid])
										min_chart_results(temp-1) = min(tstat[valid])
									endif else begin
										chart_results(temp-1) = 0
										min_chart_results(temp-1) = 0
									endelse

									if (temp eq 3) then begin
										chart_title='Alternative t-Statistic'
										y_range=[-12,12]
									endif

									; Reset data arrays
									se = temp_se
								endif else if ((sWidget.StripchartTypeOutput ge 17) $
								    AND (sWidget.StripchartTypeOutput le 19) $
								    AND (temp gt 2)) then begin
									temp_sse=sse
									valid=where(sse ne 0)
									wrong=where(sse eq 0)
									if (wrong(0) ne -1) then begin
										sse[wrong]=1.0
									endif
									if (sWidget.SlidingWindow eq -1) then begin
										fstat=((ssv(nice)/1.)/(sse/(float(temp-2))))
									endif else begin
										if (temp ge sWidget.SlidingWindow-1) then begin
											fstat=((ssv(nice)/1.)/(sse/(float(sWidget.SlidingWindow-3))))
										endif else begin
											fstat=((ssv(nice)/1.)/(sse/(float(temp-2))))
										endelse
									endelse
									if (valid(0) ne -1) then begin
										chart_results(temp-1)=max(fstat[valid])
										min_chart_results(temp-1)=min(fstat[valid])
									endif else begin
										chart_results(temp-1)=0
										min_chart_results(temp-1)=0
									endelse

									if (temp eq 3) then begin
										chart_title='F-Statistic'
										y_range = [0,120]
									endif

									sse=temp_sse
								endif else begin
									; do nothing
								endelse
								
								; Plot the data in the stripchart
								if ((temp eq 1) or (temp eq 2)) then begin
									; do nothing waiting for good data
								; create initial plot
								endif else if (temp eq 3) then begin
									if ((sWidget.StripchartTypeOutput lt 20)  or $
										((sWidget.StripchartTypeOutput ge 27) and $
										(sWidget.StripchartTypeOutput le 29))) then begin
										time=indgen((sWidget.NumImg/sWidget.NumSli))+1
										plot,time,chart_results(0:temp-1),xtitle='No. Images',$
										 	POS=[0.2,0.2,0.85,0.85],title=chart_title, $
										  	charsize=0.5,xstyle=1,ystyle=8,color=127, $
											xrange=[0,(sWidget.NumImg/sWidget.NumSli)], $
											yrange=y_range
										last_trip=chart_results(0:temp-1)
										oplot,time,min_chart_results(0:temp-1),color=128
										last_min_trip=min_chart_results(0:temp-1)
										oplot,time,zero,color=127
									endif else if ((sWidget.StripchartTypeOutput le 26) and $
										(sWidget.StripchartTypeOutput ge 24)) then begin
										time=indgen((sWidget.NumImg/sWidget.NumSli))+1
										plot,time,chart_results(0:temp-1),xtitle='No. Images',$
										 	POS=[0.2,0.2,0.85,0.85],title=chart_title, $
										  	charsize=0.5,xstyle=1,ystyle=8,color=127, $
											xrange=[0,(sWidget.NumImg/sWidget.NumSli)], $
											yrange=y_range
										last_trip=chart_results(0:temp-1)
										oplot,time,min_chart_results(0:temp-1),color=127
										oplot,time,max_plus_error(0:temp-1),color=128
										oplot,time,max_minus_error(0:temp-1),color=128
										oplot,time,min_plus_error(0:temp-1),color=128
										oplot,time,min_minus_error(0:temp-1),color=128
										last_min_trip=min_chart_results(0:temp-1)
										last_max_plus=max_plus_error(0:temp-1)
										last_max_minus=max_minus_error(0:temp-1)
										last_min_plus=min_plus_error(0:temp-1)
										last_min_minus=min_minus_error(0:temp-1)
										oplot,time,zero,color=127
									endif else begin
										time=indgen((sWidget.NumImg/sWidget.NumSli))+1
										plot,time,mean_chart_results(0:temp-1),xtitle='No. Images',$
										 	POS=[0.2,0.2,0.85,0.85],title=chart_title, $
										  	charsize=0.5,xstyle=1,ystyle=8,color=127, $
											xrange=[0,(sWidget.NumImg/sWidget.NumSli)]
										last_mean_trip=mean_chart_results(0:temp-1)
										oplot,time,plot_hrf(0:temp-1),color=255
										last_hrf_trip=plot_hrf(0:temp-1)
									endelse
								; plot over old stuff, plot new stuff
								endif else begin
									if ((sWidget.StripchartTypeOutput lt 20)  or $
										((sWidget.StripchartTypeOutput ge 27) and $
										(sWidget.StripchartTypeOutput le 29))) then begin										
								    	oplot,time,last_trip,color=0
								    	oplot,time,chart_results(0:temp-1),color=255
								    	last_trip=chart_results(0:temp-1)
								    	oplot,time,last_min_trip,color=0
								    	oplot,time,min_chart_results(0:temp-1),color=128
								    	last_min_trip=min_chart_results(0:temp-1)
									endif else if ((sWidget.StripchartTypeOutput le 26) and $
										(sWidget.StripchartTypeOutput ge 24)) then begin
									   	oplot,time,last_trip,color=0
									   	oplot,time,chart_results(0:temp-1),color=255
									   	last_trip=chart_results(0:temp-1)
									   	oplot,time,last_min_trip,color=0
									   	oplot,time,min_chart_results(0:temp-1),color=255
									   	last_min_trip=min_chart_results(0:temp-1)
									   	oplot,time,last_max_plus,color=0
									   	oplot,time,max_plus_error(0:temp-1),color=128
									   	last_max_plus=max_plus_error(0:temp-1)
									   	oplot,time,last_max_minus,color=0
									   	oplot,time,max_minus_error(0:temp-1),color=128
									   	last_max_minus=max_minus_error(0:temp-1)
									   	oplot,time,last_min_plus,color=0
									   	oplot,time,min_plus_error(0:temp-1),color=128
									   	last_min_plus=min_plus_error(0:temp-1)
									   	oplot,time,last_min_minus,color=0
									   	oplot,time,min_minus_error(0:temp-1),color=128
									   	last_min_minus=min_minus_error(0:temp-1)
									endif else begin
								    	oplot,time,last_mean_trip,color=0
								    	oplot,time,mean_chart_results(0:temp-1),color=128
								    	last_mean_trip=mean_chart_results(0:temp-1)
								    	oplot,time,last_hrf_trip,color=0
										oplot,time,plot_hrf(0:temp-1),color=255
										last_hrf_trip=plot_hrf(0:temp-1)
									endelse
								endelse
								endif

								; Based on user input, gather the specified data
								; and perform the calculations for the output display
								wset,sWidget.result_win_id
								if ((sWidget.ResultTypeOutput ge 5) AND $
								    (sWidget.ResultTypeOutput le 7)) then begin
									reg_results(*,*,temp-1)=ssv(lemon)
								endif else if ((sWidget.ResultTypeOutput ge 8) $
								    AND (sWidget.ResultTypeOutput le 10)) then begin
									reg_results(*,*,temp-1)=coef(lemon)
								endif else if ((sWidget.ResultTypeOutput ge 11) $
								    AND (sWidget.ResultTypeOutput le 13)) then begin
									;effect_size=
									;reg_results(*,*,temp-1)=effect_size
								endif else if ((sWidget.StripchartTypeOutput ge 14) $
								    AND (sWidget.StripchartTypeOutput le 16) $
								    AND (temp gt 2)) then begin
									temp_sse=sse
									temp_coef=coef
									wrong=where(sse eq 0)
									bad_coef=(coef eq 0)
									if (wrong(0) ne -1) then begin
										sse[wrong]=1.0
									endif
									if (bad_coef(0) ne -1) then begin
										coef[bad_coef]=1.0
									endif
									if (sWidget.SlidingWindow eq -1) then begin
										tstat=(sqrt((ssv(lemon)/1.)/(sse/(float(temp-2)))) $
										     *(abs(coef(lemon))/coef(lemon)))
									endif else begin
										if (temp ge sWidget.SlidingWindow-1) then begin
											tstat=(sqrt((ssv(lemon)/1.)/(sse/(float(sWidget.SlidingWindow-3)))) $
											     *(abs(coef(lemon))/coef(lemon)))
										endif else begin
											tstat=(sqrt((ssv(lemon)/1.)/(sse/(float(temp-2)))) $
											     *(abs(coef(lemon))/coef(lemon)))
										endelse
									endelse
									if (wrong(0) ne -1) then begin
										tstat[wrong]=0
									endif
									if (bad_coef(0) ne -1) then begin
										tstat[bad_coef]=0
									endif
									reg_results(*,*,temp-1)=tstat
									sse=temp_sse
									coef=temp_coef
								endif else if ((sWidget.ResultTypeOutput ge 24) AND $
									(sWidget.ResultTypeOutput le 26) AND $
									(temp gt 2)) then begin
									; Save initial data for later
									temp_se = se
									; Eliminate "bad" data points
									wrong = where(se eq 0)
									if (wrong(0) ne -1) then begin
										se[wrong]=1.0
									endif

									; Perform calculations
									tstat = coef(lemon)/se(lemon)
									if (wrong(0) ne -1) then begin
										tstat[wrong]=0
									endif

									; Put data in output array
									reg_results(*,*,temp-1)=tstat

									; Reset data arrays
									se = temp_se
								endif else if ((sWidget.StripchartTypeOutput ge 14) $
								    AND (sWidget.StripchartTypeOutput le 16) $
								    AND (temp gt 2)) then begin
									temp_sse=sse
									wrong=where(sse eq 0)
									if (wrong(0) ne -1) then begin
										sse[wrong]=1.0
									endif
									if (sWidget.SlidingWindow eq -1) then begin
										fstat=((ssv(lemon)/1.)/(sse/(float(temp-2))))
									endif else begin
										if (temp ge sWidget.SlidingWindow-1) then begin
											fstat=((ssv(lemon)/1.)/(sse/(float(sWidget.SlidingWindow-3))))
										endif else begin
											fstat=((ssv(lemon)/1.)/(sse/(float(temp-2))))
										endelse
									endelse
									if (wrong(0) ne -1) then begin
										fstat[wrong]=0
									endif

									reg_results(*,*,temp-1)=fstat

									sse=temp_sse
								endif else begin
									; do nothing
								endelse

								if (temp ge 3) then begin
								if ((sWidget.ResultTypeOutput ge 14) and (temp eq 3)) then begin
									; do nothing this round
								endif else begin
									; Show the output in the result window
									if (sWidget.ScaleMax eq -1) then begin
										number=max(abs(reg_results(*,*,temp-1)))
										hold=bytscl(reg_results(*,*,temp-1), $
											-number, number)/2+128
									endif else begin
										hold=bytscl(reg_results(*,*,temp-1), $
											-sWidget.ScaleMax,sWidget.ScaleMax)/2+128
									endelse
									hold=rebin(hold,sWidget.disp_sizex,sWidget.disp_sizey)
									hold=reverse(hold,2)
									tv,hold

									if (sWidget.Overlay ne -1) then begin
										wset,sWidget.stim_win_id
										Final_Raw_Image=rebin(Final_Raw_Image,sWidget.disp_sizex,sWidget.disp_sizey)
										temp_image=rebin(reg_results(*,*,temp-1),sWidget.disp_sizex,sWidget.disp_sizey)
										low_values = where(abs(temp_image) lt sWidget.Overlay)
										number=max(abs(temp_image))
										new_processed=bytscl(temp_image,-number, number)/2+128
										overlay_image = new_processed
										overlay_image(low_values) = Final_Raw_Image(low_values)
										overlay_image=reverse(overlay_image,2)
										tv,overlay_image
									endif
								endelse
								endif
							endif
							endif
						endif
						read_err = cvio_read_next_samples(tagged_image,t2,d2,1)

						; If have collected one full image, reset slice counter
						if (slice eq sWidget.NumSli) then begin
							slice = 0
						endif
					endwhile
				endif

				; If want to show stimulus, read necessary data and display in stimulus window
				;if (sWidget.StimStatus eq 'Yes') then begin
					;err3 = cvio_read_next_samples(stimulus,t3,d3,1)
					;print,"err3 = ",err3
					;IF (err3 EQ 0) THEN BEGIN
					;	wset,sWidget.stim_win_id
					;	d3 = congrid(d3,sWidget.disp_sizex,sWidget.disp_sizey);
					;	tv,d3
					;ENDIF
				;endif
	
				; If not enough data had been written to shared memory, wait one-tenth of a second
				; and check to make sure haven't been waiting for twenty seconds doing nothing
				if (err2 ne 0) then begin
					wait,0.1
					; If have been doing nothing for twenty seconds stop the acquisition and
					; write the data to disk files
					if ((systime(1) - time1) GT abort_time) THEN BEGIN
						acquire_finish, sWidget.Duration, sWidget.Rate, $
						  sWidget.Acquire_Channels, sWidget.Acquire_Files,3, $
						  sWidget.file5, sWidget.BaseNum, sWidget.NumImg,sWidget.output_dir
						temp = sWidget.NumImg/sWidget.NumSli
						print, 'Could not locate sufficient number of images'
						bad_run=1
					endif
				endif				
			endwhile
			
			if (bad_run ne 1) then begin
				; Stop the acquisition, writing data from shared memory to disk
				acquire_finish, sWidget.Duration, sWidget.Rate, sWidget.Acquire_Channels, $
					sWidget.Acquire_Files, 3, sWidget.file5, sWidget.BaseNum,sWidget.NumImg, $
					sWidget.output_dir
			endif
		endif else begin
			; Disk case 
			ImgEvent = 0
			temp = 0
			check = 0
			last = 0
			sec_last = 0
			third_last = 0
			last_time=0
			; With stimulus display
			if (sWidget.StimStatus eq 'Yes') then begin
				count=0
				err = CVIO_OPEN(sWidget.output_dir+sWidget.file5,1,stim_file)
				err = CVIO_GETATTRIBUTE(stim_file, "STIMULUS_NAME_ST", stim_name)
				err = CVIO_CLOSE(stim_file)
				
				openr, 1, sWidget.stimulus1_dir+stim_name
				slice2 = assoc(1, intarr(sWidget.StimSizex,sWidget.StimSizey))
				while (temp NE (sWidget.NumImg/sWidget.NumSli)) do begin
					; Read in specified number of samples and determine if image	
					; acquisition occurred in elapsed time
					err1=cvio_read_next_samples(img_acq,check_t,check_d,sWidget.Rate/5)
					shift_check=shift(check_d,2)
					shift_check(0)=sec_last
					shift_check(1)=last
					event=where((check_d-shift_check) gt 28000)
					if (event(0) ne -1) then begin
						next_shift=shift(shift_check,1)
						next_shift(0)=third_last
						next_shift=[next_shift,check_d(sWidget.Rate/5-3),check_d(sWidget.Rate/5-2)]
						for b=0,n_elements(event)-1 do begin
							if (abs(next_shift(event(b)+2))-abs(next_shift(event(b))) lt 28000) $
							    then begin
								ImgEvent = ImgEvent + 1
							endif
						endfor
						third_last = sec_last
						sec_last = last
						last = abs(check_d(sWidget.Rate/5-1))
					endif

					; If stripchart display is raw cvio data, read and display the data
					; from the designated cvio file
					if ((sWidget.StripchartTypeOutput ge 1) and $
					    (sWidget.StripchartTypeOutput le 4)) then begin
						; Cardiac has been opened for regression so read from there
						if (card_overlap eq 1) then begin
							err1 = cvio_read_next_samples(cardiac,t1,d1,sWidget.Rate/5)
						; Resp has been opened for regression so read from there
						endif else if (resp_overlap eq 1) then begin
							err1 = cvio_read_next_samples(respiratory,t1,d1,sWidget.Rate/5)
						; Imag Acq opened for regression so read from there
						endif else if (img_overlap eq 1) then begin
							t1 = check_t
							d1 = check_d
							err1 = 0
						; CVIO file not needed for regression but opened for stripchart plotting
						endif else begin
							err1 = cvio_read_next_samples(chart_type,t1,d1,sWidget.Rate/5)
						endelse

						; If have proper amount of data, plot it
						if (err1 eq 0) then begin
							wset,sWidget.chart_win_id
							if (check eq 2) then begin
								oplot,d4,color=0
								check = 1
							endif
							if (check EQ 0) then begin
								plot,d1,xtitle='No. Images',POS=[0.2,0.2,0.85,0.85],title=label, $
									charsize=0.5,color=127,yrange=[-34000,34000],xstyle=4+1, $
									ystyle=8
								check = 2
								d4 = d1
							endif
							if (check EQ 1) then begin
								oplot,d1,color=127
								check = 2
								d4 = d1
							endif
						endif
					endif

					; If have collected enough slices to be one image, proceed
					if (ImgEvent gt (sWidget.NumSli-1)) then begin
						spot = 0
						for u=1,sWidget.NumSli do begin
							err2 = cvio_read_next_samples(tagged_image,t2,d2,1)
							; If this slice is one to be regressed, display original, gather 
							; raw cvio data and place it in necessary arrays for regression
							if (u eq sWidget.RegressSlice) then begin
								holder = d2
								wset,sWidget.orig_win_id
								holder = rebin(holder,sWidget.disp_sizex,sWidget.disp_sizey)
								holder=reverse(holder,2)
								tvscl,holder

								output_time(temp)=t2
								temp = temp + 1
								print, "temp = ", temp
								if (sWidget.Constant eq 1) then begin
									hold_dependent(*,*,0)=1.0
									spot = spot + 1
								endif
								if (sWidget.Cardiac_Regress eq 1) then begin
									err=cvio_read_samples(cardiac,t2,card_data,1)
									;process card_data if necessary
									hold_dependent(*,*,spot)=float(card_data)
									spot = spot + 1
								endif
								if (sWidget.Respiratory_Regress eq 1) then begin
									junk_time(0)=t2-(5*(20000L/sWidget.Rate))
									junk_time(1:*)=next
									err=cvio_read_samples(respiratory,junk_time, $
										resp_data,n_elements(junk_time))
									med_resp = median(resp_data)
									hold_dependent(*,*,spot)=med_resp
									spot = spot + 1
								endif
								if (sWidget.Stimulus_Regress eq 1) then begin
									distance = t2 - first_time
									position = ((distance MOD (sWidget.Period*20000L))+1)/20
									if (sWidget.Stim_Conv eq 1) then begin
										err=cvio_read_samples(l_table1,position,hrf_data,1)
										hold_dependent(*,*,spot)=hrf_data
										if (sWidget.StripchartTypeOutput eq 20) then begin
											plot_hrf(temp-1)=hrf_data
										endif
										spot = spot + 1
									endif
									if (sWidget.Stim_Sin eq 1) then begin
										err=cvio_read_samples(l_table2,position,hrf_data,1)
										hold_dependent(*,*,spot)=hrf_data
										if (sWidget.StripchartTypeOutput eq 20) then begin
											plot_hrf(temp-1)=hrf_data
										endif
										spot = spot + 1
									endif
									if (sWidget.Stim_Cos eq 1) then begin
										err=cvio_read_samples(l_table3,position,hrf_data,1)
										hold_dependent(*,*,spot)=hrf_data
										if (sWidget.StripchartTypeOutput eq 20) then begin
											plot_hrf(temp-1)=hrf_data
										endif
										spot = spot + 1
									endif
								endif

								if (((u ge sWidget.GetPicture) and (u eq sWidget.RegressSlice)) or $
								((u eq sWidget.GetPicture) and (u ge sWidget.RegressSlice))) then begin
								; Discard first image from processing
								if (temp gt 1) then begin
									; Add data to regression arrays and perform regression calculations
									if ((sWidget.Cardiac_Regress eq 1) OR $
							    		(sWidget.Stimulus_Regress eq 1) OR $
				    				   	(sWidget.Respiratory_Regress eq 1)) then begin
										hold_independent = float(seq_picture)
										if (sWidget.SlidingWindow ne -1) then begin
											if (WindowIndex lt sWidget.SlidingWindow) then begin
												status=reg_add_obs(reg,hold_independent,hold_dependent)
												store_independent(*,*,WindowIndex)=hold_independent
												store_dependent(*,*,*,WindowIndex)=hold_dependent
												WindowIndex=WindowIndex+1
											endif else begin
												status=reg_add_obs(reg,hold_independent,hold_dependent)
												lose_independent=store_independent(*,*,0)
												lose_dependent=store_dependent(*,*,*,0)
												status=reg_remove_obs(reg,lose_independent, $
													lose_dependent)
												store_independent(0,0,0)=store_independent(*,*, $
													1:sWidget.SlidingWindow-1)
												store_dependent(0,0,0,0)=store_dependent(*,*,*, $
													1:sWidget.SlidingWindow-1)
												store_independent(*,*,WindowIndex-1)=hold_independent
												store_dependent(*,*,*,WindowIndex-1)=hold_dependent
											endelse
										endif else begin
											status=reg_add_obs(reg,hold_independent,hold_dependent)
										endelse

										; If have ROI simplify regression
										if (sWidget.Mask eq 1) then begin
											status = reg_calc_ss(reg,mask,0,sst,sse,ssv)
											status = reg_calc_coef(reg,mask,coef,STD_ERROR=se)
										endif else begin
											status = reg_calc_ss(reg,0,sst,sse,ssv)
											status = reg_calc_coef(reg,coef,STD_ERROR=se)
										endelse

										; Based on user input gather specified data
										; and perform the calculations for stripchart
										if (sWidget.StripchartTypeOutput ge 5) then begin
										; Set active window to stripchart display
										wset,sWidget.chart_win_id
										; Based on user input, create array which will
										; plot proper output from regression
										if ((sWidget.StripchartTypeOutput ge 5) $
										    AND (sWidget.StripchartTypeOutput le 7)) then begin
											very=ssv(nice)
											chart_results(temp-1)=max(very)
											min_chart_results(temp-1)=min(very)
											if (temp eq 2) then begin
												chart_title='Sum of Squares'
												y_range = [0,500000]
											endif
										endif else if ((sWidget.StripchartTypeOutput ge 8) $
										    AND (sWidget.StripchartTypeOutput le 10)) then begin
											very=coef(nice)
											chart_results(temp-1)=max(very)
											min_chart_results(temp-1)=min(very)

											if (temp eq 2) then begin
												chart_title = 'Regression Coefficient'
												y_range=[-600,600]
											endif
										endif else if ((sWidget.StripchartTypeOutput ge 11) $
										    AND (sWidget.StripchartTypeOutput le 13)) then begin
											;effect_size=
											;chart_results(temp-1)=effect_size
											if (temp eq 2) then begin
												chart_title='Effect Size'
											endif
										endif else if ((sWidget.StripchartTypeOutput ge 14) $
										    AND (sWidget.StripchartTypeOutput le 16) $
										    AND (temp gt 2)) then begin
											; Save initial data for later
											temp_sse=sse
											temp_coef=coef
											; Eliminate "bad" data points
											valid=where(sse ne 0)
											wrong=where(sse eq 0)
											bad_coef=where(coef(nice) eq 0)
											good_coef=where(coef(nice) ne 0)
											if (bad_coef(0) ne -1) then begin
												coef[bad_coef]=1.0
											endif
											if (wrong(0) ne -1) then begin
												sse[wrong]=1.0
											endif
											; Perform calculations
											if (sWidget.SlidingWindow eq -1) then begin
												tstat=(sqrt((ssv(nice)/1.)/(sse/(float(temp-2)))) $
												     *(abs(coef(nice))/coef(nice)))
											endif else begin
												if (temp ge sWidget.SlidingWindow-1) then begin
													tstat=(sqrt((ssv(nice)/1.)/(sse/(float(sWidget.SlidingWindow-3)))) $
													     *(abs(coef(nice))/coef(nice)))
												endif else begin
													tstat=(sqrt((ssv(nice)/1.)/(sse/(float(temp-2)))) $
													     *(abs(coef(nice))/coef(nice)))
												endelse
											endelse
											; Put data in output array
											if ((good_coef(0) ne -1) and (valid(0) ne -1)) then begin
												; Assuming t-statistic greater 0 for plotting
												if (bad_coef(0) ne -1) then begin
													tstat[bad_coef]=0
												endif
												if (wrong(0) ne -1) then begin
													tstat[wrong]=0
												endif
												max1=max(tstat[good_coef])
												min1=min(tstat[good_coef])
												max2=max(tstat[valid])
												min2=min(tstat[valid])
												if (max2 gt max1) then begin
												  chart_results(temp-1)=max2
												endif else begin
												  chart_results(temp-1)=max1
												endelse
												if (min1 lt min2) then begin
												  min_chart_results(temp-1)=min1
												endif else begin
												  min_chart_results(temp-1)=min2
												endelse 
											endif else begin
												chart_results(temp-1)=0
												min_chart_results(temp-1)=0
											endelse
											if (temp eq 3) then begin
												chart_title='t-Statistic'
												y_range=[-12,12]
											endif
											; Reset data arrays
											sse=temp_sse
											coef=temp_coef
										endif else if ((sWidget.StripchartTypeOutput ge 17) $
										    AND (sWidget.StripchartTypeOutput le 19) $
										    AND (temp gt 2)) then begin
											; Save initial data for later
											temp_sse=sse
											; Eliminate "bad" data points
											valid=where(sse ne 0)
											wrong=where(sse eq 0)
											if (wrong(0) ne -1) then begin
												sse[wrong]=1.0
											endif
											; Perform necessary calculations
											if (sWidget.SlidingWindow eq -1) then begin
												fstat=((ssv(nice)/1.)/(sse/(float(temp-2))))
											endif else begin
												if (temp ge sWidget.SlidingWindow-1) then begin
													fstat=((ssv(nice)/1.)/(sse/(float(sWidget.SlidingWindow-3))))
												endif else begin
													fstat=((ssv(nice)/1.)/(sse/(float(temp-2))))
												endelse
											endelse
											; Put data in output array
											if (valid(0) ne -1) then begin
											    chart_results(temp-1)=max(fstat[valid])
											    min_chart_results(temp-1)=min(fstat[valid])
											endif else begin
												chart_results(temp-1)=0
												min_chart_results(temp-1)=0
											endelse
											if (temp eq 3) then begin
												chart_title='F-Statistic'
												y_range = [0,120]
											endif
											; Reset data arrays
											sse=temp_sse
										endif else if ((sWidget.StripchartTypeOutput eq 21) $
											and (temp gt 4)) then begin
											total_chart_results(temp-1)= $
												total(abs(motion_picture-float(seq_picture))) $
												/(sWidget.orig_sizex*sWidget.orig_sizey)
										endif else if ((sWidget.StripchartTypeOutput eq 22) and $
											(temp gt 3)) then begin
											; Save initial data for later
											temp_sse=sse
											; Eliminate "bad" data points
											valid=where(sse ne 0)
											wrong=where(sse eq 0)
											if (wrong(0) ne -1) then begin
												sse[wrong]=1.0
											endif
											; Perform necessary calculations
											works=(((ssv(nice)+ssv(sugar))/2.)/(sse/(float(temp-3))))
											; Put data in output array
											if (valid(0) ne -1) then begin
											    chart_results(temp-1)=max(works[valid])
											    min_chart_results(temp-1)=min(works[valid])
											endif else begin
												chart_results(temp-1)=0
												min_chart_results(temp-1)=0
											endelse
											if (temp eq 4) then begin
												chart_title='Sin and Cos F-statistic'
												y_range = [0,25]
											endif
											; Reset data arrays
											sse=temp_sse
										endif else if (sWidget.StripchartTypeOutput eq 23) $
											then begin
											; Perform necessary calculations
											event=sqrt(((coef(nice))^2)+((coef(sugar))^2))
											; Put data in output array
											chart_results(temp-1)=max(event)
											min_chart_results(temp-1)=min(event)
											if (temp eq 2) then begin
												chart_title='Sin and Cos Magnitude'
												y_range = [0,120]
											endif
										endif else if ((sWidget.StripchartTypeOutput ge 24) $
										    AND (sWidget.StripchartTypeOutput le 26)) then begin

											err_array = se(nice)

											very = coef(nice)

											high_value = max(very)
											max_spot = where(very eq high_value)
											chart_results(temp-1)=high_value
											;print, "high = ",high_value

											low_value = min(very)
											min_spot = where(very eq low_value)
											min_chart_results(temp-1)=low_value
											;print, "low = ",low_value

											if ((max_spot(0) ne -1) and (min_spot(0) ne -1)) then begin
												max_plus_error(temp-1)=high_value+2*err_array[max_spot(0)]
												max_minus_error(temp-1)=high_value-2*err_array[max_spot(0)]
												;print, "max error = ", err_array[max_spot(0)]
												;print, "max plus error = ",max_plus_error(temp-1)
												;print, "max minus error = ",max_minus_error(temp-1)
												min_plus_error(temp-1)=low_value+2*err_array[min_spot(0)]
												min_minus_error(temp-1)=low_value-2*err_array[min_spot(0)]
												;print, "min error = ", err_array[min_spot(0)]
												;print, "min plus error = ",min_plus_error(temp-1)
												;print, "min minus error = ",min_minus_error(temp-1)
											endif else begin
												max_plus_error(temp-1)=0
												max_minus_error(temp-1)=0
												min_plus_error(temp-1)=0
												min_minus_error(temp-1)=0													
											endelse

											if (temp eq 2) then begin
												chart_title = 'Regression Coefficient with Error'
												y_range=[-600,600]
											endif
										endif else begin
											; do nothing
										endelse
										
										; Plot the data in the stripchart
										if ((temp eq 1) or (temp eq 2)) then begin
											; do nothing waiting for good data
										; create initial plot
										endif else if (temp eq 3) then begin
											if ((sWidget.StripchartTypeOutput lt 20) or $
												(sWidget.StripchartTypeOutput eq 23)) then begin
												plot,chart_results(0:temp-1),xtitle='No. Images',$
												 	POS=[0.2,0.2,0.85,0.85],title=chart_title, $
												  	charsize=0.5,xstyle=4+1,ystyle=8,color=127, $
													xrange=[0,(sWidget.NumImg/sWidget.NumSli)], $
													yrange=y_range
												last_trip=chart_results(0:temp-1)
												oplot,min_chart_results(0:temp-1),color=128
												last_min_trip=min_chart_results(0:temp-1)
												oplot,time,zero,color=127
											endif else if ((sWidget.StripchartTypeOutput le 26) and $
												(sWidget.StripchartTypeOutput ge 24)) then begin
												time=indgen((sWidget.NumImg/sWidget.NumSli))+1
												plot,time,chart_results(0:temp-1),xtitle='No. Images',$
												 	POS=[0.2,0.2,0.85,0.85],title=chart_title, $
												  	charsize=0.5,xstyle=1,ystyle=8,color=127, $
													xrange=[0,(sWidget.NumImg/sWidget.NumSli)], $
													yrange=y_range
												last_trip=chart_results(0:temp-1)
												oplot,time,min_chart_results(0:temp-1),color=127
												oplot,time,max_plus_error(0:temp-1),color=128
												oplot,time,max_minus_error(0:temp-1),color=128
												oplot,time,min_plus_error(0:temp-1),color=128
												oplot,time,min_minus_error(0:temp-1),color=128
												last_min_trip=min_chart_results(0:temp-1)
												last_max_plus=max_plus_error(0:temp-1)
												last_max_minus=max_minus_error(0:temp-1)
												last_min_plus=min_plus_error(0:temp-1)
												last_min_minus=min_minus_error(0:temp-1)
												oplot,time,zero,color=127
											endif else if (sWidget.StripchartTypeOutput eq 20) $
												then begin
												plot,mean_chart_results(0:temp-1),xtitle='No. Images',$
												 	POS=[0.2,0.2,0.85,0.85],title=chart_title, $
												  	charsize=0.5,xstyle=4+1,ystyle=8,color=127, $
													xrange=[0,(sWidget.NumImg/sWidget.NumSli)]
												last_mean_trip=mean_chart_results(0:temp-1)
												oplot,plot_hrf(0:temp-1),color=255
												last_hrf_trip=plot_hrf(0:temp-1)
											endif else begin
												; do nothing
											endelse
										; plot over old stuff, plot new stuff
										endif else begin
											if ((sWidget.StripchartTypeOutput lt 20) or $
												(sWidget.StripchartTypeOutput eq 23)) then begin
										    	oplot,last_trip,color=0
										    	oplot,chart_results(0:temp-1),color=255
										    	last_trip=chart_results(0:temp-1)
										    	oplot,last_min_trip,color=0
										    	oplot,min_chart_results(0:temp-1),color=128
										    	last_min_trip=min_chart_results(0:temp-1)
											endif else if (sWidget.StripchartTypeOutput eq 20) $
												then begin
										    	oplot,last_mean_trip,color=0
										    	oplot,mean_chart_results(0:temp-1),color=128
										    	last_mean_trip=mean_chart_results(0:temp-1)
										    	oplot,last_hrf_trip,color=0
												oplot,plot_hrf(0:temp-1),color=255
												last_hrf_trip=plot_hrf(0:temp-1)
											endif else if (sWidget.StripchartTypeOutput eq 22) $
												then begin
												if (temp eq 4) then begin
													plot,chart_results(0:temp-1),xtitle='No. Images',$
														POS=[0.2,0.2,0.85,0.85],title=chart_title, $
														charsize=0.5,xstyle=4+1,ystyle=8,color=127, $
														xrange=[0,(sWidget.NumImg/sWidget.NumSli)], $
														yrange=y_range
													last_trip=chart_results(0:temp-1)
													oplot,min_chart_results(0:temp-1),color=128
													last_min_trip=min_chart_results(0:temp-1)
												endif else begin
										    		oplot,last_trip,color=0
										    		oplot,chart_results(0:temp-1),color=255
										    		last_trip=chart_results(0:temp-1)
										    		oplot,last_min_trip,color=0
										    		oplot,min_chart_results(0:temp-1),color=128
										    		last_min_trip=min_chart_results(0:temp-1)
												endelse
											endif else if ((sWidget.StripchartTypeOutput le 26) and $
												(sWidget.StripchartTypeOutput ge 24)) then begin
										    	oplot,time,last_trip,color=0
										    	oplot,time,chart_results(0:temp-1),color=255
										    	last_trip=chart_results(0:temp-1)
										    	oplot,time,last_min_trip,color=0
										    	oplot,time,min_chart_results(0:temp-1),color=255
										    	last_min_trip=min_chart_results(0:temp-1)
										    	oplot,time,last_max_plus,color=0
										    	oplot,time,max_plus_error(0:temp-1),color=128
										    	last_max_plus=max_plus_error(0:temp-1)
										    	oplot,time,last_max_minus,color=0
										    	oplot,time,max_minus_error(0:temp-1),color=128
										    	last_max_minus=max_minus_error(0:temp-1)
										    	oplot,time,last_min_plus,color=0
										    	oplot,time,min_plus_error(0:temp-1),color=128
										    	last_min_plus=min_plus_error(0:temp-1)
										    	oplot,time,last_min_minus,color=0
										    	oplot,time,min_minus_error(0:temp-1),color=128
										    	last_min_minus=min_minus_error(0:temp-1)
											endif else begin
												if (temp eq 5) then begin
													plot,total_chart_results(0:temp-1),xtitle='No. Images',$
												 		POS=[0.2,0.2,0.85,0.85],title=chart_title, $
												  		charsize=0.5,xstyle=4+1,ystyle=8,color=127, $
														xrange=[0,(sWidget.NumImg/sWidget.NumSli)], $
														yrange=[0,100]
													last_total_trip=total_chart_results(0:temp-1)
												endif else begin
									    			oplot,last_total_trip,color=0
									    			oplot,total_chart_results(0:temp-1),color=128
									    			last_total_trip=total_chart_results(0:temp-1)
												endelse
											endelse
										endelse
										endif

										; Based on user input, gather specified data and perform 
										; calculations for output display
										; Set active window to output display
										wset,sWidget.result_win_id
										if ((sWidget.ResultTypeOutput ge 5) AND $
										    (sWidget.ResultTypeOutput le 7)) then begin
											reg_results(*,*,temp-1)=ssv(lemon)
										endif else if ((sWidget.ResultTypeOutput ge 8) $
										    AND (sWidget.ResultTypeOutput le 10)) then begin
											reg_results(*,*,temp-1)=coef(lemon)
										endif else if ((sWidget.ResultTypeOutput ge 11) $
										    AND (sWidget.ResultTypeOutput le 13)) then begin
											;effect_size=
											;reg_results(*,*,temp-1)=effect_size
										endif else if ((sWidget.ResultTypeOutput ge 14) $
										    AND (sWidget.ResultTypeOutput le 16) $
										    AND (temp gt 2)) then begin
											; Save initial data for later
											temp_sse=sse
											temp_coef=coef
											; Eliminate "bad" data points
											wrong=where(sse eq 0)
											bad_coef=(coef eq 0)
											if (wrong(0) ne -1) then begin
												sse[wrong]=1.0
											endif
											if (bad_coef(0) ne -1) then begin
												coef[bad_coef]=1.0	
											endif
											; Perform calculations
											if (sWidget.SlidingWindow eq -1) then begin
												tstat=(sqrt((ssv(lemon)/1.)/(sse/(float(temp-2)))) $
												     *(abs(coef(lemon))/coef(lemon)))
											endif else begin
												if (temp ge sWidget.SlidingWindow-1) then begin
													tstat=(sqrt((ssv(lemon)/1.)/(sse/(float(sWidget.SlidingWindow-3)))) $
													     *(abs(coef(lemon))/coef(lemon)))
												endif else begin
													tstat=(sqrt((ssv(lemon)/1.)/(sse/(float(temp-2)))) $
													     *(abs(coef(lemon))/coef(lemon)))
												endelse
											endelse
											if (wrong(0) ne -1) then begin
												tstat[wrong]=0
											endif
											if (bad_coef(0) ne -1) then begin
												tstat[bad_coef]=0
											endif
											; Put data in output array
											reg_results(*,*,temp-1)=tstat
											; Reset data arrays
											sse=temp_sse
											coef=temp_coef
										endif else if ((sWidget.ResultTypeOutput ge 17) $
										    AND (sWidget.ResultTypeOutput le 19) $
										    AND (temp gt 2)) then begin
											; Save initial data for later
											temp_sse=sse
											; Eliminate "bad" data points
											wrong=where(sse eq 0)
											if (wrong(0) ne -1) then begin
												sse[wrong]=1.0
											endif
											; Perform calculations
											if (sWidget.SlidingWindow eq -1) then begin
												fstat=((ssv(lemon)/1.)/(sse/(float(temp-2))))
											endif else begin
												if (temp ge sWidget.SlidingWindow-1) then begin
													fstat=((ssv(lemon)/1.)/(sse/(float(sWidget.SlidingWindow-3))))
												endif else begin
													fstat=((ssv(lemon)/1.)/(sse/(float(temp-2))))
												endelse
											endelse
											if (wrong(0) ne -1) then begin
												fstat[wrong]=0
											endif
											; Put data in output array
											reg_results(*,*,temp-1)=fstat
											; Reset data arrays
											sse=temp_sse
										endif else if ((sWidget.ResultTypeOutput eq 22) and $
											(temp gt 3)) then begin
											; Save initial data for later
											temp_sse=sse
											; Eliminate "bad" data points
											wrong=where(sse eq 0)
											if (wrong(0) ne -1) then begin
												sse[wrong]=1.0
											endif
											; Perform calculations
											works=(((ssv(lime)+ssv(lemon))/2.)/(sse/(float(temp-3))))
											if (wrong(0) ne -1) then begin
												works[wrong]=0
											endif
											; Put data in output array
											reg_results(*,*,temp-1)=works
											; Reset data arrays
											sse=temp_sse
										endif else if (sWidget.ResultTypeOutput eq 23) then begin
											reg_results(*,*,temp-1)= $
												sqrt(((coef(lemon))^2)+((coef(lime))^2))
										endif else if (sWidget.ResultTypeOutput eq 21) then begin
											reg_results(*,*,temp-1)=atan(coef(lime),coef(lemon))
										endif else if ((sWidget.ResultTypeOutput ge 24) AND $
											(sWidget.ResultTypeOutput le 26) AND $
											(temp gt 2))  then begin
											; Save initial data for later
											temp_se = se
											; Eliminate "bad" data points
											wrong = where(se eq 0)
											if (wrong(0) ne -1) then begin
												se[wrong]=1.0
											endif

											; Perform calculations
											tstat = coef(lemon)/se(lemon)
											if (wrong(0) ne -1) then begin
												tstat[wrong]=0
											endif

											; Put data in output array
											reg_results(*,*,temp-1)=tstat

											; Reset data arrays
											se = temp_se										
										endif else begin
											; do nothing
										endelse
	
										if (temp ge 3) then begin
										if ((sWidget.ResultTypeOutput ge 14) $
										    and (temp eq 3)) then begin
											; do nothing this round
										endif else begin
											; Show the output in the result window
											if (sWidget.ScaleMax eq -1) then begin
												number=max(abs(reg_results(*,*,temp-1)))
												hold=bytscl(reg_results(*,*,temp-1) $
													,-number, number)/2+128
											endif else begin
												hold=bytscl(reg_results(*,*,temp-1) $
													,-sWidget.ScaleMax,sWidget.ScaleMax)/2+128
											endelse
											hold=rebin(hold,sWidget.disp_sizex,sWidget.disp_sizey)
											hold=reverse(hold,2)
											tv,hold
										endelse
										endif
									endif
								endif
								
								ImgEvent=0
							endif
							endif
						endfor
					endif

					; Display the appropriate image of the stimulus file
					wset,sWidget.stim_win_id
					;biggie=bytscl(slice2(count))
					;biggie=reverse(biggie,1)
					;biggie=reverse(biggie,2)
					biggie=congrid(biggie,sWidget.disp_sizex,sWidget.disp_sizey);
					tv,slice2(count)

					; If have cycled through the stimulus file once completely, start over again
					; by closing and reopening stimulus file
					count = count + 1
					if (count eq 959) then begin
					;	close, 1
					;	openr, 1, sWidget.StimulusName
					;	slice2=assoc(1,intarr(sWidget.StimSizex,sWidget.StimSizey))
						count=0
					endif

				endwhile
				; Close stimulus file
				close,1
			; Disk case, no stimulus display
			endif else begin
				last_time=0
				while (temp NE (sWidget.NumImg/sWidget.NumSli)) do begin
					; Read in specified number of samples and determine if image	
					; acquisition occurred in elapsed time
					err1=cvio_read_next_samples(img_acq,check_t,check_d,sWidget.Rate/5)
					shift_check=shift(check_d,2)
					shift_check(0)=sec_last
					shift_check(1)=last
					event=where((check_d-shift_check) gt 28000)
					if (event(0) ne -1) then begin
						next_shift=shift(shift_check,1)
						next_shift(0)=third_last
						next_shift=[next_shift,check_d(sWidget.Rate/5-3),check_d(sWidget.Rate/5-2)]
						for b=0,n_elements(event)-1 do begin
							if (abs(next_shift(event(b)+2))-abs(next_shift(event(b))) lt 28000) $
							    then begin
								ImgEvent = ImgEvent + 1
							endif
						endfor
						third_last = sec_last
						sec_last = last
						last = abs(check_d(sWidget.Rate/5-1))
					endif

					; If stripchart display is raw cvio data, read and display the data
					; from the designated cvio file
					if ((sWidget.StripchartTypeOutput ge 1) and $
					    (sWidget.StripchartTypeOutput le 4)) then begin
						; Cardiac has been opened for regression so read from there
						if (card_overlap eq 1) then begin
							err1 = cvio_read_next_samples(cardiac,t1,d1,sWidget.Rate/5)
						; Resp has been opened for regression so read from there
						endif else if (resp_overlap eq 1) then begin
							err1 = cvio_read_next_samples(respiratory,t1,d1,sWidget.Rate/5)
						; Imag Acq opened for regression so read from there
						endif else if (img_overlap eq 1) then begin
							t1 = check_t
							d1 = check_d
							err1 = 0
						; CVIO file not needed for regression but opened for stripchart plotting
						endif else begin
							err1 = cvio_read_next_samples(chart_type,t1,d1,sWidget.Rate/5)
						endelse

						; If have proper amount of data, plot it
						if (err1 eq 0) then begin
							wset,sWidget.chart_win_id
							if (check eq 2) then begin
								oplot,d4,color=0
								check = 1
							endif
							if (check EQ 0) then begin
								plot,d1,xtitle='No. Images',POS=[0.2,0.2,0.85,0.85],title=label, $
									charsize=0.5,color=127,yrange=[-34000,34000],xstyle=4+1, $
									ystyle=8
								check = 2
								d4 = d1
							endif
							if (check EQ 1) then begin
								oplot,d1,color=127
								check = 2
								d4 = d1
							endif
						endif
					endif

					; If have collected enough slices to be one image, proceed
					if (ImgEvent gt (sWidget.NumSli-1)) then begin
						spot = 0
						for u=1,sWidget.NumSli do begin
							; Read in image data
							err2 = cvio_read_next_samples(tagged_image,t2,d2,1)
							; If sequential collection, store and display image data
							if ((u eq sWidget.GetPicture) and (sWidget.Order eq 0)) then begin
								seq_picture=d2
								holder=d2
								wset,sWidget.orig_win_id
								Final_Raw_Image=bytscl(holder)/2
								holder=rebin(holder,sWidget.disp_sizex,sWidget.disp_sizey)
								holder=reverse(holder,2)
								if (sWidget.StripchartTypeOutput eq 20) then begin
									mean_chart_results(temp)=mean(d2(sWidget.Comp_Store_ROI))
								endif
								tv,bytscl(holder)/2
							endif
							; If this slice is the one to be regressed, display original, gather
							; raw cvio data and place it in necessary arrays for regression
							if (u eq sWidget.RegressSlice) then begin
								if (sWidget.Order eq 1) then begin
									seq_picture = d2
									holder=d2
									wset,sWidget.orig_win_id
									Final_Raw_Image=bytscl(holder)/2
									holder=rebin(holder,sWidget.disp_sizex,sWidget.disp_sizey)
									holder=reverse(holder,2)
									if (sWidget.StripchartTypeOutput eq 20) then begin
										mean_chart_results(temp)=mean(d2(sWidget.Comp_Store_ROI))
									endif
									tv,bytscl(holder)/2
								endif

								; Store timestamp of this image
								if (sWidget.Save eq 1) then begin
									output_time(temp)=t2
								endif
								; Keep track of number of images collected
								temp=temp + 1
								this_time=systime(1)
								print, temp, this_time-last_time
								last_time=this_time
								if (temp eq 4) then begin
									motion_picture=float(d2)
								endif
								; Put array of constant in regression array
								if (sWidget.Constant eq 1) then begin
									hold_dependent(*,*,0)=1.0
									spot = spot + 1
								endif
								; Put cardiac data in regression array if necessary
								if (sWidget.Cardiac_Regress eq 1) then begin
									if (sWidget.Card_Cos eq 1) then begin
										err=cvio_read_samples(card_table2,t2,card_data2,1)
										;print, 'cos card data = ',card_data2
										hold_dependent(*,*,spot)=float(card_data2)
										spot = spot + 1
									endif
									if (sWidget.Card_Sin eq 1) then begin
										err=cvio_read_samples(card_table3,t2,card_data3,1)
										;print, 'sin card data = ',card_data3
										hold_dependent(*,*,spot)=float(card_data3)
										spot = spot + 1
									endif
									;process card_data if necessary
									;hold_dependent(*,*,spot)=float(card_data)
									;spot = spot + 1
								endif
								; Put respiratory data in regression array if necessary
								if (sWidget.Respiratory_Regress eq 1) then begin
									if (sWidget.Resp_None eq 1) then begin
										err=cvio_read_samples(resp_table1,t2,resp_data,1)
										;print, 'resp sample = ',resp_data
										hold_dependent(*,*,spot)=resp_data
										spot = spot + 1
									endif
									;junk_time(0)=t2-(5*(20000L/sWidget.Rate))
									;junk_time(1:*)=next
									;err=cvio_read_samples(respiratory,junk_time, $
									;	resp_data,n_elements(junk_time))
									;med_resp = median(resp_data)
									;hold_dependent(*,*,spot)=med_resp
									;spot = spot + 1
								endif
								; Put stimulus data in regression array if necessary
								if (sWidget.Stimulus_Regress eq 1) then begin
									distance = t2 - first_time
									position = ((distance MOD (sWidget.Period*20000L))+1)/20
									if (sWidget.Stim_Conv eq 1) then begin
										err=cvio_read_samples(l_table1,position,hrf_data,1)
										hold_dependent(*,*,spot)=hrf_data
										if (sWidget.StripchartTypeOutput eq 20) then begin
											plot_hrf(temp-1)=hrf_data
										endif
										spot = spot + 1
									endif
									if (sWidget.Stim_Sin eq 1) then begin
										err=cvio_read_samples(l_table2,position,hrf_data,1)
										hold_dependent(*,*,spot)=hrf_data
										if (sWidget.StripchartTypeOutput eq 20) then begin
											plot_hrf(temp-1)=hrf_data
										endif
										spot = spot + 1
									endif
									if (sWidget.Stim_Cos eq 1) then begin
										err=cvio_read_samples(l_table3,position,hrf_data,1)
										hold_dependent(*,*,spot)=hrf_data
										if (sWidget.StripchartTypeOutput eq 20) then begin
											plot_hrf(temp-1)=hrf_data
										endif
										spot = spot + 1
									endif
								endif
								; Put array of trending in regression array
								if (sWidget.Trending eq 1) then begin
									err=cvio_read_samples(trend_table,t2,trend_data,1)
									;print, "trend data = ",trend_data
									hold_dependent(*,*,spot)=trend_data
									spot = spot + 1
								endif
								; Put array of global average in regression array
								if (sWidget.Global eq 1) then begin
									err=cvio_read_samples(global_table,t2,global_data,1)
									;print, "global data = ",global_data
									hold_dependent(*,*,spot)=global_data
									spot = spot + 1
								endif
							endif

							if (((u ge sWidget.GetPicture) and (u eq sWidget.RegressSlice)) or $
								((u eq sWidget.GetPicture) and (u ge sWidget.RegressSlice))) then begin
								; Discard first image from processing
								if (temp gt 1) then begin
									; Add data to regression arrays and perform regression calculations
									if ((sWidget.Cardiac_Regress eq 1) OR $
							    		(sWidget.Stimulus_Regress eq 1) OR $
				    				   	(sWidget.Respiratory_Regress eq 1)) then begin
										hold_independent = float(seq_picture)
										if (sWidget.SlidingWindow ne -1) then begin
											if (WindowIndex lt sWidget.SlidingWindow) then begin
												status=reg_add_obs(reg,hold_independent,hold_dependent)
												store_independent(*,*,WindowIndex)=hold_independent
												store_dependent(*,*,*,WindowIndex)=hold_dependent
												WindowIndex=WindowIndex+1
											endif else begin
												status=reg_add_obs(reg,hold_independent,hold_dependent)
												lose_independent=store_independent(*,*,0)
												lose_dependent=store_dependent(*,*,*,0)
												status=reg_remove_obs(reg,lose_independent, $
													lose_dependent)
												store_independent(0,0,0)=store_independent(*,*, $
													1:sWidget.SlidingWindow-1)
												store_dependent(0,0,0,0)=store_dependent(*,*,*, $
													1:sWidget.SlidingWindow-1)
												store_independent(*,*,WindowIndex-1)=hold_independent
												store_dependent(*,*,*,WindowIndex-1)=hold_dependent
											endelse
										endif else begin
											status=reg_add_obs(reg,hold_independent,hold_dependent)
										endelse
								
										;this_time=systime(1)
										;print, "data input = ", this_time-last_time
										;last_time=this_time

										; If have ROI simplify regression
										if (sWidget.Mask eq 1) then begin
											status = reg_calc_ss(reg,mask,0,sst,sse,ssv)
											if (((sWidget.ResultTypeOutput ge 24) and $
												(sWidget.ResultTypeOutput le 26)) or $
												((sWidget.StripchartTypeOutput ge 24) and $
												(sWidget.StripchartTypeOutput le 29))) then begin
												status = reg_calc_coef(reg,mask,coef,STD_ERROR=se)
											endif else begin
												status = reg_calc_coef(reg,mask,coef)
											endelse												
										endif else begin
											status = reg_calc_ss(reg,0,sst,sse,ssv)
											if (((sWidget.ResultTypeOutput ge 24) and $
												(sWidget.ResultTypeOutput le 26)) or $
												((sWidget.StripchartTypeOutput ge 24) and $
												(sWidget.StripchartTypeOutput le 29))) then begin
												status = reg_calc_coef(reg,coef,STD_ERROR=se)
											endif else begin
												status = reg_calc_coef(reg,coef)
											endelse
										endelse
								
										;this_time=systime(1)
										;print, "calculations = ", this_time-last_time
										;last_time=this_time

										; Based on user input gather specified data
										; and perform the calculations for stripchart
										if (sWidget.StripchartTypeOutput ge 5) then begin
										; Set active window to stripchart display
										wset,sWidget.chart_win_id
										; Based on user input, create array which will
										; plot proper output from regression
										if ((sWidget.StripchartTypeOutput ge 5) $
										    AND (sWidget.StripchartTypeOutput le 7)) then begin
											very=ssv(nice)
											chart_results(temp-1)=max(very)
											min_chart_results(temp-1)=min(very)
											if (temp eq 2) then begin
												chart_title='Sum of Squares'
												y_range = [0,500000]
											endif
										endif else if ((sWidget.StripchartTypeOutput ge 8) $
										    AND (sWidget.StripchartTypeOutput le 10)) then begin
											very=coef(nice)
											chart_results(temp-1)=max(very)
											min_chart_results(temp-1)=min(very)

											if (temp eq 2) then begin
												chart_title = 'Regression Coefficient'
												y_range=[-600,600]
											endif
										endif else if ((sWidget.StripchartTypeOutput ge 11) $
										    AND (sWidget.StripchartTypeOutput le 13)) then begin
											;effect_size=
											;chart_results(temp-1)=effect_size
											if (temp eq 2) then begin
												chart_title='Effect Size'
											endif
										endif else if ((sWidget.StripchartTypeOutput ge 14) $
										    AND (sWidget.StripchartTypeOutput le 16) $
										    AND (temp gt 2)) then begin
											; Save initial data for later
											temp_sse=sse
											temp_coef=coef
											; Eliminate "bad" data points
											valid=where(sse ne 0)
											wrong=where(sse eq 0)
											bad_coef=where(coef(nice) eq 0)
											good_coef=where(coef(nice) ne 0)
											if (bad_coef(0) ne -1) then begin
												coef[bad_coef]=1.0
											endif
											if (wrong(0) ne -1) then begin
												sse[wrong]=1.0
											endif
											; Perform calculations
											if (sWidget.SlidingWindow eq -1) then begin
												tstat=(sqrt((ssv(nice)/1.)/(sse/(float(temp-2)))) $
												     *(abs(coef(nice))/coef(nice)))
											endif else begin
												if (temp ge sWidget.SlidingWindow-1) then begin
													tstat=(sqrt((ssv(nice)/1.)/(sse/(float(sWidget.SlidingWindow-3)))) $
													     *(abs(coef(nice))/coef(nice)))
												endif else begin
													tstat=(sqrt((ssv(nice)/1.)/(sse/(float(temp-2)))) $
													     *(abs(coef(nice))/coef(nice)))
												endelse
											endelse
											; Put data in output array
											if ((good_coef(0) ne -1) and (valid(0) ne -1)) then begin
												; Assuming t-statistic greater 0 for plotting
												if (bad_coef(0) ne -1) then begin
													tstat[bad_coef]=0
												endif
												if (wrong(0) ne -1) then begin
													tstat[wrong]=0
												endif
												max1=max(tstat[good_coef])
												min1=min(tstat[good_coef])
												max2=max(tstat[valid])
												min2=min(tstat[valid])
												if (max2 gt max1) then begin
													chart_results(temp-1)=max2
												endif else begin
													chart_results(temp-1)=max1
												endelse
												if (min1 lt min2) then begin
													min_chart_results(temp-1)=min1
												endif else begin
													min_chart_results(temp-1)=min2
												endelse 
											endif else begin
												chart_results(temp-1)=0
												min_chart_results(temp-1)=0
											endelse
											if (temp eq 3) then begin
												chart_title='t-Statistic'
												y_range=[-12,12]
											endif
											; Reset data arrays
											sse=temp_sse
											coef=temp_coef
										endif else if ((sWidget.StripchartTypeOutput ge 17) $
										    AND (sWidget.StripchartTypeOutput le 19) $
										    AND (temp gt 2)) then begin
											; Save initial data for later
											temp_sse=sse
											; Eliminate "bad" data points
											valid=where(sse ne 0)
											wrong=where(sse eq 0)
											if (wrong(0) ne -1) then begin
												sse[wrong]=1.0
											endif
											; Perform necessary calculations
											if (sWidget.SlidingWindow eq -1) then begin
												fstat=((ssv(nice)/1.)/(sse/(float(temp-2))))
											endif else begin
												if (temp ge sWidget.SlidingWindow-1) then begin
													fstat=((ssv(nice)/1.)/(sse/(float(sWidget.SlidingWindow-3))))
												endif else begin
													fstat=((ssv(nice)/1.)/(sse/(float(temp-2))))
												endelse
											endelse
											; Put data in output array
											if (valid(0) ne -1) then begin
											    chart_results(temp-1)=max(fstat[valid])
											    min_chart_results(temp-1)=min(fstat[valid])
											endif else begin
												chart_results(temp-1)=0
												min_chart_results(temp-1)=0
											endelse
											if (temp eq 3) then begin
												chart_title='F-Statistic'
												y_range = [0,120]
											endif
											; Reset data arrays
											sse=temp_sse
										endif else if ((sWidget.StripchartTypeOutput eq 21) $
											and (temp gt 4)) then begin
											total_chart_results(temp-1)= $
												total(abs(motion_picture-float(seq_picture))) $
												/(sWidget.orig_sizex*sWidget.orig_sizey)
										endif else if ((sWidget.StripchartTypeOutput eq 22) and $
											(temp gt 3)) then begin
											; Save initial data for later
											temp_sse=sse
											; Eliminate "bad" data points
											valid=where(sse ne 0)
											wrong=where(sse eq 0)
											if (wrong(0) ne -1) then begin
												sse[wrong]=1.0
											endif
											; Perform necessary calculations
											works=(((ssv(nice)+ssv(sugar))/2.)/(sse/(float(temp-3))))
											; Put data in output array
											if (valid(0) ne -1) then begin
											    chart_results(temp-1)=max(works[valid])
											    min_chart_results(temp-1)=min(works[valid])
											endif else begin
												chart_results(temp-1)=0
												min_chart_results(temp-1)=0
											endelse
											if (temp eq 4) then begin
												chart_title='Sin and Cos t-stat'
												y_range = [0,25]
											endif
											; Reset data arrays
											sse=temp_sse
										endif else if (sWidget.StripchartTypeOutput eq 23) $
											then begin
											; Perform necessary calculations
											event=sqrt(((coef(nice))^2)+((coef(sugar))^2))
											; Put data in output array
											chart_results(temp-1)=max(event)
											min_chart_results(temp-1)=min(event)
											if (temp eq 2) then begin
												chart_title='Sin and Cos Magnitude'
												y_range = [0,120]
											endif
										endif else if ((sWidget.StripchartTypeOutput ge 24) $
										    AND (sWidget.StripchartTypeOutput le 26)) then begin

											; Save initial data for later
											temp_se = se

											err_arr = se(nice)
											very = coef(nice)
											
											; Eliminate "bad" data points
											valid = where(err_arr ne 0)
											wrong = where(err_arr eq 0)
											if (wrong(0) ne -1) then begin
												err_arr[wrong]=1.0
											endif

											;print, "max coef = ",max(very)
											;print, "max error = ",max(err_arr)											
									
											; Perform calculations
											tstat = very/err_arr
											if (wrong(0) ne -1) then begin
												tstat[wrong]=0
											endif

											big_t = max(tstat)
											;print, "high t = ",big_t
											little_t = min(tstat)
											;print, "low t = ", little_t
											big_spot = where(tstat eq big_t)
											little_spot = where(tstat eq little_t)

											if ((big_spot(0) ne -1) and (little_spot(0) ne -1)) then begin
												chart_results(temp-1)=very[big_spot(0)]
												;print, "high = ",chart_results(temp-1)
												min_chart_results(temp-1)=very[little_spot(0)]
												;print, "low = ",min_chart_results(temp-1)
												max_plus_error(temp-1)=very[big_spot(0)]+sWidget.ConfidenceInterval*err_arr[big_spot(0)]
												max_minus_error(temp-1)=very[big_spot(0)]-sWidget.ConfidenceInterval*err_arr[big_spot(0)]
												min_plus_error(temp-1)=very[little_spot(0)]+sWidget.ConfidenceInterval*err_arr[little_spot(0)]
												min_minus_error(temp-1)=very[little_spot(0)]-sWidget.ConfidenceInterval*err_arr[little_spot(0)]
												;print, "max error = ", err_arr[big_spot(0)]
												;print, "max plus error = ",max_plus_error(temp-1)
												;print, "max minus error = ",max_minus_error(temp-1)
												;print, "min error = ", err_arr[little_spot(0)]
												;print, "min plus error = ",min_plus_error(temp-1)
												;print, "min minus error = ",min_minus_error(temp-1)
											endif else begin
												max_plus_error(temp-1)=0
												max_minus_error(temp-1)=0
												min_plus_error(temp-1)=0
												min_minus_error(temp-1)=0													
											endelse

											if (temp eq 2) then begin
												chart_title = 'Regression Coefficient with Error'
												y_range=[-200,200]
											endif

											se = temp_se
										endif else if ((sWidget.StripchartTypeOutput ge 27) AND $
											(sWidget.StripchartTypeOutput le 29) AND $
											(temp gt 2)) then begin
											; Save initial data for later
											temp_se = se

											; Eliminate "bad" data points
											valid = where(se ne 0)
											wrong = where(se eq 0)
											if (wrong(0) ne -1) then begin
												se[wrong]=1.0
											endif

											; Perform calculations
											tstat = coef(nice)/se(nice)
											if (wrong(0) ne -1) then begin
												tstat[wrong]=0
											endif

											if (valid(0) ne -1) then begin
												chart_results(temp-1) = max(tstat[valid])
												min_chart_results(temp-1) = min(tstat[valid])
											endif else begin
												chart_results(temp-1) = 0
												min_chart_results(temp-1) = 0
											endelse

											if (temp eq 3) then begin
												chart_title='Alternative t-Statistic'
												y_range=[-12,12]
											endif

											; Reset data arrays
											se = temp_se
										endif else begin
											; do nothing
										endelse
										
										; Plot the data in the stripchart
										if ((temp eq 1) or (temp eq 2)) then begin
											; do nothing waiting for good data
										; create initial plot
										endif else if (temp eq 3) then begin
											if ((sWidget.StripchartTypeOutput lt 20) or $
												(sWidget.StripchartTypeOutput eq 23) or $
												((sWidget.StripchartTypeOutput ge 27) and $
												 (sWidget.StripchartTypeOutput le 29))) then begin
												time=indgen((sWidget.NumImg/sWidget.NumSli))+1
												plot,time,chart_results(0:temp-1),xtitle='No. Images',$
												 	POS=[0.2,0.2,0.85,0.85],title=chart_title, $
												  	charsize=0.5,xstyle=1,ystyle=8,color=127, $
													xrange=[0,(sWidget.NumImg/sWidget.NumSli)], $
													yrange=y_range
												last_trip=chart_results(0:temp-1)
												oplot,time,min_chart_results(0:temp-1),color=128
												last_min_trip=min_chart_results(0:temp-1)
												oplot,time,zero,color=127
											endif else if (sWidget.StripchartTypeOutput eq 20) $
												then begin
												time=indgen((sWidget.NumImg/sWidget.NumSli))+1
												plot,time,mean_chart_results(0:temp-1),xtitle='No. Images',$
												 	POS=[0.2,0.2,0.85,0.85],title=chart_title, $
												  	charsize=0.5,xstyle=1,ystyle=8,color=127, $
													xrange=[0,(sWidget.NumImg/sWidget.NumSli)]
												last_mean_trip=mean_chart_results(0:temp-1)
												oplot,time,plot_hrf(0:temp-1),color=255
												last_hrf_trip=plot_hrf(0:temp-1)
											endif else if ((sWidget.StripchartTypeOutput le 26) and $
												(sWidget.StripchartTypeOutput ge 24)) then begin
												time=indgen((sWidget.NumImg/sWidget.NumSli))+1
												plot,time,chart_results(0:temp-1),xtitle='No. Images',$
												 	POS=[0.2,0.2,0.85,0.85],title=chart_title, $
												  	charsize=0.5,xstyle=1,ystyle=8,color=127, $
													xrange=[0,(sWidget.NumImg/sWidget.NumSli)], $
													yrange=y_range
												last_trip=chart_results(0:temp-1)
												oplot,time,min_chart_results(0:temp-1),color=127
												oplot,time,max_plus_error(0:temp-1),color=128
												oplot,time,max_minus_error(0:temp-1),color=128
												oplot,time,min_plus_error(0:temp-1),color=128
												oplot,time,min_minus_error(0:temp-1),color=128
												last_min_trip=min_chart_results(0:temp-1)
												last_max_plus=max_plus_error(0:temp-1)
												last_max_minus=max_minus_error(0:temp-1)
												last_min_plus=min_plus_error(0:temp-1)
												last_min_minus=min_minus_error(0:temp-1)
												oplot,time,zero,color=127
											endif else begin
												; do nothing
											endelse
										; plot over old stuff, plot new stuff
										endif else begin
											if ((sWidget.StripchartTypeOutput lt 20) or $
												(sWidget.StripchartTypeOutput eq 23) or $
												((sWidget.StripchartTypeOutput ge 27) and $
												 (sWidget.StripchartTypeOutput le 29))) then begin
										    	oplot,time,last_trip,color=0
										    	oplot,time,chart_results(0:temp-1),color=255
										    	last_trip=chart_results(0:temp-1)
										    	oplot,time,last_min_trip,color=0
										    	oplot,time,min_chart_results(0:temp-1),color=128
										    	last_min_trip=min_chart_results(0:temp-1)
											endif else if ((sWidget.StripchartTypeOutput le 26) and $
												(sWidget.StripchartTypeOutput ge 24)) then begin
										    	oplot,time,last_trip,color=0
										    	oplot,time,chart_results(0:temp-1),color=255
										    	last_trip=chart_results(0:temp-1)
										    	oplot,time,last_min_trip,color=0
										    	oplot,time,min_chart_results(0:temp-1),color=255
										    	last_min_trip=min_chart_results(0:temp-1)
										    	oplot,time,last_max_plus,color=0
										    	oplot,time,max_plus_error(0:temp-1),color=128
										    	last_max_plus=max_plus_error(0:temp-1)
										    	oplot,time,last_max_minus,color=0
										    	oplot,time,max_minus_error(0:temp-1),color=128
										    	last_max_minus=max_minus_error(0:temp-1)
										    	oplot,time,last_min_plus,color=0
										    	oplot,time,min_plus_error(0:temp-1),color=128
										    	last_min_plus=min_plus_error(0:temp-1)
										    	oplot,time,last_min_minus,color=0
										    	oplot,time,min_minus_error(0:temp-1),color=128
										    	last_min_minus=min_minus_error(0:temp-1)
											endif else if (sWidget.StripchartTypeOutput eq 20) $
												then begin
										    	oplot,time,last_mean_trip,color=0
										    	oplot,time,mean_chart_results(0:temp-1),color=128
										    	last_mean_trip=mean_chart_results(0:temp-1)
										    	oplot,time,last_hrf_trip,color=0
												oplot,time,plot_hrf(0:temp-1),color=255
												last_hrf_trip=plot_hrf(0:temp-1)
											endif else if (sWidget.StripchartTypeOutput eq 22) $
												then begin
												if (temp eq 4) then begin
													time=indgen((sWidget.NumImg/sWidget.NumSli))+1
													plot,time,chart_results(0:temp-1),xtitle='No. Images',$
														POS=[0.2,0.2,0.85,0.85],title=chart_title, $
														charsize=0.5,xstyle=1,ystyle=8,color=127, $
														xrange=[0,(sWidget.NumImg/sWidget.NumSli)], $
														yrange=y_range
													last_trip=chart_results(0:temp-1)
													oplot,time,min_chart_results(0:temp-1),color=128
													last_min_trip=min_chart_results(0:temp-1)
													oplot,time,zero,color=127
												endif else begin
										    		oplot,time,last_trip,color=0
										    		oplot,time,chart_results(0:temp-1),color=255
										    		last_trip=chart_results(0:temp-1)
										    		oplot,time,last_min_trip,color=0
										    		oplot,time,min_chart_results(0:temp-1),color=128
										    		last_min_trip=min_chart_results(0:temp-1)
												endelse
											endif else begin
												if (temp eq 5) then begin
													time=indgen((sWidget.NumImg/sWidget.NumSli))+1
													plot,time,total_chart_results(0:temp-1),xtitle='No. Images',$
												 		POS=[0.2,0.2,0.85,0.85],title=chart_title, $
												  		charsize=0.5,xstyle=1,ystyle=8,color=127, $
														xrange=[0,(sWidget.NumImg/sWidget.NumSli)], $
														yrange=[0,100]
													last_total_trip=total_chart_results(0:temp-1)
												endif else if (temp gt 5) then begin
													time=indgen((sWidget.NumImg/sWidget.NumSli))+1
									    			oplot,time,last_total_trip,color=0
									    			oplot,time,total_chart_results(0:temp-1),color=128
									    			last_total_trip=total_chart_results(0:temp-1)
												endif else begin
													; do nothing
												endelse
											endelse
										endelse
										endif

										; Based on user input, gather specified data and perform 
										; calculations for output display
										; Set active window to output display
										wset,sWidget.result_win_id
										if ((sWidget.ResultTypeOutput ge 5) AND $
										    (sWidget.ResultTypeOutput le 7)) then begin
											reg_results(*,*,temp-1)=ssv(lemon)
										endif else if ((sWidget.ResultTypeOutput ge 8) $
										    AND (sWidget.ResultTypeOutput le 10)) then begin
											reg_results(*,*,temp-1)=coef(lemon)
										endif else if ((sWidget.ResultTypeOutput ge 11) $
										    AND (sWidget.ResultTypeOutput le 13)) then begin
											;effect_size=
											;reg_results(*,*,temp-1)=effect_size
										endif else if ((sWidget.ResultTypeOutput ge 14) $
										    AND (sWidget.ResultTypeOutput le 16) $
										    AND (temp gt 2)) then begin
											; Save initial data for later
											temp_sse=sse
											temp_coef=coef
											; Eliminate "bad" data points
											wrong=where(sse eq 0)
											bad_coef=(coef eq 0)
											if (wrong(0) ne -1) then begin
												sse[wrong]=1.0
											endif
											if (bad_coef(0) ne -1) then begin
												coef[bad_coef]=1.0
											endif
											; Perform calculations
											if (sWidget.SlidingWindow eq -1) then begin
												tstat=(sqrt((ssv(lemon)/1.)/(sse/(float(temp-2)))) $
												     *(abs(coef(lemon))/coef(lemon)))
											endif else begin
												if (temp ge sWidget.SlidingWindow-1) then begin
													tstat=(sqrt((ssv(lemon)/1.)/(sse/(float(sWidget.SlidingWindow-3)))) $
													     *(abs(coef(lemon))/coef(lemon)))
												endif else begin
													tstat=(sqrt((ssv(lemon)/1.)/(sse/(float(temp-2)))) $
													     *(abs(coef(lemon))/coef(lemon)))
												endelse
											endelse
											if (wrong(0) ne -1) then begin
												tstat[wrong]=0
											endif
											if (bad_coef(0) ne -1) then begin
												tstat[bad_coef]=0
											endif
											; Put data in output array
											reg_results(*,*,temp-1)=tstat
											; Reset data arrays
											sse=temp_sse
											coef=temp_coef
										endif else if ((sWidget.ResultTypeOutput ge 17) $
										    AND (sWidget.ResultTypeOutput le 19) $
										    AND (temp gt 2)) then begin
											; Save initial data for later
											temp_sse=sse
											; Eliminate "bad" data points
											wrong=where(sse eq 0)
											if (wrong(0) ne -1) then begin
												sse[wrong]=1.0
											endif
											; Perform calculations
											if (sWidget.SlidingWindow eq -1) then begin
												fstat=((ssv(lemon)/1.)/(sse/(float(temp-2))))
											endif else begin
												if (temp ge sWidget.SlidingWindow-1) then begin
													fstat=((ssv(lemon)/1.)/(sse/(float(sWidget.SlidingWindow-3))))
												endif else begin
													fstat=((ssv(lemon)/1.)/(sse/(float(temp-2))))
												endelse
											endelse
											if (wrong(0) ne -1) then begin
												fstat[wrong]=0
											endif
											; Put data in output array
											reg_results(*,*,temp-1)=fstat
											; Reset data arrays
											sse=temp_sse
										endif else if ((sWidget.ResultTypeOutput eq 22) and $
											(temp gt 3)) then begin
											; Save initial data for later
											temp_sse=sse
											; Eliminate "bad" data points
											wrong=where(sse eq 0)
											if (wrong(0) ne -1) then begin
												sse[wrong]=1.0
											endif
											; Perform calculations
											works=(((ssv(lime)+ssv(lemon))/2.)/(sse/(float(temp-3))))
											if (wrong(0) ne -1) then begin
												works[wrong]=0
											endif
											; Put data in output array
											reg_results(*,*,temp-1)=works
											; Reset data arrays
											sse=temp_sse
										endif else if (sWidget.ResultTypeOutput eq 23) then begin
											reg_results(*,*,temp-1)= $
												sqrt(((coef(lemon))^2)+((coef(lime))^2))
										endif else if (sWidget.ResultTypeOutput eq 21) then begin
											store=coef(lemon)
											pile=coef(lemon)
											garbage=where(pile eq 0)
											if (garbage(0) ne -1) then begin
												pile(garbage)=1.0
											endif
											reg_results(*,*,temp-1)=atan(coef(lime),pile)
											if (garbage(0) ne -1) then begin
												reg_results(garbage,temp-1)=0
											endif
											coef(lemon)=store
										endif else if ((sWidget.ResultTypeOutput ge 24) AND $
											(sWidget.ResultTypeOutput le 26) AND $
											(temp gt 2)) then begin
											; Save initial data for later
											temp_se = se
											; Eliminate "bad" data points
											wrong = where(se eq 0)
											if (wrong(0) ne -1) then begin
												se[wrong]=1.0
											endif

											; Perform calculations
											tstat = coef(lemon)/se(lemon)
											if (wrong(0) ne -1) then begin
												tstat[wrong]=0
											endif

											; Put data in output array
											reg_results(*,*,temp-1)=tstat

											; Reset data arrays
											se = temp_se
										endif else begin
											; do nothing
										endelse
	
										if (temp ge 3) then begin
										if ((sWidget.ResultTypeOutput ge 14) $
										    and (temp eq 3)) then begin
											; do nothing this round
										endif else begin
											; Show the output in the result window
											if (sWidget.ScaleMax eq -1) then begin
												number=max(abs(reg_results(*,*,temp-1)))
												hold=bytscl(reg_results(*,*,temp-1) $
													,-number, number)/2+128
											endif else begin
												hold=bytscl(reg_results(*,*,temp-1) $
													,-sWidget.ScaleMax,sWidget.ScaleMax)/2+128
											endelse
											hold=rebin(hold,sWidget.disp_sizex,sWidget.disp_sizey)
											hold=reverse(hold,2)
											tv,hold

											if (sWidget.Overlay ne -1) then begin
												wset,sWidget.stim_win_id
												Final_Raw_Image=rebin(Final_Raw_Image,sWidget.disp_sizex,sWidget.disp_sizey)
												temp_image=rebin(reg_results(*,*,temp-1),sWidget.disp_sizex,sWidget.disp_sizey)
												low_values = where(abs(temp_image) lt sWidget.Overlay)
												number=max(abs(temp_image))
												new_processed=bytscl(temp_image,-number, number)/2+128
												overlay_image = new_processed
												overlay_image(low_values) = Final_Raw_Image(low_values)
												overlay_image=reverse(overlay_image,2)
												tv,overlay_image
											endif
											
										endelse
										endif
									endif
								endif
								
								ImgEvent=0
							endif
						endfor
					endif
				endwhile
			endelse
		endelse

		sWidget.Final_Image(0:sWidget.orig_sizex-1,0:sWidget.orig_sizey-1)=reg_results(*,*,temp-1)

		if (sWidget.Source eq 0) then begin
			headerInfo2, sWidget
		endif

		if (sWidget.Save eq 1) then begin
			; Write inputs and outputs to specified files using file type-specific commands
			if (sWidget.OutputFileType EQ 'Static') then begin
				; Call function to write user inputs to designated output files
				; Create and open files to store the ouput
				OPENW,20,sWidget.output_dir+sWidget.OriginalOutputFile
				OPENW,21,sWidget.output_dir+sWidget.OriginalOutputFile+'_time'
				; Write the outputs to the file
				WRITEU,20,reg_results
				WRITEU,21,output_time
				; Close the files
				CLOSE,20
				CLOSE,21
				; Update the user on status
				WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Information Successfully Saved'
			endif else begin
				; Call function to write user inputs to designated output files
				; Create and open files to store the ouput
				err=CVIO_CREATE(sWidget.output_dir+sWidget.OriginalOutputFile, $
					(sWidget.NumImg/sWidget.NumSli), $
					32,2,[sWidget.orig_sizex,sWidget.orig_sizey],0)
				print,"save create err = ",err
				err=CVIO_OPEN(sWidget.output_dir+sWidget.OriginalOutputFile,2,write_out)
				print,"save open err = ",err
				; Write the outputs to the file
				err = CVIO_ADD_SAMPLES(write_out,output_time,reg_results, $
					(sWidget.NumImg/sWidget.NumSli))
				print,"save add err = ",err
				; Close the files
				err = CVIO_CLOSE(write_out)
				print,"save close err = ",err
				; Update the cvio header file with all of the entered input
				headerInfo, sWidget
				; Update the user on status
				WIDGET_CONTROL, sWidget.wUpdateText, SET_VALUE = 'Information Successfully Saved'
			endelse
		endif

		; Close all of the cvio files which were opened for the run	
		if (sWidget.Cardiac_Regress eq 1) then begin
			if (sWidget.Card_Conv eq 1) then begin
				err=cvio_close(card_table1)
				print, "card table close = ",err
			endif
			if (sWidget.Card_Cos eq 1) then begin
				err=cvio_close(card_table2)
				print, "card table close = ",err
			endif
			if (sWidget.Card_Sin eq 1) then begin
				err=cvio_close(card_table3)
				print, "card table close = ",err
			endif
			;err=cvio_close(cardiac)
			;print, "card close = ",err
		endif
		if (sWidget.Respiratory_Regress eq 1) then begin
			err=cvio_close(resp_table1)
			print, "resp close = ",err
		endif
		if (sWidget.Stimulus_Regress eq 1) then begin
			if (sWidget.Stim_Sin eq 1) then begin
				err=cvio_close(l_table2)
				print, "table stim close = ",err
			endif
			if (sWidget.Stim_Cos eq 1) then begin
				err=cvio_close(l_table3)
				print, "table stim close = ",err
			endif
			;if (sWidget.Source eq 0) then begin
			;	err=cvio_close(stimulus)
			;	print, "shmem stim close = ",err
			;endif
		endif
		if (sWidget.Trending eq 1) then begin
			err=cvio_close(trend_table)
			print, "trend close = ", err
		endif
		if (sWidget.Global eq 1) then begin
			err=cvio_close(global_table)
			print, "global close = ", err
		endif
		if ((card_overlap eq -1) and (resp_overlap eq -1) and (img_overlap eq -1) and $
		    (sWidget.StripchartTypeOutput ge 1) and (sWidget.StripchartTypeOutput le 4)) $
		    then begin
			err=cvio_close(chart_type)
			print, "chart close = ",err
		endif
		if (sWidget.Source eq 1) then begin
			err = cvio_close(img_acq)
		endif
		err=cvio_close(tagged_image)
		print, "tag close = ",err

		; Destroy the regression
		if ((sWidget.Cardiac_Regress eq 1) OR (sWidget.Stimulus_Regress eq 1) OR $
		    (sWidget.Respiratory_Regress eq 1)) then begin
			status = reg_destroy(reg)
			print, "Destroy: ",status
		endif

		; Now that have performed regression, change appropriate program variables
		sWidget.Already_Regressed=1

		; Deactivate the go button
		WIDGET_CONTROL,sWidget.wGoButton,SENSITIVE=0
		WIDGET_CONTROL,sWidget.wUpdateText,SET_VALUE='Run Successfully Completed'
	END

	; Stop the procedure
	sWidget.wStopbutton : begin
		stop
	END
	
	; Exit IDL
	sWidget.wContinueButton : begin
		exit
	END

	sWidget.wClearButton: begin
		; Clear all four of the display windows in the interface
		wset,sWidget.orig_win_id
		erase
		wset,sWidget.stim_win_id
		erase
		wset,sWidget.result_win_id
		erase
		wset,sWidget.chart_win_id
		erase
	end

	sWidget.wTrackerButton : begin
		wset,sWidget.result_win_id
		device, cursor_standard=68
		rdpix, reverse(rebin(sWidget.Final_Image(0:sWidget.orig_sizex-1,0:sWidget.orig_sizey-1), $
			sWidget.disp_sizex,sWidget.disp_sizey),2)
		device, cursor_standard=34
	end

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
 	testcolors

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
;	PURPOSE: simplify activating cardiac labels
;
;--------------------------------------------------------------------------------
pro cardActive, sWidget

	; Activate all of the cardiac buttons
	WIDGET_CONTROL, sWidget.wOSOSCardButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wCSOSCardButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wORegCoefCardButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wCRegCoefCardButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wOEffectCardButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wCEffectCardButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wOFstatCardButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wCFstatCardButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wOTstatCardButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wCTstatCardButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wOErrorCoefCardButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wCErrorCoefCardButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wCErrorTstatCardButton, SENSITIVE=1
	
end ; of function CardActive

;--------------------------------------------------------------------------------
;
;	PURPOSE: simplify resetting values cardiac labels
;
;--------------------------------------------------------------------------------
pro cardReset, sWidget

	; Reset all cardiac buttons to original value
	WIDGET_CONTROL, sWidget.wOSOSCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wCSOSCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wORegCoefCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wCRegCoefCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wOEffectCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wCEffectCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wOFstatCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wCFstatCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wOTstatCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wCTstatCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wOErrorCoefCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wCErrorCoefCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wCErrorTstatCardButton, SET_VALUE='Cardiac'

END ; of function cardReset

;--------------------------------------------------------------------------------
;
;	PURPOSE: simplify deactivating cardiac labels
;
;--------------------------------------------------------------------------------
pro cardInactive, sWidget

	; Deactivate all cardiac buttons
	WIDGET_CONTROL, sWidget.wOSOSCardButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCSOSCardButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wORegCoefCardButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCRegCoefCardButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wOEffectCardButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCEffectCardButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wOFstatCardButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCFstatCardButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wOTstatCardButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCTstatCardButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wOErrorCoefCardButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCErrorCoefCardButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCErrorTstatCardButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=0
	
end ; of function CardInactive

;--------------------------------------------------------------------------------
;
;	PURPOSE: simplify resetting all output choice labels
;
;--------------------------------------------------------------------------------
pro OutReset, sWidget

	; Reset all output labels
	WIDGET_CONTROL, sWidget.wOSOSCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wORegCoefCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wOEffectCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wOFstatCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wOTstatCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wOErrorCoefCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wOSOSRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wORegCoefRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wOEffectRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wOFstatRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wOTstatRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wOErrorCoefRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wOSOSStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wORegCoefStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wOEffectStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wOFstatStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wOTstatStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wOErrorCoefStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wOCombocoefButton, SET_VALUE='Sin and Cos Magnitude'
	WIDGET_CONTROL, sWidget.wOComboangleButton, SET_VALUE='Sin and Cos Phase Lag'
	WIDGET_CONTROL, sWidget.wOCombofstatButton, SET_VALUE='Sin and Cos F-stat'
	
end ; of function OutReset

;--------------------------------------------------------------------------------
;
;	PURPOSE: simplify resetting all stripchart choice labels
;
;--------------------------------------------------------------------------------
pro ChartReset, sWidget

	; Reset all stripchart labels
	WIDGET_CONTROL, sWidget.wCCardiacButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wCSOSCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wCRegCoefCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wCEffectCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wCFstatCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wCTstatCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wCErrorCoefCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wCErrorTstatCardButton, SET_VALUE='Cardiac'
	WIDGET_CONTROL, sWidget.wCRespiratoryButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wCSOSRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wCRegCoefRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wCEffectRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wCFstatRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wCTstatRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wCErrorCoefRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wCErrorTstatRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wCStimulusButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wCSOSStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wCRegCoefStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wCEffectStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wCFstatStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wCTstatStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wCErrorCoefStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wCErrorTstatStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wCImgAcqButton, SET_VALUE='Image Acquisition'
	WIDGET_CONTROL, sWidget.wCSelectRawButton, SET_VALUE='Mean of ROI Pixels'
	WIDGET_CONTROL, sWidget.wCMotionButton, SET_VALUE='Motion Detection'
	WIDGET_CONTROL, sWidget.wCCombocoefButton, SET_VALUE='Sin and Cos Magnitude'
	WIDGET_CONTROL, sWidget.wCCombofstatButton, SET_VALUE='Sin and Cos F-stat'
	WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=0
	
end ; of function Chart Reset

;--------------------------------------------------------------------------------
;
;	PURPOSE: simplify activating respiratory labels
;
;--------------------------------------------------------------------------------
pro respActive, sWidget

	; Activate all of the respiratory buttons
	WIDGET_CONTROL, sWidget.wOSOSRespButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wCSOSRespButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wORegCoefRespButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wCRegCoefRespButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wOEffectRespButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wCEffectRespButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wOFstatRespButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wCFstatRespButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wOTstatRespButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wCTstatRespButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wOErrorCoefRespButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wCErrorCoefRespButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wCErrorTstatRespButton, SENSITIVE=1
	
end ; of function respActive

;--------------------------------------------------------------------------------
;
;	PURPOSE: simplify resetting all of the respiratory labels
;
;--------------------------------------------------------------------------------
pro respReset, sWidget

	; Reset all respiratory buttons to original value
	WIDGET_CONTROL, sWidget.wOSOSRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wCSOSRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wORegCoefRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wCRegCoefRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wOEffectRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wCEffectRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wOFstatRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wCFstatRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wOTstatRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wCTstatRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wOErrorCoefRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wCErrorCoefRespButton, SET_VALUE='Respiratory'
	WIDGET_CONTROL, sWidget.wCErrorTstatRespButton, SET_VALUE='Respiratory'
	
end ; of function respReset

;--------------------------------------------------------------------------------
;
;	PURPOSE: simplify deactivating cardiac labels
;
;--------------------------------------------------------------------------------
pro respInactive, sWidget

	; Deactivate all the respiratory buttons
	WIDGET_CONTROL, sWidget.wOSOSRespButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCSOSRespButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wORegCoefRespButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCRegCoefRespButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wOEffectRespButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCEffectRespButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wOFstatRespButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCFstatRespButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wOTstatRespButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCTstatRespButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wOErrorCoefRespButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCErrorCoefRespButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCErrorTstatRespButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=0
	
end ; of function respInactive

;--------------------------------------------------------------------------------
;
;	PURPOSE: simplify activating stimulus labels
;
;--------------------------------------------------------------------------------
pro stimActive, sWidget

	; Activate all of the stimulus buttons
	WIDGET_CONTROL, sWidget.wOSOSStimButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wCSOSStimButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wORegCoefStimButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wCRegCoefStimButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wOEffectStimButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wCEffectStimButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wOFstatStimButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wCFstatStimButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wOTstatStimButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wCTstatStimButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wOErrorCoefStimButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wCErrorCoefStimButton, SENSITIVE=1
	WIDGET_CONTROL, sWidget.wCErrorTstatStimButton, SENSITIVE=1
	
end ; of function stimActive

;--------------------------------------------------------------------------------
;
;	PURPOSE: simplify resetting stimulus labels
;
;--------------------------------------------------------------------------------
pro stimReset, sWidget

	; Reset the value of all of the stimulus buttons
	WIDGET_CONTROL, sWidget.wOSOSStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wCSOSStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wORegCoefStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wCRegCoefStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wOEffectStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wCEffectStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wOFstatStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wCFstatStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wOTstatStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wCTstatStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wOErrorCoefStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wCErrorCoefStimButton, SET_VALUE='Stimulus'
	WIDGET_CONTROL, sWidget.wCErrorTstatStimButton, SET_VALUE='Stimulus'
	
end ; of function stimReset

;--------------------------------------------------------------------------------
;
;	PURPOSE: simplify deactivating stimulus labels
;
;--------------------------------------------------------------------------------
pro stimInactive, sWidget

	; Deactivate all of the stimulus buttons
	WIDGET_CONTROL, sWidget.wOSOSStimButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCSOSStimButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wORegCoefStimButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCRegCoefStimButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wOEffectStimButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCEffectStimButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wOFstatStimButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCFstatStimButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wOTstatStimButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCTstatStimButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wOErrorCoefStimButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCErrorCoefStimButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCErrorTstatStimButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wConIntervalText, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wConIntervalLabel, SENSITIVE=0
	
end ; of function stimInactive

;--------------------------------------------------------------------------------
;
;	PURPOSE: simplify resetting cardiac labels
;
;--------------------------------------------------------------------------------
pro determinePosition, sWidget
	
	if (sWidget.Constant eq 1) then begin
		if (sWidget.Cardiac_Regress eq 1) then begin
			sWidget.CardShift=1
			if (sWidget.Respiratory_Regress eq 1) then begin
				sWidget.RespShift=2
				if (sWidget.Stimulus_Regress eq 1) then begin
					sWidget.StimShift=3
				endif
			endif else if (sWidget.Stimulus_Regress eq 1) then begin
				sWidget.StimShift=2
			endif else begin
				; do nothing
			endelse
		endif else if (sWidget.Respiratory_Regress eq 1) then begin
			sWidget.RespShift=1
			if (sWidget.Stimulus_Regress eq 1) then begin
				sWidget.StimShift=2
			endif
		endif else if (sWidget.Stimulus_Regress eq 1) then begin
			sWidget.StimShift=1
		endif else begin
			; do nothing
		endelse
	endif else begin
		if (sWidget.Cardiac_Regress eq 1) then begin
			sWidget.CardShift=0
			if (sWidget.Respiratory_Regress eq 1) then begin
				sWidget.RespShift=1
				if (sWidget.Stimulus_Regress eq 1) then begin
					sWidget.StimShift=2
				endif
			endif
			if (sWidget.Stimulus_Regress eq 1) then begin
				sWidget.StimShift=1
			endif
		endif else if (sWidget.Respiratory_Regress eq 1) then begin
			sWidget.RespShift=0
			if (sWidget.Stimulus_Regress eq 1) then begin
				sWidget.StimShift=1
			endif
		endif else if (sWidget.Stimulus_Regress eq 1) then begin
			sWidget.StimShift=0
		endif else begin
			; do nothing
		endelse
	endelse

	print, "card shift: ",sWidget.CardShift
	print, "resp shift: ",sWidget.RespShift
	print, "stim shift: ",sWidget.StimShift

END ; of function determinePosition

;--------------------------------------------------------------------------------
;
;	PURPOSE: simplify resetting stimulus choice labels
;
;--------------------------------------------------------------------------------
pro resetStimChoice,sWidget

	WIDGET_CONTROL, sWidget.wPolarButton, SET_VALUE = 'Old Polar'
	WIDGET_CONTROL, sWidget.wNewPolar30Button, SET_VALUE = 'New Polar 30'
	WIDGET_CONTROL, sWidget.wNewPolar20Button, SET_VALUE = 'New Polar 20'
	WIDGET_CONTROL, sWidget.wEccentricButton, SET_VALUE = 'Old Ecc'
	WIDGET_CONTROL, sWidget.wNewEccentric30Button, SET_VALUE = 'New Ecc 30'
	WIDGET_CONTROL, sWidget.wNewEccentric20Button, SET_VALUE = 'New Ecc 20'
	WIDGET_CONTROL, sWidget.wCombinedButton, SET_VALUE = 'Old Combined'
	WIDGET_CONTROL, sWidget.wNewCombinedr30e20Button, SET_VALUE='Combined r30 e20 - 4 plus 8'
	WIDGET_CONTROL, sWidget.wNewCombinedr20e30Button, SET_VALUE='Combined r20 e30 - 4 plus 8'
	WIDGET_CONTROL, sWidget.wNewCombinedr20e302plus8Button, SET_VALUE='Combined r20 e30 - 2 plus 8'
	WIDGET_CONTROL, sWidget.wTomSpecialButton, SET_VALUE='Tom Special'
	WIDGET_CONTROL, sWidget.w2HzHemiButton, SET_VALUE='2 Hz Rt. Hemifield'
	WIDGET_CONTROL, sWidget.w4HzHemiButton, SET_VALUE='4 Hz Rt. Hemifield'
	WIDGET_CONTROL, sWidget.w8HzHemiButton, SET_VALUE='8 Hz Rt. Hemifield'
	WIDGET_CONTROL, sWidget.wFeedbackStimButton, SET_VALUE='Feedback'
	WIDGET_CONTROL, sWidget.wEccFeedbackStimButton, SET_VALUE='Ecc Feedback'
	WIDGET_CONTROL, sWidget.wRandomButton, SET_VALUE='Random'
	WIDGET_CONTROL, sWidget.wBlockButton, SET_VALUE = 'FTN (block)'
	WIDGET_CONTROL, sWidget.wThreeButton, SET_VALUE = 'FTN (random)'
	WIDGET_CONTROL, sWidget.wFourButton, SET_VALUE = 'FTAN (random)'
	WIDGET_CONTROL, sWidget.wStemsButton, SET_VALUE = 'Stems'
	WIDGET_CONTROL, sWidget.wLettersButton, SET_VALUE = 'Letters'
	WIDGET_CONTROL, sWidget.wWords1Button, SET_VALUE = 'Words1'
	WIDGET_CONTROL, sWidget.wWords2Button, SET_VALUE = 'Words2'
	WIDGET_CONTROL, sWidget.wWords3Button, SET_VALUE = 'Words3'
	WIDGET_CONTROL, sWidget.wWords4Button, SET_VALUE = 'Words4'

END ; of function resetStimChoice

;--------------------------------------------------------------------------------
;
;	PURPOSE: simplify resetting use of an ROI
;
;--------------------------------------------------------------------------------
pro resetROIStuff,sWidget

	WIDGET_CONTROL, sWidget.wCompROI1Label, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCompROI1Text, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCompROI2Label, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCompROI2Text, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wOptionsList, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wROISliceLabel, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wROISliceText, SENSITIVE=0
  	WIDGET_CONTROL, sWidget.wPositionList, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCutoffLabel, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wCutoffText, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wExpandROILabel, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wExpandROIText, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wROILabel, SENSITIVE=0
  	WIDGET_CONTROL, sWidget.wROIButton, SENSITIVE=0
	WIDGET_CONTROL, sWidget.wNoROILabel, SENSITIVE=0
  	WIDGET_CONTROL, sWidget.wNoROIButton, SENSITIVE=0

END ; of function resetROIStuff

;--------------------------------------------------------------------------------
;
;	PURPOSE: add new attributes to the header
;
;--------------------------------------------------------------------------------
pro headerInfo2,sWidget

	cmd = "read header /usr/CVMR "+sWidget.BaseNum
	;spawn,cmd,data

	err = CVIO_OPEN(sWidget.output_dir+sWidget.file5,2,tag_file)

	err = CVIO_SETATTRIBUTE(tag_file, "PID_ST", sWidget.PID)
	if (sWidget.StimType ne '') then begin
		err = CVIO_SETATTRIBUTE(tag_file, "STIMULUS_TYPE_ST", sWidget.StimType)
		if (sWidget.StimType eq 'block design') then begin
			err = CVIO_SETATTRIBUTE(tag_file, "STIMULUS_NAME_ST", sWidget.StimulusName)
		endif else begin
			err = CVIO_SETATTRIBUTE(tag_file, "ISI_ST", sWidget.ISI)
			err = CVIO_SETATTRIBUTE(tag_file, "JITTER_PRESENT_ST", sWidget.Jitter)
		endelse
	endif
	err = CVIO_SETATTRIBUTE(tag_file, "DURATION_ST", sWidget.Duration)
	if (sWidget.Order eq 0) then begin
			err = CVIO_SETATTRIBUTE(tag_file, "COLLECTION_ORDER_ST", "Sequential")
	endif else begin
		err = CVIO_SETATTRIBUTE(tag_file, "COLLECTION_ORDER_ST", "Interleaved")
	endelse
	err = CVIO_SETATTRIBUTE(tag_file, "NUMBER_IMAGES_ST", sWidget.NumImg)
	err = CVIO_SETATTRIBUTE(tag_file, "NUMBER_SLICES_ST", sWidget.NumSli)
	if (sWidget.FeedbackType ne '') then begin
		err = CVIO_SETATTRIBUTE(tag_file, "FEEDBACK_PRESENT_ST", sWidget.FeedbackType)
	endif
		
	; add attributes from image header

	err = CVIO_CLOSE(tag_file)

END ; of function headerInfo2

;--------------------------------------------------------------------------------
;
;	PURPOSE: add new attributes to the header
;
;--------------------------------------------------------------------------------
pro headerInfo,sWidget

	err = CVIO_OPEN(sWidget.output_dir+sWidget.OriginalOutputFile,2,file)

	if (sWidget.Source eq 0) then begin

		err = CVIO_SETATTRIBUTE(file, "PID_ST", sWidget.PID)
		if (sWidget.StimType ne '') then begin
			err = CVIO_SETATTRIBUTE(file, "STIMULUS_TYPE_ST", sWidget.StimType)
			if (sWidget.StimType eq 'block design') then begin
				err = CVIO_SETATTRIBUTE(file, "STIMULUS_NAME_ST", sWidget.StimulusName)
			endif else begin
				err = CVIO_SETATTRIBUTE(file, "ISI_ST", sWidget.ISI)
				err = CVIO_SETATTRIBUTE(file, "JITTER_PRESENT_ST", sWidget.Jitter)
			endelse
		endif
		err = CVIO_SETATTRIBUTE(file, "DURATION_ST", sWidget.Duration)
		err = CVIO_SETATTRIBUTE(file, "SOURCE_ST", "Shared Memory")
		err = CVIO_SETATTRIBUTE(file, "BASE_NUMBER_ST", sWidget.BaseNum)
		if (sWidget.Order eq 0) then begin
			err = CVIO_SETATTRIBUTE(file, "COLLECTION_ORDER_ST", "Sequential")
		endif else begin
			err = CVIO_SETATTRIBUTE(file, "COLLECTION_ORDER_ST", "Interleaved")
		endelse
		err = CVIO_SETATTRIBUTE(file, "NUMBER_IMAGES_ST", sWidget.NumImg)
		err = CVIO_SETATTRIBUTE(file, "NUMBER_SLICES_ST", sWidget.NumSli)
		err = CVIO_SETATTRIBUTE(file, "NUMBER_CVIO_CHANNELS_ST", sWidget.Channels)
		err = CVIO_SETATTRIBUTE(file, "CVIO_COLLECTION_RATE_ST", sWidget.Rate)
		if (sWidget.SlidingWindow ne -1) then begin
			err = CVIO_SETATTRIBUTE(file, "SLIDING_WINDOW_LENGTH_ST", sWidget.SlidingWindow)
		endif
			if (sWidget.FeedbackType ne '') then begin
			err = CVIO_SETATTRIBUTE(file, "FEEDBACK_PRESENT_ST", sWidget.FeedbackType)
		endif
		err = CVIO_SETATTRIBUTE(file, "NUMBER_REGRESSION_VARIABLES_ST", sWidget.Num_Vars)
		if (sWidget.Constant eq 1) then begin
			err = CVIO_SETATTRIBUTE(file, "CONSTANT_INCLUDED_ST", "Yes")
		endif else begin
			err = CVIO_SETATTRIBUTE(file, "CONSTANT_INCLUDED_ST", "No")
		endelse
		if (sWidget.Cardiac_Regress eq 1) then begin
			err = CVIO_SETATTRIBUTE(file, "CARDIAC_REGRESSION_ST", "Yes")
		endif else begin
			err = CVIO_SETATTRIBUTE(file, "CARDIAC_REGRESSION_ST", "No")
		endelse
		if (sWidget.Respiratory_Regress eq 1) then begin
			err = CVIO_SETATTRIBUTE(file, "RESPIRATORY_REGRESSION_ST", "Yes")
		endif else begin
			err = CVIO_SETATTRIBUTE(file, "RESPIRATORY_REGRESSION_ST", "No")
		endelse
		if (sWidget.Stimulus_Regress eq 1) then begin
			err = CVIO_SETATTRIBUTE(file, "STIMULUS_REGRESSION_ST", "Yes")
		endif else begin
			err = CVIO_SETATTRIBUTE(file, "STIMULUS_REGRESSION_ST", "No")
		endelse
		if (sWidget.ROIFile ne '') then begin
			err = CVIO_SETATTRIBUTE(file, "ROI_FILE_ST", sWidget.ROIFile)
			err = CVIO_SETATTRIBUTE(file, "ROI_SLICE_ST", sWidget.ROISlice)
		endif
		err = CVIO_SETATTRIBUTE(file, "REGRESS_SLICE_ST", sWidget.GetPicture)
		if (sWidget.StripchartTypeOutput eq 1) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Cardiac")
		endif else if (sWidget.StripchartTypeOutput eq 2) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Respiratory")
		endif else if (sWidget.StripchartTypeOutput eq 3) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Stimulus")
		endif else if (sWidget.StripchartTypeOutput eq 4) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Image Acquisition")
		endif else if (sWidget.StripchartTypeOutput eq 5) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Sum of Squares - Cardiac")
		endif else if (sWidget.StripchartTypeOutput eq 6) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Sum of Squares - Respiratory")
		endif else if (sWidget.StripchartTypeOutput eq 7) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Sum of Squares - Stimulus")
		endif else if (sWidget.StripchartTypeOutput eq 8) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Regression Coefficient - Cardiac")
		endif else if (sWidget.StripchartTypeOutput eq 9) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Regression Coefficient - Respiratory")
		endif else if (sWidget.StripchartTypeOutput eq 10) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Regression Coefficient - Stimulus")
		endif else if (sWidget.StripchartTypeOutput eq 11) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Effect Size - Cardiac")
		endif else if (sWidget.StripchartTypeOutput eq 12) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Effect Size - Respiratory")
		endif else if (sWidget.StripchartTypeOutput eq 13) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Effect Size - Stimulus")
		endif else if (sWidget.StripchartTypeOutput eq 14) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "t-statistic - Cardiac")
		endif else if (sWidget.StripchartTypeOutput eq 15) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "t-statistic - Respiratory")
		endif else if (sWidget.StripchartTypeOutput eq 16) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "t-statistic - Stimulus")
		endif else if (sWidget.StripchartTypeOutput eq 17) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "F-statistic - Cardiac")
		endif else if (sWidget.StripchartTypeOutput eq 18) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "F-statistic - Respiratory")
		endif else if (sWidget.StripchartTypeOutput eq 19) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "F-statistic - Stimulus")
		endif else if (sWidget.StripchartTypeOutput eq 20) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Mean of ROI Pixels")
		endif else begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "")
		endelse
		if (sWidget.ResultTypeOutput eq 5) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Sum of Squares - Cardiac")
		endif else if (sWidget.ResultTypeOutput eq 6) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Sum of Squares - Respiratory")
		endif else if (sWidget.ResultTypeOutput eq 7) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Sum of Squares - Stimulus")
		endif else if (sWidget.ResultTypeOutput eq 8) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Regression Coefficient - Cardiac")
		endif else if (sWidget.ResultTypeOutput eq 9) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Regression Coefficient - Respiratory")
		endif else if (sWidget.ResultTypeOutput eq 10) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Regression Coefficient - Stimulus")
		endif else if (sWidget.ResultTypeOutput eq 11) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Effect Size - Cardiac")
		endif else if (sWidget.ResultTypeOutput eq 12) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Effect Size - Respiratory")
		endif else if (sWidget.ResultTypeOutput eq 13) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Effect Size - Stimulus")
		endif else if (sWidget.ResultTypeOutput eq 14) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "t-statistic - Cardiac")
		endif else if (sWidget.ResultTypeOutput eq 15) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "t-statistic - Respiratory")
		endif else if (sWidget.ResultTypeOutput eq 16) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "t-statistic - Stimulus")
		endif else if (sWidget.ResultTypeOutput eq 17) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "F-statistic - Cardiac")
		endif else if (sWidget.ResultTypeOutput eq 18) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "F-statistic - Respiratory")
		endif else if (sWidget.ResultTypeOutput eq 19) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "F-statistic - Stimulus")
		endif else if (sWidget.ResultTypeOutput eq 20) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "F-statistic - Stimulus")
		endif else if (sWidget.ResultTypeOutput eq 21) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Phase Lag Sin and Cos")
		endif else if (sWidget.ResultTypeOutput eq 22) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Sin and Cos F-statistic")
		endif else if (sWidget.ResultTypeOutput eq 23) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Magnitude Sin and Cos")
		endif else begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "")
		endelse
	endif else begin

		err = CVIO_SETATTRIBUTE(file, "PID_ST", sWidget.PID)

		;err = CVIO_OPEN(sWidget.output_dir+sWidget.file5,2,tag_file)
		;err = CVIO_GETATTRIBUTE(tag_file,"STIMULUS_TYPE_ST",stim_type)
		;if (stim_type ne '') then begin
		;	err = CVIO_SETATTRIBUTE(file, "STIMULUS_TYPE_ST", stim_type)
		;	if (stim_type eq 'block design') then begin
		;		err = CVIO_GETATTRIBUTE(tag_file, "STIMULUS_NAME_ST", stim_name)
		;		err = CVIO_SETATTRIBUTE(file, "STIMULUS_NAME_ST", stim_name)
		;	endif else begin
		;		err = CVIO_GETATTRIBUTE(tag_file, "ISI_ST", stim_ISI)
		;		err = CVIO_SETATTRIBUTE(file, "ISI_ST", stim_ISI)
		;		err = CVIO_GETATTRIBUTE(tag_file, "JITTER_PRESENT_ST", stim_jitter)
		;		err = CVIO_SETATTRIBUTE(file, "JITTER_PRESENT_ST", stim_jitter)
		;	endelse
		;endif
		;err = cvio_close(tag_file)

		if (sWidget.StimStatus eq 'Yes') then begin
			err = CVIO_SETATTRIBUTE(file, "SHOW_STIMULUS_ST", "Yes")
		endif else begin
			err = CVIO_SETATTRIBUTE(file, "SHOW_STIMULUS_ST", "No")
		endelse
		if (sWidget.Order eq 0) then begin
			err = CVIO_SETATTRIBUTE(file, "COLLECTION_ORDER_ST", "Sequential")
		endif else begin
			err = CVIO_SETATTRIBUTE(file, "COLLECTION_ORDER_ST", "Interleaved")
		endelse
		err = CVIO_SETATTRIBUTE(file, "NUMBER_IMAGES_ST", sWidget.NumImg)
		err = CVIO_SETATTRIBUTE(file, "NUMBER_SLICES_ST", sWidget.NumSli)
		err = CVIO_SETATTRIBUTE(file, "NUMBER_CVIO_CHANNELS_ST", sWidget.Channels)
		err = CVIO_SETATTRIBUTE(file, "CVIO_COLLECTION_RATE_ST", sWidget.Rate)
		if (sWidget.SlidingWindow ne -1) then begin
			err = CVIO_SETATTRIBUTE(file, "SLIDING_WINDOW_LENGTH_ST", sWidget.SlidingWindow)
		endif
		if (sWidget.FeedbackType ne '') then begin
			err = CVIO_SETATTRIBUTE(file, "FEEDBACK_PRESENT_ST", sWidget.FeedbackType)
		endif
		err = CVIO_SETATTRIBUTE(file, "NUMBER_REGRESSION_VARIABLES_ST", sWidget.Num_Vars)
		if (sWidget.Constant eq 1) then begin
			err = CVIO_SETATTRIBUTE(file, "CONSTANT_INCLUDED_ST", "Yes")
		endif else begin
			err = CVIO_SETATTRIBUTE(file, "CONSTANT_INCLUDED_ST", "No")
		endelse
		if (sWidget.Cardiac_Regress eq 1) then begin
			err = CVIO_SETATTRIBUTE(file, "CARDIAC_REGRESSION_ST", "Yes")
		endif else begin
			err = CVIO_SETATTRIBUTE(file, "CARDIAC_REGRESSION_ST", "No")
		endelse
		if (sWidget.Respiratory_Regress eq 1) then begin
			err = CVIO_SETATTRIBUTE(file, "RESPIRATORY_REGRESSION_ST", "Yes")
		endif else begin
			err = CVIO_SETATTRIBUTE(file, "RESPIRATORY_REGRESSION_ST", "No")
		endelse
		if (sWidget.Stimulus_Regress eq 1) then begin
			err = CVIO_SETATTRIBUTE(file, "STIMULUS_REGRESSION_ST", "Yes")
		endif else begin
			err = CVIO_SETATTRIBUTE(file, "STIMULUS_REGRESSION_ST", "No")
		endelse
		if (sWidget.ROIFile ne '') then begin
			err = CVIO_SETATTRIBUTE(file, "ROI_FILE_ST", sWidget.ROIFile)
			err = CVIO_SETATTRIBUTE(file, "ROI_SLICE_ST", sWidget.ROISlice)
		endif
		err = CVIO_SETATTRIBUTE(file, "REGRESS_SLICE_ST", sWidget.GetPicture)
		if (sWidget.StripchartTypeOutput eq 1) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Cardiac")
		endif else if (sWidget.StripchartTypeOutput eq 2) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Respiratory")
		endif else if (sWidget.StripchartTypeOutput eq 3) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Stimulus")
		endif else if (sWidget.StripchartTypeOutput eq 4) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Image Acquisition")
		endif else if (sWidget.StripchartTypeOutput eq 5) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Sum of Squares - Cardiac")
		endif else if (sWidget.StripchartTypeOutput eq 6) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Sum of Squares - Respiratory")
		endif else if (sWidget.StripchartTypeOutput eq 7) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Sum of Squares - Stimulus")
		endif else if (sWidget.StripchartTypeOutput eq 8) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Regression Coefficient - Cardiac")
		endif else if (sWidget.StripchartTypeOutput eq 9) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Regression Coefficient - Respiratory")
		endif else if (sWidget.StripchartTypeOutput eq 10) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Regression Coefficient - Stimulus")
		endif else if (sWidget.StripchartTypeOutput eq 11) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Effect Size - Cardiac")
		endif else if (sWidget.StripchartTypeOutput eq 12) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Effect Size - Respiratory")
		endif else if (sWidget.StripchartTypeOutput eq 13) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Effect Size - Stimulus")
		endif else if (sWidget.StripchartTypeOutput eq 14) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "t-statistic - Cardiac")
		endif else if (sWidget.StripchartTypeOutput eq 15) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "t-statistic - Respiratory")
		endif else if (sWidget.StripchartTypeOutput eq 16) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "t-statistic - Stimulus")
		endif else if (sWidget.StripchartTypeOutput eq 17) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "F-statistic - Cardiac")
		endif else if (sWidget.StripchartTypeOutput eq 18) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "F-statistic - Respiratory")
		endif else if (sWidget.StripchartTypeOutput eq 19) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "F-statistic - Stimulus")
		endif else if (sWidget.StripchartTypeOutput eq 20) then begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "Mean of ROI Pixels")
		endif else begin
			err = CVIO_SETATTRIBUTE(file, "STRIPCHART_OUTPUT_ST", "")
		endelse
		if (sWidget.ResultTypeOutput eq 5) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Sum of Squares - Cardiac")
		endif else if (sWidget.ResultTypeOutput eq 6) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Sum of Squares - Respiratory")
		endif else if (sWidget.ResultTypeOutput eq 7) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Sum of Squares - Stimulus")
		endif else if (sWidget.ResultTypeOutput eq 8) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Regression Coefficient - Cardiac")
		endif else if (sWidget.ResultTypeOutput eq 9) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Regression Coefficient - Respiratory")
		endif else if (sWidget.ResultTypeOutput eq 10) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Regression Coefficient - Stimulus")
		endif else if (sWidget.ResultTypeOutput eq 11) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Effect Size - Cardiac")
		endif else if (sWidget.ResultTypeOutput eq 12) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Effect Size - Respiratory")
		endif else if (sWidget.ResultTypeOutput eq 13) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Effect Size - Stimulus")
		endif else if (sWidget.ResultTypeOutput eq 14) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "t-statistic - Cardiac")
		endif else if (sWidget.ResultTypeOutput eq 15) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "t-statistic - Respiratory")
		endif else if (sWidget.ResultTypeOutput eq 16) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "t-statistic - Stimulus")
		endif else if (sWidget.ResultTypeOutput eq 17) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "F-statistic - Cardiac")
		endif else if (sWidget.ResultTypeOutput eq 18) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "F-statistic - Respiratory")
		endif else if (sWidget.ResultTypeOutput eq 19) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "F-statistic - Stimulus")
		endif else if (sWidget.ResultTypeOutput eq 20) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "F-statistic - Stimulus")
		endif else if (sWidget.ResultTypeOutput eq 21) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Phase Lag Sin and Cos")
		endif else if (sWidget.ResultTypeOutput eq 22) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Sin and Cos F-statistic")
		endif else if (sWidget.ResultTypeOutput eq 23) then begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "Magnitude Sin and Cos")
		endif else begin
			err = CVIO_SETATTRIBUTE(file, "RESULT_OUTPUT_ST", "")
		endelse
	endelse

	err = CVIO_CLOSE(file)
	

END ; of function headerInfo

;--------------------------------------------------------------------------------
;
;	PURPOSE: write user input information to specified file
;
;--------------------------------------------------------------------------------
pro writeInputs, sWidget

	OPENW, 10, sWidget.output_dir+sWidget.InputFile

	PRINTF, 10, 'Patient ID: ', sWidget.PID
	PRINTF, 10, 'Date: ', sWidget.Date
	PRINTF, 10, 'Run Number: ', sWidget.RunNum
	PRINTF, 10, 'Run Label: ', sWidget.RunLabel
	if (sWidget.StimType ne '') then begin
		PRINTF, 10, 'Stimulus Type: ', sWidget.StimType
		if (sWidget.StimType eq 'block design') then begin
			PRINTF, 10, 'Stimulus Name: ', sWidget.StimulusName
		endif else begin
			PRINTF, 10, 'Interstimulus Interval (sec): ', sWidget.ISI
			PRINTF, 10, 'Jitter Present: ', sWidget.Jitter
		endelse
	endif
	if (sWidget.Source eq 1) then begin 
		if (sWidget.StimStatus eq 'Yes') then begin
			PRINTF, 10, 'Show Stimulus: Yes'
		endif else begin
			PRINTF, 10, 'Show Stimulus: No'
		endelse
	endif
	if (sWidget.Source eq 0) then begin
		PRINTF, 10, 'Run Duration (sec): ', sWidget.Duration
		PRINTF, 10, 'Data Source: Shared Memory'
		PRINTF, 10, 'Base Number: ', sWidget.BaseNum
	endif else begin
		PRINTF, 10, 'Data Source: Disk'
	endelse
	if (sWidget.Order eq 0) then begin
		PRINTF, 10, 'Collection Order: Sequential'
	endif else begin
		PRINTF, 10, 'Collection Order: Interleaved'
	endelse
	PRINTF, 10, 'Number of Images: ', sWidget.NumImg
	PRINTF, 10, 'Number of Slices: ', sWidget.NumSli
	PRINTF, 10, 'Number of CVIO Channels: ', sWidget.Channels
	PRINTF, 10, 'CVIO Channels: ', sWidget.Acquire_Files(0:*)
	PRINTF, 10, 'Collection Rate (Hz): ', sWidget.Rate
	if (sWidget.SlidingWindow ne -1) then begin
		PRINTF, 10, 'Sliding Window Size (sec): ', sWidget.SlidingWindow
	endif
	if (sWidget.FeedbackType ne '') then begin
		PRINTF, 10, 'Feedback Type: ', sWidget.FeedbackType
	endif
	PRINTF, 10, 'Number Regression Variables: ', sWidget.Num_Vars
	if (sWidget.Cardiac_Regress eq 1) then begin
		PRINTF, 10, 'Cardiac Regress: Yes'
	endif else begin
		PRINTF, 10, 'Cardiac Regress: No'
	endelse
	if (sWidget.Respiratory_Regress eq 1) then begin
		PRINTF, 10, 'Respiratory Regress: Yes'
	endif else begin
		PRINTF, 10, 'Respiratory Regress: No'
	endelse
	if (sWidget.Stimulus_Regress eq 1) then begin
		PRINTF, 10, 'Stimulus Regress: Yes'
	endif else begin
		PRINTF, 10, 'Stimulus Regress: No'
	endelse
	if (sWidget.Constant eq 1) then begin
		PRINTF, 10, 'Constant Included: Yes'
	endif else begin
		PRINTF, 10, 'Constant Included: No'
	endelse
	if (sWidget.ROIFile ne '') then begin
		PRINTF, 10, 'ROI File Name: ', sWidget.ROIFile
		PRINTF, 10, 'ROI Slice Number: ', sWidget.ROISlice
	endif
	PRINTF, 10, 'Regression Slice: ', sWidget.GetPicture
	if (sWidget.StripchartTypeOutput eq 1) then begin
		PRINTF, 10, 'Stripchart Type: Cardiac'
	endif else if (sWidget.StripchartTypeOutput eq 2) then begin
		PRINTF, 10, 'Stripchart Type: Respiratory'
	endif else if (sWidget.StripchartTypeOutput eq 3) then begin
		PRINTF, 10, 'Stripchart Type: Stimulus'
	endif else if (sWidget.StripchartTypeOutput eq 4) then begin
		PRINTF, 10, 'Stripchart Type: Image Acquisition'
	endif else if (sWidget.StripchartTypeOutput eq 5) then begin
		PRINTF, 10, 'Stripchart Type: Sum of Squares - Cardiac'
	endif else if (sWidget.StripchartTypeOutput eq 6) then begin
		PRINTF, 10, 'Stripchart Type: Sum of Squares - Respiratory'
	endif else if (sWidget.StripchartTypeOutput eq 7) then begin
		PRINTF, 10, 'Stripchart Type: Sum of Squares - Stimulus'
	endif else if (sWidget.StripchartTypeOutput eq 8) then begin
		PRINTF, 10, 'Stripchart Type: Regression Coefficient - Cardiac'
	endif else if (sWidget.StripchartTypeOutput eq 9) then begin
		PRINTF, 10, 'Stripchart Type: Regression Coefficient - Respiratory'
	endif else if (sWidget.StripchartTypeOutput eq 10) then begin
		PRINTF, 10, 'Stripchart Type: Regression Coefficient - Stimulus'
	endif else if (sWidget.StripchartTypeOutput eq 11) then begin
		PRINTF, 10, 'Stripchart Type: Effect Size - Cardiac'
	endif else if (sWidget.StripchartTypeOutput eq 12) then begin
		PRINTF, 10, 'Stripchart Type: Effect Size - Respiratory'
	endif else if (sWidget.StripchartTypeOutput eq 13) then begin
		PRINTF, 10, 'Stripchart Type: Effect Size - Stimulus'
	endif else if (sWidget.StripchartTypeOutput eq 14) then begin
		PRINTF, 10, 'Stripchart Type: t-statistic - Cardiac'
	endif else if (sWidget.StripchartTypeOutput eq 15) then begin
		PRINTF, 10, 'Stripchart Type: t-statistic - Respiratory'
	endif else if (sWidget.StripchartTypeOutput eq 16) then begin
		PRINTF, 10, 'Stripchart Type: t-statistic - Stimulus'
	endif else if (sWidget.StripchartTypeOutput eq 17) then begin
		PRINTF, 10, 'Stripchart Type: F-statistic - Cardiac'
	endif else if (sWidget.StripchartTypeOutput eq 18) then begin
		PRINTF, 10, 'Stripchart Type: F-statistic - Respiratory'
	endif else if (sWidget.StripchartTypeOutput eq 19) then begin
		PRINTF, 10, 'Stripchart Type: F-statistic - Stimulus'
	endif else if (sWidget.StripchartTypeOutput eq 20) then begin
		PRINTF, 10, 'Stripchart Type: Mean of ROI Pixels'
	endif else if (sWidget.StripchartTypeOutput eq 21) then begin
		PRINTF, 10, 'Stripchart Type: Motion Detection'
	endif else if (sWidget.StripchartTypeOutput eq 22) then begin
		PRINTF, 10, 'Stripchart Type: Sin and Cos F-statistic'
	endif else if (sWidget.StripchartTypeOutput eq 23) then begin
		PRINTF, 10, 'Stripchart Type: Magnitude Sin and Cos'
	endif else begin
		PRINTF, 10, 'Stripchart Type: '
	endelse
	if (sWidget.ResultTypeOutput eq 5) then begin
		PRINTF, 10, 'Result Type: Sum of Squares - Cardiac'
	endif else if (sWidget.ResultTypeOutput eq 6) then begin
		PRINTF, 10, 'Result Type: Sum of Squares - Respiratory'
	endif else if (sWidget.ResultTypeOutput eq 7) then begin
		PRINTF, 10, 'Result Type: Sum of Squares - Stimulus'
	endif else if (sWidget.ResultTypeOutput eq 8) then begin
		PRINTF, 10, 'Result Type: Regression Coefficient - Cardiac'
	endif else if (sWidget.ResultTypeOutput eq 9) then begin
		PRINTF, 10, 'Result Type: Regression Coefficient - Respiratory'
	endif else if (sWidget.ResultTypeOutput eq 10) then begin
		PRINTF, 10, 'Result Type: Regression Coefficient - Stimulus'
	endif else if (sWidget.ResultTypeOutput eq 11) then begin
		PRINTF, 10, 'Result Type: Effect Size - Cardiac'
	endif else if (sWidget.ResultTypeOutput eq 12) then begin
		PRINTF, 10, 'Result Type: Effect Size - Respiratory'
	endif else if (sWidget.ResultTypeOutput eq 13) then begin
		PRINTF, 10, 'Result Type: Effect Size - Stimulus'
	endif else if (sWidget.ResultTypeOutput eq 14) then begin
		PRINTF, 10, 'Result Type: t-statistic - Cardiac'
	endif else if (sWidget.ResultTypeOutput eq 15) then begin
		PRINTF, 10, 'Result Type: t-statistic - Respiratory'
	endif else if (sWidget.ResultTypeOutput eq 16) then begin
		PRINTF, 10, 'Result Type: t-statistic - Stimulus'
	endif else if (sWidget.ResultTypeOutput eq 17) then begin
		PRINTF, 10, 'Result Type: F-statistic - Cardiac'
	endif else if (sWidget.ResultTypeOutput eq 18) then begin
		PRINTF, 10, 'Result Type: F-statistic - Respiratory'
	endif else if (sWidget.ResultTypeOutput eq 19) then begin
		PRINTF, 10, 'Result Type: F-statistic - Stimulus'
	endif else if (sWidget.ResultTypeOutput eq 21) then begin
		PRINTF, 10, 'Result Type: Phase Lag Sin and Cos'
	endif else if (sWidget.ResultTypeOutput eq 22) then begin
		PRINTF, 10, 'Result Type: Sin and Cos F-statistic'
	endif else if (sWidget.ResultTypeOutput eq 23) then begin
		PRINTF, 10, 'Result Type: Magnitude Sin and Cos'
	endif else begin
		PRINTF, 10, 'Result Type: '
	endelse
	if (sWidget.Save eq 1) then begin
		PRINTF, 10, 'Output File Type: ', sWidget.OutputFileType
		PRINTF, 10, 'Output File Name: ', sWidget.OriginalOutputFile
	end

	CLOSE, 10

END ; of function writeInputs

;--------------------------------------------------------------------------------
;
;	PURPOSE: interface creation program
;
;--------------------------------------------------------------------------------
pro c_interface
device, true=24, decomposed=0
  Title = "To be Determined"
  Name = 'my_interface'

  !EXCEPT = 0

  disp_sizex = 256
  disp_sizey = 256

  my_font = '-*-times-*-r-*-*-15-*-*-*-*-*-*-*'
  title_font = '-*-times-bold-*-*-*-20-*-*-*-*-*-*-*'

  wBigBase = Widget_Base( GROUP_LEADER=wGroup, UNAME='wBigBase'  $
  	,TITLE='Chris' ,SPACE=3 ,/ROW, TLB_FRAME_ATTR=1, MAP=0, $
	/TLB_KILL_REQUEST_EVENTS)

	wBigRightBase = Widget_Base(wBigBase, UNAME = 'wBigRightBase',/COLUMN)

 	wDemoBase=Widget_Base(wBigRightBase,UNAME='wDemoBase',TITLE='IDL',SPACE=3,/COLUMN)

  		wDemoLabel = Widget_Label(wDemoBase, UNAME='wDemoLabel'  $
    	  	,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Demographics' $
			,FONT=title_font)

		wDemoTopBase = Widget_Base(wDemoBase, UNAME='wDemoTopBase',/ROW)

		wPIDLabel = Widget_Label(wDemoTopBase, UNAME='wPIDLabel'  $
      		,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Patient ID:',FONT=my_font)

  		wPIDText = Widget_Text(wDemoTopBase, UNAME='wPIDText' $
    	  		,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS)

		wDateLabel = Widget_Label(wDemoTopBase, UNAME='wDateLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Date:' ,FONT=my_font)

  		wDateText = Widget_Text(wDemoTopBase, UNAME='wDateText'  $
      			,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS)

		wDemoBottomBase = Widget_Base(wDemoBase, UNAME='wDemoBottomBase',/ROW)

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

		wDesignTopBase = Widget_Base(wDesignBase, UNAME='wDesignTopBase',/ROW)

		stimTypeListValues = [ 'block design', 'event related' ]
  		wStimTypeList = Widget_Droplist(wDesignTopBase, UNAME='wStimTypeList'  $
      		,TITLE='Stimulus Type: ' ,VALUE=stimTypeListValues ,FONT=my_font, $
			UVALUE=stimTypeListValues)

  		wStimLiButton = Widget_Button(wDesignTopBase, UNAME='wStimLiButton'  $
      			,VALUE = 'Stimulus' ,FONT=my_font, /MENU, /DYNAMIC_RESIZE, SENSITIVE=0)
			
			wStimVisualButton = Widget_Button(wStimLiButton, $
				VALUE='Phase-Mapping Checkerboard',UNAME='wStimVisualButton' $
				,SENSITIVE=0 ,/MENU, FONT=my_font, /DYNAMIC_RESIZE)
				
				wPolarButton = Widget_Button(wStimVisualButton,VALUE='Old Polar' $
					,FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0,UNAME='wPolarButton')
				
				wNewPolar30Button = Widget_Button(wStimVisualButton, $
					VALUE='New Polar 30', FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0, $
					UNAME='wNewPolar30Button')
				
				wNewPolar20Button = Widget_Button(wStimVisualButton, $
					VALUE='New Polar 20', FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0, $
					UNAME='wNewPolar20Button')
				
				wEccentricButton = Widget_Button(wStimVisualButton, $ 
					VALUE='Old Ecc', FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0, $
					UNAME='wEccentricButton')
				
				wNewEccentric30Button = Widget_Button(wStimVisualButton, $ 
					VALUE='New Ecc 30', FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0, $
					UNAME='wNewEccentric30Button')
				
				wNewEccentric20Button = Widget_Button(wStimVisualButton, $ 
					VALUE='New Ecc 20', FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0, $
					UNAME='wNewEccentric20Button')
			
				wCombinedButton = Widget_Button(wStimVisualButton, $
					VALUE='Old Combined', FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0, $
					UNAME='wCombinedButton')
			
				wNewCombinedr30e20Button = Widget_Button(wStimVisualButton, $
					VALUE='Combined r30 e20 - 4 plus 8', FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0, $
					UNAME='wNewCombinedr30e20Button')
			
				wNewCombinedr20e30Button = Widget_Button(wStimVisualButton, $
					VALUE='Combined r20 e30 - 4 plus 8', FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0, $
					UNAME='wNewCombinedr20e30Button')
			
				wNewCombinedr20e302plus8Button = Widget_Button(wStimVisualButton, $
					VALUE='Combined r20 e30 - 2 plus 8', FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0, $
					UNAME='wNewCombinedr20e302plus8Button')
			
				wTomSpecialButton = Widget_Button(wStimVisualButton, $
					VALUE='Tom Special', FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0, $
					UNAME='wTomSpecialButton')
			
				w2HzHemiButton = Widget_Button(wStimVisualButton, $
					VALUE='2 Hz Rt. Hemifield', FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0, $
					UNAME='w2HzHemiButton')
			
				w4HzHemiButton = Widget_Button(wStimVisualButton, $
					VALUE='4 Hz Rt. Hemifield', FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0, $
					UNAME='w4HzHemiButton')
			
				w8HzHemiButton = Widget_Button(wStimVisualButton, $
					VALUE='8 Hz Rt. Hemifield', FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0, $
					UNAME='w8HzHemiButton')
			
				wFeedbackStimButton = Widget_Button(wStimVisualButton, $
					VALUE='Feedback', FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0, $
					UNAME='wFeedbackStimButton')
			
				wEccFeedbackStimButton = Widget_Button(wStimVisualButton, $
					VALUE='Ecc Feedback', FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0, $
					UNAME='wEccFeedbackStimButton')
			
				wRandomButton = Widget_Button(wStimVisualButton, $
					VALUE='Random', FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0, $
					UNAME='wRandomButton')

			wStimNamingButton = Widget_Button(wStimLiButton, VALUE='Visual Naming' $
				,UNAME='wStimNamingButton',SENSITIVE=0,/MENU, FONT=my_font, /DYNAMIC_RESIZE)
		
				wBlockButton = Widget_Button(wStimNamingButton,VALUE='FTN (block)', $
					UNAME='wBlockButton',FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0)
				
				wThreeButton = Widget_Button(wStimNamingButton,VALUE='FTN (random)', $
					UNAME='wThreeButton',FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0)
			
				wFourButton = Widget_Button(wStimNamingButton,VALUE='FTAN (random)', $
					UNAME='wFourButton',FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0)

			wStimWordButton = Widget_Button(wStimLiButton, VALUE='Word Association' $
				,UNAME='wStimWordButton',SENSITIVE=0,/MENU,FONT=my_font,/DYNAMIC_RESIZE)
		
				wStemsButton = Widget_Button(wStimWordButton,VALUE='Stems', $
					UNAME='wStemsButton',FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0)
		
				wLettersButton = Widget_Button(wStimWordButton,VALUE='Letters', $
					UNAME='wLettersButton',FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0)
		
				wWords1Button = Widget_Button(wStimWordButton,VALUE='Words 1', $
					UNAME='wWords1Button',FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0)
		
				wWords2Button = Widget_Button(wStimWordButton,VALUE='Words 2', $
					UNAME='wWords2Button',FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0)
		
				wWords3Button = Widget_Button(wStimWordButton,VALUE='Words 3', $
					UNAME='wWords3Button',FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0)
		
				wWords4Button = Widget_Button(wStimWordButton,VALUE='Words 4', $
					UNAME='wWords4Button',FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0)
		
				wBulbButton = Widget_Button(wStimWordButton,VALUE='Bulb', $
					UNAME='wBulbButton',FONT=my_font, /DYNAMIC_RESIZE, SENSITIVE=0)

		wPeriodLabel = Widget_Label(wDesignTopBase, UNAME='wPeriodLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Stim Period (sec): ' ,FONT=my_font)

  		wPeriodText = Widget_Text(wDesignTopBase, UNAME='wPeriodText'  $
      			,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS, XSIZE=4)

		wDesignMiddleBase = Widget_Base(wDesignBase, UNAME='wDesignMiddleBase',/ROW)

  		wISILabel = Widget_Label(wDesignMiddleBase, UNAME='wISILabel'  $
      			,SENSITIVE=0 ,/ALIGN_LEFT ,VALUE='Mean ISI: ' ,FONT=my_font)

  		wISIText = Widget_Text(wDesignMiddleBase, UNAME='wISIText' ,SENSITIVE=0 $
			,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS, XSIZE=4)

		jitterListValues = [ 'Yes', 'No' ]
  		wJitterList = Widget_Droplist(wDesignMiddleBase, UNAME='wJitterList'  $
      		,TITLE='Jitter: ' ,VALUE=jitterListValues ,FONT=my_font, $
			UVALUE=jitterListValues, SENSITIVE=0)

  		wRandomLabel = Widget_Label(wDesignMiddleBase, UNAME='wRandomLabel'  $
      			,SENSITIVE=0 ,/ALIGN_LEFT ,VALUE='CVIO File: ' ,FONT=my_font)

  		wRandomText = Widget_Text(wDesignMiddleBase, UNAME='wRandomText' ,SENSITIVE=0 $
			,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS)

  	wCollectionBase = Widget_Base(wBigRightBase, UNAME='wCollectionBase'  $
      	,TITLE='IDL' ,SPACE=3 ,/COLUMN)

  		wCollectionLabel = Widget_Label(wCollectionBase, UNAME='Data Source'  $
      		,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Data Source' ,FONT=title_font)

		wCollectionTopBase=Widget_Base(wCollectionBase,UNAME='wCollectionTopBase',/ROW)

		sourceListValues = [ 'Shared Memory', 'Disk' ]
  		wSourceList = Widget_Droplist(wCollectionTopBase, UNAME='wSourceList'  $
      		,TITLE='Run Mode: ' ,VALUE=sourceListValues ,FONT=my_font, $
			UVALUE=sourceListValues)

		orderListValues = [ 'Interleaved', 'Sequential' ]
  		wOrderList = Widget_Droplist(wCollectionTopBase, UNAME='wOrderList'  $
      		,TITLE='Acquisition Order: ' ,VALUE=orderListValues ,FONT=my_font $
			,UVALUE=orderListValues)

		wCollectionMiddleBase=Widget_Base(wCollectionBase,UNAME='wCollectionMiddleBase',/ROW)

  		wNumImgLabel = Widget_Label(wCollectionMiddleBase, UNAME='wNumImgLabel'  $
      		,SENSITIVE=1,/ALIGN_LEFT ,VALUE='Number of Images: ' ,FONT=my_font)

  		wNumImgText = Widget_Text(wCollectionMiddleBase, UNAME='wNumImgText'  $
      		,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS, XSIZE=8)

  		wNumSliLabel = Widget_Label(wCollectionMiddleBase, UNAME='wNumSliLabel'  $
      		,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Number of Slices: ' ,FONT=my_font)

  		wNumSliText = Widget_Text(wCollectionMiddleBase, UNAME='wNumSliText'  $
      		,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS, XSIZE=8)

		wCollectionSuperBottomBase = Widget_Base(wCollectionBase, $
			UNAME='wCollectionSuperBottomBase' ,/ROW)

		wXDimensionLabel = Widget_Label(wCollectionSuperBottomBase, UNAME='wXDimensionLabel'  $
      		,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='X Dimension: ' ,FONT=my_font)

  		wXDimensionText = Widget_Text(wCollectionSuperBottomBase, UNAME='wXDimensionText' $
 			,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS, XSIZE=8)	

		wYDimensionLabel = Widget_Label(wCollectionSuperBottomBase, UNAME='wYDimensionLabel'  $
      		,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Y Dimension: ' ,FONT=my_font)

  		wYDimensionText = Widget_Text(wCollectionSuperBottomBase, UNAME='wYDimensionText' $
 			,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS, XSIZE=8)

		wCollectionBottomBase = Widget_Base(wCollectionBase, $
			UNAME='wCollectionBottomBase' ,/ROW)

		wChannelsButton = Widget_Button(wCollectionBottomBase, UNAME='wChannelsButton', $
			VALUE='CVIO Channels', /MENU ,FONT=my_font, /DYNAMIC_RESIZE)

			wCVIOImagAcqButton = Widget_Button(wChannelsButton, UNAME='wCVIOImagAcqButton', $
				VALUE='Image Acquisition', SENSITIVE=1, FONT=my_font, /DYNAMIC_RESIZE)
	
			wCVIOCardiacButton = Widget_Button(wChannelsButton, UNAME='wCVIOCardiacButton', $
				VALUE='Cardiac', SENSITIVE=1, FONT=my_font, /DYNAMIC_RESIZE)

			wCVIORespiratoryButton = Widget_Button(wChannelsButton, UNAME='wCVIORespiratoryButton', $
				VALUE='Respiratory', SENSITIVE=1, FONT=my_font, /DYNAMIC_RESIZE)

			wCVIOStimulusButton = Widget_Button(wChannelsButton, UNAME='wCVIOStimulusButton', $
				VALUE='Stimulus', SENSITIVE=1, FONT=my_font, /DYNAMIC_RESIZE)

		wRateLabel = Widget_Label(wCollectionBottomBase, UNAME='wRateLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Collection Rate (Hz): ' ,FONT=my_font)

		wRateText = Widget_Text(wCollectionBottomBase, UNAME='wRateText'  $
      			,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS, XSIZE=8)

  		wDurationLabel = Widget_Label(wCollectionBottomBase, UNAME='wDurationLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Run Duration (sec): ' ,FONT=my_font)

  		wDurationText = Widget_Text(wCollectionBottomBase, UNAME='wDurationText'  $
      			,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS, XSIZE=8)	

  	wAnalysisBase = Widget_Base(wBigRightBase, UNAME='wAnalysisBase'  $
   		,TITLE='IDL' ,SPACE=3 ,/COLUMN)

  		wAnalysisLabel = Widget_Label(wAnalysisBase, UNAME='wAnalysisLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Analysis' ,FONT=title_font)

		wAnalysisTopBase = Widget_Base(wAnalysisBase, UNAME = 'wAnalysisTopBase' ,/ROW)

		wRegressionButton = Widget_Button(wAnalysisTopBase, VALUE='Regression Model', $
			UNAME='wRegressionButton', /Menu ,FONT=my_font, /DYNAMIC_RESIZE)

		wDependentButton = Widget_Button(wRegressionButton, $
			VALUE='Select Dependent Variable', UVALUE='DEPENDENT', SENSITIVE=1, $
			UNAME='wDependentButton', /MENU ,FONT=my_font, /DYNAMIC_RESIZE)

			wDependentVarLabelButton = Widget_Button(wDependentButton, $
				VALUE='Dependent Variable', FONT=my_font, /DYNAMIC_RESIZE, $
				SENSITIVE=0, UNAME='wDependentVarLabel')

			wImageTimeSeriesButton = Widget_Button(wDependentButton, VALUE='MRI Images', $
				UVALUE='IMAGETIMESERIES', SENSITIVE=1, FONT=my_font, $
				UNAME='wImageTimeSeriesButton', /MENU, /DYNAMIC_RESIZE)

				wImageTimeSeriesLabelButton = Widget_Button(wImageTimeSeriesButton, $
					VALUE='Available Data Files', SENSITIVE=0, UNAME='wImageTimeSeriesLabel', $
					FONT=my_font, /DYNAMIC_RESIZE)

				wTaggedImgButton = Widget_Button(wImageTimeSeriesButton, $
					VALUE='Tagged Img Data',/DYNAMIC_RESIZE,UVALUE='TAGGEDIMGDATA', $
					 SENSITIVE=0,FONT=my_font ,UNAME='wTaggedImgButton')
	
		wIndependentButton = Widget_Button(wRegressionButton, $
			VALUE='Select Independent Variable', UVALUE='INDEPENDENT', /MENU, $
			UNAME='wIndependentButton' ,FONT=my_font)

			wIndependentVarLabelButton = Widget_Button(wIndependentButton, $
				VALUE='Available Covariables', $
				SENSITIVE=0, UNAME='wIndependentVarLabel' ,FONT=my_font)
	
			wCardiacButton = Widget_Button(wIndependentButton, VALUE='Cardiac', $
				UVALUE='CARDIAC',SENSITIVE=0,/MENU,UNAME='wCardiacButton',FONT=my_font)

				wCardTransformLabelButton = Widget_Button(wCardiacButton, $
					VALUE='Supported Transforms', UVALUE='CARDIAC_TRANSFORM', $
					SENSITIVE=0, UNAME='wCardTransformLabel' ,FONT=my_font)

				wCardNoneButton = Widget_Button(wCardiacButton, $
					SENSITIVE=1, FONT=my_font, VALUE='None', UVALUE='CARDNONE', $
					UNAME='wCardNoneButton', /DYNAMIC_RESIZE)
		
				wCardPostEventButton = Widget_Button(wCardiacButton, FONT=my_font, $
					VALUE='Post-Event Interval',SENSITIVE=1,UVALUE='CARDPOSTEVENT' $
					,UNAME='wCardPostEventButton', /DYNAMIC_RESIZE)

				wCardPreEventButton = Widget_Button(wCardiacButton, FONT=my_font, $
					VALUE='Pre-Event Interval', SENSITIVE=1, UVALUE='CARDPREEVENT', $
					UNAME='wCardPreEventButton', /DYNAMIC_RESIZE)

				wCardRelativeCycleButton = Widget_Button(wCardiacButton, FONT=my_font, $
					VALUE='Relative Cycle Position', SENSITIVE=1, $
					UVALUE='CARDRELATIVEPOSITION', UNAME='wCardRelativeCycleButton' $
					,/DYNAMIC_RESIZE)

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
					VALUE='Convolution with HRF', SENSITIVE=1, $
					UVALUE='RESPCONVOLUTION', FONT=my_font, $
					UNAME='wRespConvolutionButton', /DYNAMIC_RESIZE)

				wRespSineButton = Widget_Button(wRespiratoryButton, FONT=my_font, $
					UNAME='wRespSineButton', SENSITIVE=1, VALUE='Sine', $
					UVALUE='RESPSINE', /DYNAMIC_RESIZE)
		
				wRespCosineButton = Widget_Button(wRespiratoryButton, $
					VALUE='Cosine', FONT=my_font, SENSITIVE=1, UVALUE='RESPCOSINE', $
					UNAME='wRespCosineButton', /DYNAMIC_RESIZE)

				wRespLookupButton = Widget_Button(wRespiratoryButton, FONT=my_font, $
					VALUE='Lookup Value', SENSITIVE=1, UVALUE='RESPLOOKUP', $
					UNAME='wRespLookupButton', /DYNAMIC_RESIZE)

			wStimulusButton = Widget_Button(wIndependentButton, FONT=my_font, $
				VALUE='Stimulus', SENSITIVE=0, UVALUE='STIMULUS', $
				UNAME='wStimulusButton', /MENU, /DYNAMIC_RESIZE)

				wStimTransformLabelButton = Widget_Button(wStimulusButton,FONT=my_font, $
					VALUE='Supported Transforms', UVALUE='STIMULUS_TRANSFORM', $
					SENSITIVE=0, UNAME='wStimTransformLabel', /DYNAMIC_RESIZE)

				wStimNoneButton = Widget_Button(wStimulusButton, FONT=my_font, $
					SENSITIVE=1, VALUE='None', UVALUE='STIMNONE', $
					UNAME='wStimNoneButton', /DYNAMIC_RESIZE)
		
				wStimPostEventButton = Widget_Button(wStimulusButton, FONT=my_font, $
					VALUE='Post-Event Interval',SENSITIVE=1,UVALUE='STIMPOSTEVENT', $
					UNAME='wStimPostEventButton', /DYNAMIC_RESIZE)

				wStimPreEventButton = Widget_Button(wStimulusButton, FONT=my_font, $
					VALUE='Pre-Event Interval', SENSITIVE=1, UVALUE='STIMPREEVENT', $
					UNAME='wStimPreEventButton', /DYNAMIC_RESIZE)

				wStimRelativeCycleButton = Widget_Button(wStimulusButton, FONT=my_font, $
					VALUE='Relative Cycle Position', SENSITIVE=1, /DYNAMIC_RESIZE $
					,UVALUE='STIMRELATIVEPOSITION', UNAME='wStimRelativeCycleButton')

				wStimConvolutionButton = Widget_Button(wStimulusButton, FONT=my_font, $
					VALUE='Convolution with HRF', SENSITIVE=1, /DYNAMIC_RESIZE, $
					UVALUE='STIMCONVOLUTION', UNAME='wStimConvolutionButton')

				wStimSineButton = Widget_Button(wStimulusButton, FONT=my_font, $
					SENSITIVE=1, VALUE='Sine', UVALUE='STIMSINE', $
					UNAME='wStimSineButton', /DYNAMIC_RESIZE)
		
				wStimCosineButton = Widget_Button(wStimulusButton, FONT=my_font, $
					VALUE='Cosine', SENSITIVE=1, UVALUE='STIMCOSINE', $
					UNAME='wStimCosineButton', /DYNAMIC_RESIZE)

				wStimLookupButton = Widget_Button(wStimulusButton, FONT=my_font, $
					VALUE='Lookup Value', SENSITIVE=1, UVALUE='STIMLOOKUP', $
					UNAME='wStimLookupButton', /DYNAMIC_RESIZE)

		wConstantButton = Widget_Button(wRegressionButton, VALUE='Remove Constant', $
			UVALUE='CONSTANT', UNAME='wConstantButton' ,FONT=my_font, /DYNAMIC_RESIZE)

		wTrendingButton = Widget_Button(wRegressionButton, VALUE='Include Linear Trend', $
			UVALUE='TRENDING', UNAME='wTrendingButton' ,FONT=my_font, /DYNAMIC_RESIZE)

		wGlobalButton = Widget_Button(wRegressionButton, VALUE='Include Global Average', $
			UVALUE='GLOBAL', UNAME='wGlobalButton' ,FONT=my_font, /DYNAMIC_RESIZE)

		wAutoRegressionButton = Widget_Button(wRegressionButton, FONT=my_font, $
			VALUE='First Order Autoregression Model', UVALUE='AUTOREGRESSION', $
			UNAME='wAutoRegressionButton', /DYNAMIC_RESIZE)

  		wWinLabel = Widget_Label(wAnalysisTopBase, UNAME='wWinLabel'  $
      		,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Sliding Window Width (images): ' $
      		,FONT=my_font)

  		wWinText = Widget_Text(wAnalysisTopBase, UNAME='wWinText'  $
      		,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS, XSIZE=4)

		wRegressSliceLabel = Widget_Label(wAnalysisTopBase, UNAME='wRegressSliceLabel'  $
      		,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Regressed Slice: ', FONT=my_font)

  		wRegressSliceText=Widget_Text(wAnalysisTopBase, UNAME='wRegressSliceText'  $
      		,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS, XSIZE=4)

		wAnalysisBottomBase=Widget_Base(wAnalysisBase,UNAME='wAnalysisBottomBase',/COLUMN)

		wAnalysisUpperBottomBase=Widget_Base(wAnalysisBottomBase, $
			UNAME='wAnalysisUpperBottomBase',/ROW)

		ROITypeValues = [ 'User Defined - Arbitrary', 'User Defined - Box', $
			'Threshold - One Run', 'Threshold - Two Runs' ]
  		wROITypeList = Widget_Droplist(wAnalysisUpperBottomBase, UNAME='wROITypeList'  $
      		,TITLE='ROI Type: ' ,VALUE=ROITypeValues ,FONT=my_font, UVALUE=ROITypeValues)

		wAnalysisMiddleBottomBase = Widget_Base(wAnalysisBottomBase, $
			UNAME = 'wAnalysisMiddleBottomBase', /ROW)

		wCompROI1Label = Widget_Label(wAnalysisMiddleBottomBase, UNAME='wCompROI1Label'  $
	      	,SENSITIVE=0 ,/ALIGN_LEFT ,VALUE='Run 1:' ,FONT=my_font)

		wCompROI1Text = Widget_Text(wAnalysisMiddleBottomBase, UNAME='wCompROI1Text'  $
	      	,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS, XSIZE=4,SENSITIVE=0)

		wCompROI2Label = Widget_Label(wAnalysisMiddleBottomBase, UNAME='wCompROI2Label'  $
	      	,SENSITIVE=0 ,/ALIGN_LEFT ,VALUE='Run 2:' ,FONT=my_font)

		wCompROI2Text = Widget_Text(wAnalysisMiddleBottomBase, UNAME='wCompROI2Text'  $
	      	,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS, XSIZE=4,SENSITIVE=0)

		OptionsListValues=['Raw Image','Sum of Squares','Regression Coefficient','Effect Size', $
			't-Statistic','F-Statistic','Phase Lag Sin and Cos','Sin and Cos F-Statistic', $
			'Magnitude Sin and Cos']
  		wOptionsList = Widget_Droplist(wAnalysisMiddleBottomBase, UNAME='wOptionsList' $
	      	,TITLE='Image Type: ' ,VALUE=OptionsListValues, FONT=my_font $
			,UVALUE=OptionsListValues,SENSITIVE=0)

		wROISliceLabel = Widget_Label(wAnalysisMiddleBottomBase, UNAME='wROISliceLabel'  $
	      	,/ALIGN_LEFT ,VALUE='Slice:' ,FONT=my_font, SENSITIVE=0)

		wROISliceText = Widget_Text(wAnalysisMiddleBottomBase, UNAME='wROISliceText'  $
	      	,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS, XSIZE=3,SENSITIVE=0)

		wAnalysisFinalBottomBase = Widget_Base(wAnalysisBottomBase, $
			UNAME = 'wAnalysisFinalBottomBase', /ROW)

		PositionListValues=[ 'Above', 'Below' ]
  		wPositionList = Widget_Droplist(wAnalysisFinalBottomBase, UNAME='wPositionList' $
	      	,TITLE='Position: ' ,VALUE=PositionListValues, FONT=my_font $
			,UVALUE=PositionListValues,SENSITIVE=0)

		wCutoffLabel = Widget_Label(wAnalysisFinalBottomBase, UNAME='wCutoffLabel'  $
	      	,SENSITIVE=0 ,/ALIGN_LEFT ,VALUE='Threshold:' ,FONT=my_font)

		wCutoffText = Widget_Text(wAnalysisFinalBottomBase, UNAME='wCutoffText'  $
	      	,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS, XSIZE=8,SENSITIVE=0)

		wROILabel = Widget_Label(wAnalysisFinalBottomBase, UNAME='wROILabel'  $
	      	,SENSITIVE=0 ,/ALIGN_LEFT ,VALUE='Create ROI' ,FONT=my_font)

  		wROIButton = Widget_Button(wAnalysisFinalBottomBase, UNAME='wROIButton'  $
	     	,FRAME=1 ,/ALIGN_CENTER ,VALUE='Create' ,FONT=my_font,SENSITIVE=0)

		wAnalysisVeryBottomBase = Widget_Base(wAnalysisBottomBase, $
			UNAME = 'wAnalysisVeryBottomBase', /ROW)

		wNoROILabel = Widget_Label(wAnalysisVeryBottomBase, UNAME='wNoROILabel'  $
	      	,SENSITIVE=0 ,/ALIGN_LEFT ,VALUE='Remove ROI' ,FONT=my_font)

  		wNoROIButton = Widget_Button(wAnalysisVeryBottomBase, UNAME='wNoROIButton'  $
	     		,FRAME=1 ,/ALIGN_CENTER ,VALUE='Remove' ,FONT=my_font,SENSITIVE=0)

		wExpandROILabel = Widget_Label(wAnalysisVeryBottomBase, UNAME='wExpandROILabel'  $
	      	,SENSITIVE=0 ,/ALIGN_LEFT ,VALUE='ROI Dilation' ,FONT=my_font)

		wExpandROIText = Widget_Text(wAnalysisVeryBottomBase, UNAME='wExpandROIText'  $
	      	,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS, XSIZE=8,SENSITIVE=0)		

		;wNoWindowLabel = Widget_Label(wAnalysisUltraBase, UNAME='wNoWindowLabel'  $
	    ;  	,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Click to Remove Sliding Window' ,FONT=my_font)

  		;wNoWindowButton = Widget_Button(wAnalysisUltraBase, UNAME='wNoWindowButton'  $
	    ; 		,FRAME=1 ,/ALIGN_CENTER ,VALUE='Remove' ,FONT=my_font)

  	wOutputBase = Widget_Base(wBigRightBase, UNAME='wOutputBase'  $
      		,TITLE='IDL' ,SPACE=3 ,/COLUMN)

  		wOutputLabel = Widget_Label(wOutputBase, UNAME='wOutputLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Output' ,FONT=title_font)

		wOutputTopBase = Widget_Base(wOutputBase, UNAME = 'OutputTopBase',/ROW)

  		wOutputButton = Widget_Button(wOutputTopBase, UNAME='wOutputButton'  $
      		,VALUE='Output Image Type', /MENU, /DYNAMIC_RESIZE, FONT=my_font, SENSITIVE=1)

			wOSOSButton = Widget_Button(wOutputButton, /MENU, $
				UNAME='wOSOSButton', /DYNAMIC_RESIZE, FONT=my_font, $
				VALUE='Sum of Squares', SENSITIVE=1)
				
				wOSOSCardButton = Widget_Button(wOSOSButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wOSOSCardButton', $
					VALUE='Cardiac', /DYNAMIC_RESIZE)
				
				wOSOSRespButton = Widget_Button(wOSOSButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wOSOSRespButton', $
					VALUE='Respiratory', /DYNAMIC_RESIZE)
				
				wOSOSStimButton = Widget_Button(wOSOSButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wOSOSStimButton', $
					VALUE='Stimulus', /DYNAMIC_RESIZE)

			wORegCoefButton = Widget_Button(wOutputButton, /MENU, $
				UNAME='wORegCoefButton', /DYNAMIC_RESIZE, FONT=my_font $
				,VALUE='Regression Coefficient', SENSITIVE=1)
				
				wORegCoefCardButton = Widget_Button(wORegCoefButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wORegCoefCardButton', $
					VALUE='Cardiac', /DYNAMIC_RESIZE)
				
				wORegCoefRespButton = Widget_Button(wORegCoefButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wORegCoefRespButton', $
					VALUE='Respiratory', /DYNAMIC_RESIZE)
				
				wORegCoefStimButton = Widget_Button(wORegCoefButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wORegCoefStimButton', $
					VALUE='Stimulus', /DYNAMIC_RESIZE)
				
			wOEffectButton = Widget_Button(wOutputButton, /MENU, $
				UNAME='wOEffectButton', /DYNAMIC_RESIZE, FONT=my_font $
				,VALUE='Effect Size', SENSITIVE=1)
				
				wOEffectCardButton = Widget_Button(wOEffectButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wOEffectCardButton', $
					VALUE='Cardiac', /DYNAMIC_RESIZE)
				
				wOEffectRespButton = Widget_Button(wOEffectButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wOEffectRespButton', $
					VALUE='Respiratory', /DYNAMIC_RESIZE)
				
				wOEffectStimButton = Widget_Button(wOEffectButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wOEffectStimButton', $
					VALUE='Stimulus', /DYNAMIC_RESIZE)
				
			wOFstatButton = Widget_Button(wOutputButton, /MENU, $
				UNAME='wOFstatButton', /DYNAMIC_RESIZE, FONT=my_font $
				,VALUE='F-statistic', SENSITIVE=1)
				
				wOFstatCardButton = Widget_Button(wOFstatButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wOFstatCardButton', $
					VALUE='Cardiac', /DYNAMIC_RESIZE)
				
				wOFstatRespButton = Widget_Button(wOFstatButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wOFstatRespButton', $
					VALUE='Respiratory', /DYNAMIC_RESIZE)
				
				wOFstatStimButton = Widget_Button(wOFstatButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wOFstatStimButton', $
					VALUE='Stimulus', /DYNAMIC_RESIZE)
				
			wOTstatButton = Widget_Button(wOutputButton, /MENU, $
				UNAME='wOTstatButton', /DYNAMIC_RESIZE, FONT=my_font $
				,VALUE='t-statistic', SENSITIVE=1)
				
				wOTstatCardButton = Widget_Button(wOTstatButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wOTstatCardButton', $
					VALUE='Cardiac', /DYNAMIC_RESIZE)
				
				wOTstatRespButton = Widget_Button(wOTstatButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wOTstatRespButton', $
					VALUE='Respiratory', /DYNAMIC_RESIZE)
				
				wOTstatStimButton = Widget_Button(wOTstatButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wOTstatStimButton', $
					VALUE='Stimulus', /DYNAMIC_RESIZE)
				
			wOErrorCoefButton = Widget_Button(wOutputButton, /MENU, $
				UNAME='wOErrorCoefButton', /DYNAMIC_RESIZE, FONT=my_font $
				,VALUE='Alt. t-statistic', SENSITIVE=1)
				
				wOErrorCoefCardButton = Widget_Button(wOErrorCoefButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wOErrorCoefCardButton', $
					VALUE='Cardiac', /DYNAMIC_RESIZE)
				
				wOErrorCoefRespButton = Widget_Button(wOErrorCoefButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wOErrorCoefRespButton', $
					VALUE='Respiratory', /DYNAMIC_RESIZE)
				
				wOErrorCoefStimButton = Widget_Button(wOErrorCoefButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wOErrorCoefStimButton', $
					VALUE='Stimulus', /DYNAMIC_RESIZE)
				
			wOCombocoefButton = Widget_Button(wOutputButton, $
				UNAME='wOCombocoefButton', /DYNAMIC_RESIZE, FONT=my_font $
				,VALUE='Sin and Cos Magnitude', SENSITIVE=1)
				
			wOCombofstatButton = Widget_Button(wOutputButton, $
				UNAME='wOCombofstatButton', /DYNAMIC_RESIZE, FONT=my_font $
				,VALUE='Sin and Cos F-stat', SENSITIVE=1)
				
			wOComboangleButton = Widget_Button(wOutputButton, $
				UNAME='wOComboangleButton', /DYNAMIC_RESIZE, FONT=my_font $
				,VALUE='Sin and Cos Phase Lag', SENSITIVE=1)

  		wStripchartButton = Widget_Button(wOutputTopBase, /MENU, SENSITIVE=1, $
	      	UNAME='wStripchartButton', FONT=my_font, /DYNAMIC_RESIZE, VALUE='Stripchart Type')

			wCSOSButton = Widget_Button(wStripchartButton, /MENU, $
				UNAME='wCSOSButton', /DYNAMIC_RESIZE, FONT=my_font, $
				VALUE='Sum of Squares', SENSITIVE=1)
				
				wCSOSCardButton = Widget_Button(wCSOSButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wCSOSCardButton', $
					VALUE='Cardiac', /DYNAMIC_RESIZE)
				
				wCSOSRespButton = Widget_Button(wCSOSButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wCSOSRespButton', $
					VALUE='Respiratory', /DYNAMIC_RESIZE)
				
				wCSOSStimButton = Widget_Button(wCSOSButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wCSOSStimButton', $
					VALUE='Stimulus', /DYNAMIC_RESIZE)

			wCRegCoefButton = Widget_Button(wStripchartButton, /MENU, $
				UNAME='wCRegCoefButton', /DYNAMIC_RESIZE, FONT=my_font $
				,VALUE='Regression Coefficient', SENSITIVE=1)
				
				wCRegCoefCardButton = Widget_Button(wCRegCoefButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wCRegCoefCardButton', $
					VALUE='Cardiac', /DYNAMIC_RESIZE)
				
				wCRegCoefRespButton = Widget_Button(wCRegCoefButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wCRegCoefRespButton', $
					VALUE='Respiratory', /DYNAMIC_RESIZE)
				
				wCRegCoefStimButton = Widget_Button(wCRegCoefButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wCRegCoefStimButton', $
					VALUE='Stimulus', /DYNAMIC_RESIZE)
				
			wCEffectButton = Widget_Button(wStripchartButton, /MENU, $
				UNAME='wCEffectButton', /DYNAMIC_RESIZE, FONT=my_font $
				,VALUE='Effect Size', SENSITIVE=1)
				
				wCEffectCardButton = Widget_Button(wCEffectButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wCEffectCardButton', $
					VALUE='Cardiac', /DYNAMIC_RESIZE)
				
				wCEffectRespButton = Widget_Button(wCEffectButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wCEffectRespButton', $
					VALUE='Respiratory', /DYNAMIC_RESIZE)
				
				wCEffectStimButton = Widget_Button(wCEffectButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wCEffectStimButton', $
					VALUE='Stimulus', /DYNAMIC_RESIZE)
				
			wCFstatButton = Widget_Button(wStripchartButton, /MENU, $
				UNAME='wCFstatButton', /DYNAMIC_RESIZE, FONT=my_font $
				,VALUE='F-statistic', SENSITIVE=1)
				
				wCFstatCardButton=Widget_Button(wCFstatButton,SENSITIVE=0, $
					FONT=my_font,UNAME='wCFstatCardButton',VALUE='Cardiac', $
					/DYNAMIC_RESIZE)
				
				wCFstatRespButton = Widget_Button(wCFstatButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wCFstatRespButton', $
					VALUE='Respiratory', /DYNAMIC_RESIZE)
				
				wCFstatStimButton = Widget_Button(wCFstatButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wCFstatStimButton', $
					VALUE='Stimulus', /DYNAMIC_RESIZE)
				
			wCTstatButton = Widget_Button(wStripchartButton, /MENU, $
				UNAME='wCTstatButton', /DYNAMIC_RESIZE, FONT=my_font $
				,VALUE='t-statistic', SENSITIVE=1)
				
				wCTstatCardButton = Widget_Button(wCTstatButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wCTstatCardButton', $
					VALUE='Cardiac', /DYNAMIC_RESIZE)
				
				wCTstatRespButton = Widget_Button(wCTstatButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wCTstatRespButton', $
					VALUE='Respiratory', /DYNAMIC_RESIZE)
				
				wCTstatStimButton = Widget_Button(wCTstatButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wCTstatStimButton', $
					VALUE='Stimulus', /DYNAMIC_RESIZE)
				
			wCErrorCoefButton = Widget_Button(wStripchartButton, /MENU, $
				UNAME='wCErrorCoefButton', /DYNAMIC_RESIZE, FONT=my_font $
				,VALUE='Reg Coef with Error', SENSITIVE=1)
				
				wCErrorCoefCardButton = Widget_Button(wCErrorCoefButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wCErrorCoefCardButton', $
					VALUE='Cardiac', /DYNAMIC_RESIZE)
				
				wCErrorCoefRespButton = Widget_Button(wCErrorCoefButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wCErrorCoefRespButton', $
					VALUE='Respiratory', /DYNAMIC_RESIZE)
				
				wCErrorCoefStimButton = Widget_Button(wCErrorCoefButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wCErrorCoefStimButton', $
					VALUE='Stimulus', /DYNAMIC_RESIZE)
				
			wCErrorTstatButton = Widget_Button(wStripchartButton, /MENU, $
				UNAME='wCErrorTstatButton', /DYNAMIC_RESIZE, FONT=my_font $
				,VALUE='Alt. t-statistic', SENSITIVE=1)
				
				wCErrorTstatCardButton = Widget_Button(wCErrorTstatButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wCErrorTstatCardButton', $
					VALUE='Cardiac', /DYNAMIC_RESIZE)
				
				wCErrorTstatRespButton = Widget_Button(wCErrorTstatButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wCErrorTstatRespButton', $
					VALUE='Respiratory', /DYNAMIC_RESIZE)
				
				wCErrorTstatStimButton = Widget_Button(wCErrorTstatButton, SENSITIVE=0, $
					FONT=my_font, UNAME='wCErrorTstatStimButton', $
					VALUE='Stimulus', /DYNAMIC_RESIZE)
			
			wCCardiacButton = Widget_Button(wStripchartButton, $
				UNAME='wCCardiacButton', /DYNAMIC_RESIZE, FONT=my_font $
				,VALUE='Cardiac', SENSITIVE=1)
			
			wCRespiratoryButton = Widget_Button(wStripchartButton, $
				UNAME='wCRespiratoryButton', /DYNAMIC_RESIZE, FONT=my_font $
				,VALUE='Respiratory', SENSITIVE=1)
			
			wCStimulusButton = Widget_Button(wStripchartButton, $
				UNAME='wCStimulusButton', /DYNAMIC_RESIZE, FONT=my_font $
				,VALUE='Stimulus', SENSITIVE=1)
			
			wCImgAcqButton = Widget_Button(wStripchartButton, $
				UNAME='wCImgAcqButton', /DYNAMIC_RESIZE, FONT=my_font $
				,VALUE='Image Acquisition', SENSITIVE=1)
			
			wCSelectRawButton = Widget_Button(wStripchartButton, $
				UNAME='wCSelectRawButton', /DYNAMIC_RESIZE, FONT=my_font $
				,VALUE='Mean of ROI Pixels', SENSITIVE=1)
			
			wCMotionButton = Widget_Button(wStripchartButton, $
				UNAME='wCMotionButton', /DYNAMIC_RESIZE, FONT=my_font $
				,VALUE='Motion Detection', SENSITIVE=1)
			
			wCCombocoefButton = Widget_Button(wStripchartButton, $
				UNAME='wCCombocoefButton', /DYNAMIC_RESIZE, FONT=my_font $
				,VALUE='Sin and Cos Magnitude', SENSITIVE=1)
			
			wCCombofstatButton = Widget_Button(wStripchartButton, $
				UNAME='wCCombofstatButton', /DYNAMIC_RESIZE, FONT=my_font $
				,VALUE='Sin and Cos F-stat', SENSITIVE=1)

  		wConIntervalLabel = Widget_Label(wOutputTopBase, UNAME='wConIntervalLabel'  $
 	     		,/ALIGN_LEFT ,VALUE='95% Confidence Interval:', $
				FONT=my_font, SENSITIVE=0)

		wConIntervalText = Widget_Text(wOutputTopBase, UNAME='wConIntervalText'  $
	    	,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS, XSIZE=8, SENSITIVE=0)

		wOutputMiddleBase = Widget_Base(wOutputBase, UNAME='OutputMiddleBase', /ROW)

  		wOverlayLabel = Widget_Label(wOutputMiddleBase, UNAME='wOverlayLabel'  $
 	     		,/ALIGN_LEFT ,VALUE='Overlay Threshold:', FONT=my_font, SENSITIVE=1)

		wOverlayText = Widget_Text(wOutputMiddleBase, UNAME='wOverlayText'  $
	    	,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS, XSIZE=8, SENSITIVE=1)	
				
		feedbackListValues=[ 'None','Image - Visual','Image - Auditory','Stripchart' ]
  		wFeedbackList = Widget_Droplist(wOutputMiddleBase,  $
      			UNAME='wFeedbackList' ,TITLE='Subject Feedback: ', $
      			VALUE=feedbackListValues ,FONT=my_font, UVALUE=feedbackListValues)	

		wOutputBottomBase = Widget_Base(wOutputBase, UNAME = 'OutputBottomBase' ,/ROW)

		wOutputEndBase = Widget_Base(wOutputBase, UNAME = 'OutputEndBase' ,/ROW)

		outputFileListValues = [ 'Static', 'CVIO' ]
		wOutputFileList = Widget_Droplist(wOutputEndBase,  $
      		UNAME='wOutputFileList' ,TITLE='Output Data File Type: ' $
	      	,VALUE=outputFileListValues ,FONT=my_font, UVALUE=outputFileListValues)

		wSaveLabel = Widget_Label(wOutputEndBase, UNAME='wSaveLabel',SENSITIVE=1 $
			,/ALIGN_LEFT ,VALUE='Click to Save to Files' ,FONT=my_font)

  		wSaveButton = Widget_Button(wOutputEndBase, UNAME='wSaveButton'  $
	      		,FRAME=1 ,/ALIGN_CENTER ,VALUE='Save' ,FONT=my_font)

	wBigLeftBase = Widget_Base(wBigBase, UNAME = 'wBigLeftBase' ,/COLUMN)

  	wBasePictures=Widget_Base(wBigLeftBase,UNAME='wBasePictures',TITLE='IDL',SPACE=3,/COLUMN)

		wBaseTopPictures = Widget_Base(wBasePictures, UNAME='wBaseTopPictures' ,/ROW)

		wBaseTopOutputPictures= Widget_Base(wBaseTopPictures, $
			UNAME='wBaseTopOutputPictures' ,/COLUMN)

  		wDrawOutput = Widget_Draw(wBaseTopOutputPictures, UNAME='wDrawOutput'  $
	      	 ,GRAPHICS_LEVEL=1 ,SCR_XSIZE=256 ,SCR_YSIZE=256 )

		wDrawOutputLabel = Widget_Label(wBaseTopOutputPictures, UNAME='wDrawOutputLabel'  $
	      	,SENSITIVE=1 ,VALUE='Output Image' ,FONT=my_font)

		wBaseTopChartPictures=Widget_Base(wBaseTopPictures, $
			UNAME='wBaseTopChartPictures' ,/COLUMN)

  		wDrawChart = Widget_Draw(wBaseTopChartPictures, UNAME='wDrawChart' $
			,GRAPHICS_LEVEL=1 ,SCR_XSIZE=256 ,SCR_YSIZE=256 )

  		wDrawChartLabel = Widget_Label(wBaseTopChartPictures, UNAME='wDrawChartLabel'  $
	      	,SENSITIVE=1 ,VALUE='Strip Chart' ,FONT=my_font)

		wBaseBottomPictures=Widget_Base(wBasePictures,UNAME='wBaseBottomPictures',/ROW)

		wBaseBottomStimPictures= Widget_Base(wBaseBottomPictures, $
			UNAME='wBaseBottomStimPictures' ,/COLUMN)

  		wDrawStimulus = Widget_Draw(wBaseBottomStimPictures, UNAME='wDrawStimulus'  $
	      		,GRAPHICS_LEVEL=1 ,SCR_XSIZE=256 ,SCR_YSIZE=256 )

		wDrawStimulusLabel = Widget_Label(wBaseBottomStimPictures,  $
			UNAME='wDrawStimulusLabel' ,SENSITIVE=1 ,VALUE='Image Overlay' ,FONT=my_font)

		wBaseBottomOrigPictures= Widget_Base(wBaseBottomPictures, $
			UNAME='wBaseBottomOrigPictures' ,/COLUMN)

  		wDrawOriginal = Widget_Draw(wBaseBottomOrigPictures, UNAME='wDrawOriginal'  $
	      		,GRAPHICS_LEVEL=1 ,SCR_XSIZE=256 ,SCR_YSIZE=256)

  		wDrawOriginalLabel = Widget_Label(wBaseBottomOrigPictures, SENSITIVE=1,  $
			UNAME='wDrawOriginalLabel', VALUE='Original Image' ,FONT=my_font)

 	wUpdateBase=Widget_Base(wBigLeftBase,UNAME='wUpdateBase',TITLE='IDL',SPACE=3,/COLUMN)

  		wUpdateText=Widget_Text(wUpdateBase,UNAME='wUpdateText',FONT=my_font,SCR_XSIZE=512)

  	wExitBase=Widget_Base(wBigLeftBase,UNAME='wExitBase',TITLE='IDL',SPACE=3,/COLUMN)

		wUpperExitBase = WIDGET_BASE(wExitBase, UNAME = 'wUpperExitBase' ,/ROW)

			wScaleMaxLabel = Widget_Label(wUpperExitBase, UNAME='wScaleMaxLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Scale Max: ' ,FONT=my_font)

  			wScaleMaxText = Widget_Text(wUpperExitBase, UNAME='wScaleMaxText' $
 				,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS, XSIZE=8)

		wLowerExitBase = WIDGET_BASE(wExitBase, UNAME = 'wLowerExitBase' ,/ROW)
		
			wPrepareButton = Widget_Button(wLowerExitBase, UNAME='wPrepareButton'  $
     			,FRAME=1 ,VALUE='PREP' ,FONT=my_font)

			wGoButton = Widget_Button(wLowerExitBase, UNAME='wGoButton'  $
     			,FRAME=1 ,VALUE='GO' ,FONT=my_font, SENSITIVE=0)

			wStopButton = Widget_Button(wLowerExitBase, UNAME='wStopButton'  $
     			,FRAME=1 ,VALUE='STOP' ,FONT=my_font)
	
  			wContinueButton = Widget_Button(wLowerExitBase, UNAME='wContinueButton'  $
     			,FRAME=1 ,VALUE='CONT' ,FONT=my_font)
		
			wClearButton = Widget_Button(wLowerExitBase, UNAME='wClearButton'  $
     			,FRAME=1 ,VALUE='CLEAR' ,FONT=my_font)
		
			wTrackerButton = Widget_Button(wLowerExitBase, UNAME='wTrackerButton'  $
     			,FRAME=1 ,VALUE='TRACK' ,FONT=my_font)		
		
		wBottomExitBase = WIDGET_BASE(wExitBase, UNAME = 'wBottomExitBase' ,/ROW)

			wStimulusDirLabel = Widget_Label(wBottomExitBase, UNAME='wStimulusDirLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Stimulus Directory: ' ,FONT=my_font)

  			wStimulusDirText = Widget_Text(wBottomExitBase, UNAME='wStimulusDirText' $
 				,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS, XSIZE=40)

		wJunkExitBase = WIDGET_BASE(wExitBase, UNAME = 'JunkExitBase' ,/ROW)

			wDataDirLabel = Widget_Label(wJunkExitBase, UNAME='wDataDirLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Disk Data Directory: ' ,FONT=my_font)

  			wDataDirText = Widget_Text(wJunkExitBase, UNAME='wDataDirText' $
 				,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS, XSIZE=40)

		wLowestExitBase = WIDGET_BASE(wExitBase, UNAME = 'wLowestExitBase' ,/ROW)

			wOutputDirLabel = Widget_Label(wLowestExitBase, UNAME='wOutputDirLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Output Directory: ' ,FONT=my_font)

  			wOutputDirText = Widget_Text(wLowestExitBase, UNAME='wOutputDirText' $
 				,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS, XSIZE=40)

		wAbsoluteExitBase = WIDGET_BASE(wExitBase, UNAME = 'wAbsoluteExitBase' ,/ROW)

			wLookupDirLabel = Widget_Label(wAbsoluteExitBase, UNAME='wLookupDirLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Lookup Table Directory: ', FONT=my_font)

  			wLookupDirText = Widget_Text(wAbsoluteExitBase, UNAME='wLookupDirText' $
 				,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS, XSIZE=40)

		wLookatmeExitBase = WIDGET_BASE(wExitBase, UNAME = 'wLookatmeExitBase' ,/ROW)

			wRandomDirLabel = Widget_Label(wLookatmeExitBase, UNAME='wRandomDirLabel'  $
      			,SENSITIVE=1 ,/ALIGN_LEFT ,VALUE='Random Directory: ', FONT=my_font)

  			wRandomDirText = Widget_Text(wLookatmeExitBase, UNAME='wRandomDirText' $
 				,/EDITABLE ,VALUE=[ '' ] ,FONT=my_font, /ALL_EVENTS, XSIZE=40)

	; Create a structure to hold information required by the program

  	sWidget = { orig_sizex : 128, $
  		    orig_sizey : 128, $
		    disp_sizex : disp_sizex, $
		    disp_sizey : disp_sizey, $
		    stimulus1_dir : '/home/people/chris/project/video_stim/blackandwhite/', $
		    stimulus2_dir : '/home/people/chris/project/video_stim/feedback/', $
		    stimulus3_dir : '/home/people/chris/project/video_stim/pictures/', $
		    stimulus4_dir : '/home/people/chris/project/video_stim/haller/', $
		    cvioData_dir : '/home/people/chris/rjf/cvio_data/', $
		    random_dir : '/home/people/chris/project/video_stim/random/', $
		    lookuptable_dir : './', $
		    output_dir : '/home/people/chris/rjf/cvio_data/', $
		    wDemoBase : wDemoBase, $
		    wPIDText : wPIDText, $
		    wRunNumText : wRunNumText, $
		    wDateText : wDateText, $
		    wRunLabText : wRunLabText, $
		    wDesignBase : wDesignBase, $
		    wStimTypeList : wStimTypeList, $
		    wStimLiButton : wStimLiButton, $
		    wStimNamingButton : wStimNamingButton, $
		    wStimWordButton : wStimWordButton, $
		    wStimVisualButton : wStimVisualButton, $
		    wPolarButton : wPolarButton, $
		    wNewPolar20Button : wNewPolar20Button, $
		    wNewPolar30Button : wNewPolar30Button, $
		    wEccentricButton : wEccentricButton, $
		    wNewEccentric20Button : wNewEccentric20Button, $
		    wNewEccentric30Button : wNewEccentric30Button, $
		    wCombinedButton : wCombinedButton, $
		    wNewCombinedr30e20Button : wNewCombinedr30e20Button, $
		    wNewCombinedr20e30Button : wNewCombinedr20e30Button, $
		    wNewCombinedr20e302plus8Button : wNewCombinedr20e302plus8Button, $
			wTomSpecialButton : wTomSpecialButton, $
		    w2HzHemiButton : w2HzHemiButton, $
		    w4HzHemiButton : w4HzHemiButton, $
		    w8HzHemiButton : w8HzHemiButton, $
		    wFeedbackStimButton : wFeedbackStimButton, $
		    wEccFeedbackStimButton : wEccFeedbackStimButton, $
		    wRandomButton : wRandomButton, $
		    wThreeButton : wThreeButton, $
		    wFourButton : wFourButton, $
		    wBlockButton : wBlockButton, $
		    wStemsButton : wStemsButton, $
		    wLettersButton : wLettersButton, $
		    wWords1Button : wWords1Button, $
		    wWords2Button : wWords2Button, $
		    wWords3Button : wWords3Button, $
		    wWords4Button : wWords4Button, $
		    wBulbButton : wBulbButton, $
		    wISIText : wISIText, $
		    wISILabel : wISILabel, $
		    wRandomText : wRandomText, $
		    wRandomLabel : wRandomLabel, $
		    wJitterList : wJitterList, $
		    wDurationText : wDurationText, $
		    wPeriodText : wPeriodText, $
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
		    wTaggedImgButton : wTaggedImgButton, $
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
		    wTrendingButton : wTrendingButton, $
		    wGlobalButton : wGlobalButton, $
		    wAutoRegressionButton : wAutoRegressionButton, $
		    wOrderList : wOrderList, $
		    wAnalysisBase : wAnalysisBase, $
		    wWinText : wWinText, $
		    wRegressSliceText : wRegressSliceText, $
  			wROITypeList : wROITypeList, $
			wCompROI1Text : wCompROI1Text, $
			wCompROI1Label : wCompROI1Label, $
			wCompROI2Text : wCompROI2Text, $
			wCompROI2Label : wCompROI2Label, $
		    wOptionsList : wOptionsList, $
		    wROISliceText : wROISliceText, $
		    wROISliceLabel : wROISliceLabel, $
		    wPositionList : wPositionList, $
			wCutoffText : wCutoffText, $
			wCutoffLabel : wCutoffLabel, $
			wExpandROIText : wExpandROIText, $
			wExpandROILabel : wExpandROILabel, $
		    wROIButton : wROIButton, $
		    wROILabel : wROILabel, $
		    wNoROIButton : wNoROIButton, $
		    wNoROILabel : wNoROILabel, $
		    wOutputBase : wOutputBase, $
		    wOutputButton : wOutputButton, $
		    wOSOSButton : wOSOSButton, $
		    wOSOSCardButton : wOSOSCardButton, $
		    wOSOSRespButton : wOSOSRespButton, $
		    wOSOSStimButton : wOSOSStimButton, $
		    wORegCoefButton : wORegCoefButton, $
		    wORegCoefCardButton : wORegCoefCardButton, $
		    wORegCoefStimButton : wORegCoefStimButton, $
		    wORegCoefRespButton : wORegCoefRespButton, $
		    wOEffectButton : wOEffectButton, $
		    wOEffectCardButton : wOEffectCardButton, $
		    wOEffectRespButton : wOEffectRespButton, $
		    wOEffectStimButton : wOEffectStimButton, $
		    wOFstatButton : wOFstatButton, $
		    wOFstatCardButton : wOFstatCardButton, $
		    wOFstatRespButton : wOFstatRespButton, $
		    wOFstatStimButton : wOFstatStimButton, $
		    wOTstatButton : wOTstatButton, $
		    wOTstatCardButton : wOTstatCardButton, $
		    wOTstatRespButton : wOTstatRespButton, $
		    wOTstatStimButton : wOTstatStimButton, $
		    wOErrorCoefButton : wOErrorCoefButton, $
		    wOErrorCoefCardButton : wOErrorCoefCardButton, $
		    wOErrorCoefRespButton : wOErrorCoefRespButton, $
		    wOErrorCoefStimButton : wOErrorCoefStimButton, $
		    wOCombocoefButton : wOCombocoefButton, $
		    wOComboangleButton : wOComboangleButton, $
		    wOCombofstatButton : wOCombofstatButton, $
		    wFeedbackList : wFeedbackList, $
		    wStripchartButton : wStripchartButton, $
		    wCSOSButton : wCSOSButton, $
		    wCSOSCardButton : wCSOSCardButton, $
		    wCSOSRespButton : wCSOSRespButton, $
		    wCSOSStimButton : wCSOSStimButton, $
		    wCRegCoefButton : wCRegCoefButton, $
		    wCRegCoefCardButton : wCRegCoefCardButton, $
		    wCRegCoefStimButton : wCRegCoefStimButton, $
		    wCRegCoefRespButton : wCRegCoefRespButton, $
		    wCEffectButton : wCEffectButton, $
		    wCEffectCardButton : wCEffectCardButton, $
		    wCEffectRespButton : wCEffectRespButton, $
		    wCEffectStimButton : wCEffectStimButton, $
		    wCFstatButton : wCFstatButton, $
		    wCFstatCardButton : wCFstatCardButton, $
		    wCFstatRespButton : wCFstatRespButton, $
		    wCFstatStimButton : wCFstatStimButton, $
		    wCTstatButton : wCTstatButton, $
		    wCTstatCardButton : wCTstatCardButton, $
		    wCTstatRespButton : wCTstatRespButton, $
		    wCTstatStimButton : wCTstatStimButton, $
		    wCErrorCoefButton : wCErrorCoefButton, $
		    wCErrorCoefCardButton : wCErrorCoefCardButton, $
		    wCErrorCoefRespButton : wCErrorCoefRespButton, $
		    wCErrorCoefStimButton : wCErrorCoefStimButton, $
		    wCErrorTstatButton : wCErrorTstatButton, $
		    wCErrorTstatCardButton : wCErrorTstatCardButton, $
		    wCErrorTstatRespButton : wCErrorTstatRespButton, $
		    wCErrorTstatStimButton : wCErrorTstatStimButton, $
		    wCCardiacButton : wCCardiacButton, $
		    wCRespiratoryButton : wCRespiratoryButton, $
		    wCStimulusButton : wCStimulusButton, $
		    wCImgAcqButton : wCImgAcqButton, $
		    wCSelectRawButton : wCSelectRawButton, $
		    wCMotionButton : wCMotionButton, $
		    wCCombofstatButton : wCCombofstatButton, $
		    wConIntervalText : wConIntervalText, $
		    wOverlayText : wOverlayText, $
		    wConIntervalLabel : wConIntervalLabel, $
		    wCCombocoefButton : wCCombocoefButton, $
		    ;wInputFileText : wInputFileText, $
		    wOutputFileList : wOutputFileList, $
		    ;wOutputFileText : wOutputFileText, $
		    wSaveButton : wSaveButton, $
		    wBasePictures : wBasePictures, $
		    wDrawOutput : wDrawOutput, $
		    wDrawChart : wDrawChart, $
		    wDrawStimulus : wDrawStimulus, $
		    wDrawOriginal : wDrawOriginal, $
		    wUpdateBase : wUpdateBase, $
		    wUpdateText : wUpdateText, $
		    wUpperExitBase : wUpperExitBase, $
		    wLowerExitBase : wLowerExitBase, $
		    wBottomExitBase : wBottomExitBase, $
		    wJunkExitBase : wJunkExitBase, $
		    wLowestExitBase : wLowestExitBase, $
		    wAbsoluteExitBase : wAbsoluteExitBase, $
		    wLookatmeExitBase : wLookatmeExitBase, $
		    wClearButton : wClearButton, $
		    wOutputDirText : wOutputDirText, $
		    wLookupDirText : wLookupDirText, $
		    wRandomDirText : wRandomDirText, $
		    wDataDirText : wDataDirText, $
		    wStimulusDirText : wStimulusDirText, $
		    wPrepareButton : wPrepareButton, $
		    wTrackerButton : wTrackerButton, $
		    wGoButton : wGoButton, $
		    wStopButton : wStopButton, $
		    wContinueButton : wContinueButton, $
		    wScaleMaxText : wScaleMaxText, $
		    wXDimensionText : wXDimensionText, $
		    wYDimensionText : wYDimensionText, $
		    CardShift : -1, $
		    RespShift : -1, $
		    StimShift : -1, $
		    orig_win_id : -1, $
		    stim_win_id : -1, $
		    result_win_id : -1, $
		    chart_win_id : -1, $
		    PID : '', $
		    RunNum : '', $
		    Number_Entered : -1, $
		    Num_Vars : 1, $
		    file1 : '', $
		    file2 : '', $
		    file3 : '', $
		    file4 : '', $
		    file5 : '', $
		    file6 : '', $
		    file7 : '', $
		    file8 : '', $
		    file9 : '', $
		    file10 : '', $
		    Date : '', $
		    RunLabel : '', $
		    StimType : '', $
		    StimulusName : '', $
		    StimulusName2 : '', $
		    StimSizex : -1, $
		    StimSizey : -1, $
		    block_click : -1, $
		    event_click : -1, $
		    StimSelect : -1, $
		    ISI : -1, $
		    Jitter : '', $
		    Duration : -1, $
		    Period : -1, $
		    Rate : -1, $
		    Source : -1, $
		    NumImg : -1, $
		    NumSli : -1, $
		    Order : -1, $
		    Channels : 0, $
		    Acquire_Channels : [-1,-1,-1,-1], $
		    Acquire_Files : ['','','',''], $
		    IndependentCheck : -1, $
		    ROIFile : '', $
		    ROISlice : '', $
		    ROIx : 0, $
		    ROIy : 0, $
		    ROIdx : 128, $
		    ROIdy : 128, $
			Expand : -1, $
		    ROIType : '', $
			CompROI1 : '', $
			CompROI2 : '', $
			CompROICut : '', $
			Final_Image : fltarr(256,256), $
			Store_ROI : lonarr(65536), $
			Comp_Store_ROI : lonarr(65536), $
		    SlidingWindow : -1, $
		    SlidingWindowCounter : -1, $
		    RegressSlice : -1, $
		    GetPicture : -1, $
		    FeedbackType : '', $
		    PositionType : '', $
		    OptionsType : '', $
		    StripchartTypeOutput : -1, $
		    ResultTypeOutput : -1, $
		    ConfidenceInterval : 2, $
		    Overlay : -1., $
		    Constant : 1, $
			Trending : -1, $
			Global : -1, $
		    Cardiac_Regress : -1, $
		    Multi_Card_Regress : 0, $
		    Respiratory_Regress : -1, $
		    Multi_Resp_Regress : 0, $
		    Stimulus_Regress : -1, $
		    Multi_Stim_Regress : 0, $
		    StimStart : -1, $
		    Card_None : -1, $
		    Card_Post : -1, $
		    Card_Pre : -1, $
		    Card_Conv : -1, $
		    Card_Sin : -1, $
		    Card_Cos : -1, $
		    Card_Look : -1, $
		    Card_Rel : -1, $
		    Resp_None : -1, $
		    Resp_Post : -1, $
		    Resp_Pre : -1, $
		    Resp_Conv : -1, $
		    Resp_Sin : -1, $
		    Resp_Cos : -1, $
		    Resp_Look : -1, $
		    Resp_Rel : -1, $
		    Stim_None : -1, $
		    Stim_Post : -1, $
		    Stim_Pre : -1, $
		    Stim_Conv : -1, $
		    Stim_Sin : -1, $
		    Stim_Cos : -1, $
		    Stim_Look : -1, $
		    Stim_Rel : -1, $
	 	    StimStatus : '', $
		    InputFile : '', $
		    OutputFileType : '', $
		    OutputFile : '', $
		    OriginalOutputFile : '', $
		    Save : -1, $
		    CardiacCheck : -1, $
		    RespiratoryCheck : -1, $
		    ImagAcqCheck : -1, $
		    StimulusCheck : -1, $
		    Mask : -1, $
		    BaseNum : '', $
		    BaseNum2 : '', $
		    ScaleMax : -1., $
			Random_File : '', $
		    Already_Regressed : -1}

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
