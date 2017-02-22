###########################################################################
#                                                                         #
#  Praat Script Syllable Nuclei                                           #
#  Copyright (C) 2008  Nivja de Jong and Ton Wempe                        #
#                                                                         #
#    This program is free software: you can redistribute it and/or modify #
#    it under the terms of the GNU General Public License as published by #
#    the Free Software Foundation, either version 3 of the License, or    #
#    (at your option) any later version.                                  #
#                                                                         #
#    This program is distributed in the hope that it will be useful,      #
#    but WITHOUT ANY WARRANTY; without even the implied warranty of       #
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        #
#    GNU General Public License for more details.                         #
#                                                                         #
#    You should have received a copy of the GNU General Public License    #
#    along with this program.  If not, see http://www.gnu.org/licenses/   #
#                                                                         #
###########################################################################

# changed 19.09.2013 by Andreas Seiderer

# console version for one file (removed unnecssary file output steps)

# added voice report that uses the same wav file without reload

###########################################################################

# updated 09.17.2010 by Nivja de Jong, Hugo Quené, Ingrid Persoon

# + adapted to also run on MAC-computers

# + refer to objects by unique identifier, not by name

# + keep track of all created intermediate objects, select these explicitly, 

#     then Remove

# + programming of checking loop for mindip adjusted

###########################################################################



# counts syllables of all sound utterances in a directory
# NB unstressed syllables are sometimes overlooked
# NB filter sounds that are quite noisy beforehand
# NB use Ignorance Level/Intensity Median (dB) = 0 for unfiltered sounds, 
#    use 2 for filtered sounds
# NB use Minimum dip between peaks (dB) = 2 for unfiltered sounds,
#    use 4 for filtered sounds


form Counting Syllables in Sound Utterances
   real Ignorance_Level/Intensity_Median_(dB) 0 or 2
   real Minimum_dip_between_peaks_(dB) 2 or 4
   boolean display_name yes
   sentence soundFileName 'audiofile.wav'
endform

 

# shorten variables
iglevel = 'ignorance_Level/Intensity_Median'
mindip = 'minimum_dip_between_peaks'


sound = Read from file... 'soundFileName$'

obj$ = selected$("Sound")

soundid = selected("Sound")


# voice report
  pitch = To Pitch (cc)... 0.01 50 15 no 0.03 0.45 0.01 0.35 0.14 300
  plus sound
  pulses = To PointProcess (cc)
  plus sound
  plus pitch
  report$ = Voice report... 0 0 50 300 1.3 1.6 0.03 0.45

  
  # extract numbers (modified from: http://www.linguistics.ucla.edu/faciliti/facilities/acoustic/voice_rep_from_object_window.txt)
  
  medianPitch = extractNumber (report$, "Median pitch: ")
  meanPitch = extractNumber (report$, "Mean pitch: ")
  sdPitch =extractNumber (report$, "Standard deviation: ")
  minPitch = extractNumber (report$, "Minimum pitch: ")
  maxPitch = extractNumber (report$, "Maximum pitch: ")
  
  nPulses = extractNumber (report$, "Number of pulses: ")
  nPeriods = extractNumber (report$, "Number of periods: ")
  meanPeriod = extractNumber (report$, "Mean period: ")
  sdPeriod = extractNumber (report$, "Standard deviation of period: ")
  
  pctUnvoiced = extractNumber (report$, "Fraction of locally unvoiced frames: ") * 100
  nVoicebreaks = extractNumber (report$, "Number of voice breaks: ")
  pctVoicebreaks = extractNumber (report$, "Degree of voice breaks: ") * 100
  degreeVoicebreaks$ = extractLine$ (report$, "Degree ") 
  
  jitter_loc = extractNumber (report$, "Jitter (local): ") * 100
  jitter_loc_abs = extractNumber (report$, "Jitter (local, absolute): ")
  jitter_rap = extractNumber (report$, "Jitter (rap): ") * 100
  jitter_ppq5 = extractNumber (report$, "Jitter (ppq5): ") * 100
  jitter_ddp = extractNumber (report$, "Jitter (ddp): ") * 100
  
  shimmer_loc = extractNumber (report$, "Shimmer (local): ") * 100
  shimmer_loc_dB = extractNumber (report$, "Shimmer (local, dB): ")
  shimmer_apq3 = extractNumber (report$, "Shimmer (apq3): ") * 100
  shimmer_apq5 = extractNumber (report$, "Shimmer (apq5): ") * 100
  shimmer_apq11 = extractNumber (report$, "Shimmer (apq11): ") * 100
  shimmer_dda = extractNumber (report$, "Shimmer (dda): ") * 100
  
  mean_autocor = extractNumber (report$, "Mean autocorrelation: ")
  mean_nhr = extractNumber (report$, "Mean noise-to-harmonics ratio: ")
  mean_hnr = extractNumber (report$, "Mean harmonics-to-noise ratio: ")
  
  
  # output reformatted
  
  writeInfoLine("Pitch median (Hz): ",  medianPitch)
  writeInfoLine("Pitch mean (Hz): ",    meanPitch)  
  writeInfoLine("Pitch sd (Hz): ",      sdPitch)
  writeInfoLine("Pitch min (Hz): ",     minPitch)
  writeInfoLine("Pitch max (Hz): ",     maxPitch)
  
  writeInfoLine("Pulses number: ",      nPulses)
  writeInfoLine("Periods number: ",     nPeriods)
  writeInfoLine("Period mean (sec): ",  meanPeriod)
  writeInfoLine("Period sd (sec): ",    sdPeriod)
  
  writeInfoLine("Fraction locally unvoiced frames (%): ", pctUnvoiced)
  writeInfoLine("Voice breaks number: ",                  nVoicebreaks)
  writeInfoLine("Voice breaks degree (%): ",              pctVoicebreaks)
  
  writeInfoLine("Jitter local (%): ",        jitter_loc)
  writeInfoLine("Jitter local abs (sec): ",  jitter_loc_abs)
  writeInfoLine("Jitter rap (%): ",          jitter_rap)
  writeInfoLine("Jitter ppq5 (%): ",         jitter_ppq5)
  writeInfoLine("Jitter ddp (%): ",          jitter_ddp)
  
  writeInfoLine("Shimmer local (%): ",       shimmer_loc)
  writeInfoLine("Shimmer local (dB): ",      shimmer_loc_dB)
  writeInfoLine("Shimmer apq3 (%): ",        shimmer_apq3)
  writeInfoLine("Shimmer apq5 (%): ",        shimmer_apq5)
  writeInfoLine("Shimmer apq11 (%): ",       shimmer_apq11)
  writeInfoLine("Shimmer dda (%): ",         shimmer_dda)
  
  writeInfoLine("Harmonicity mean autocor: ",                         mean_autocor)
  writeInfoLine("Harmonicity mean noise-to-harmonics ratio: ",        mean_nhr)
  writeInfoLine("Harmonicity mean harmonics-to-noise ratio (dB): ",   mean_hnr)
  
  
select 'soundid'


originaldur = Get total duration


Subtract mean


# Use intensity to get threshold
To Intensity... 50 0 yes

intid = selected("Intensity")

start = Get time from frame number... 1

nframes = Get number of frames
end = Get time from frame number... 'nframes'

# estimate noise floor
minint = Get minimum... 0 0 Parabolic


# estimate noise max
maxint = Get maximum... 0 0 Parabolic


#get median of Intensity: limits influence of high peaks
medint = Get quantile... 0 0 0.5


# estimate Intensity threshold
threshold = medint + iglevel
if threshold < minint
   threshold = minint
endif


Down to Matrix

matid = selected("Matrix")
# Convert intensity to sound
To Sound (slice)... 1

sndintid = selected("Sound")



intdur = Get finishing time
intmax = Get maximum... 0 0 Parabolic


# estimate peak positions (all peaks)
To PointProcess (extrema)... Left yes no Sinc70

ppid = selected("PointProcess")


numpeaks = Get number of points


# fill array with time points
for i from 1 to numpeaks
   t'i' = Get time from index... 'i'
endfor


# fill array with intensity values

select 'sndintid'

peakcount = 0
for i from 1 to numpeaks
   value = Get value at time... t'i' Cubic
   if value > threshold
         peakcount += 1
         int'peakcount' = value
         timepeaks'peakcount' = t'i'
   endif
endfor

# fill array with valid peaks: only intensity values if preceding 
# dip in intensity is greater than mindip

select 'intid'

validpeakcount = 0
precedingtime = timepeaks1
precedingint = int1
for p to peakcount-1
  following = p + 1
  followingtime = timepeaks'following'
  dip = Get minimum... 'precedingtime' 'followingtime' None
  diffint = abs(precedingint - dip)
  if diffint > mindip
     validpeakcount += 1
     validtime'validpeakcount' = timepeaks'p'
  endif
  precedingtime = timepeaks'following'
  precedingint = Get value at time... timepeaks'following' Cubic
endfor

# Look for only voiced parts

select 'soundid' 

To Pitch (ac)... 0.02 30 4 no 0.03 0.25 0.01 0.35 0.25 450
pitchid = selected("Pitch")



voicedcount = 0
for i from 1 to validpeakcount
  querytime = validtime'i'

  value = Get value at time... 'querytime' Hertz Linear


  if value <> undefined
     voicedcount = voicedcount + 1
     voicedpeak'voicedcount' = validtime'i'
  endif
endfor




# output data
writeInfoLine("Speechrate duration (sec): ", intdur)
writeInfoLine("Speechrate voiced count: ", voicedcount)
writeInfoLine("Speechrate (syllables/sec): ", voicedcount/intdur)


Remove