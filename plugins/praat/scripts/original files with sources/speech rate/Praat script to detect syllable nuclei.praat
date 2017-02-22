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
   sentence directory C:/directorywithsoundfiles
endform

 

# shorten variables
iglevel = 'ignorance_Level/Intensity_Median'
mindip = 'minimum_dip_between_peaks'

 

Create Strings as file list... list 'directory$'/*.wav
numberOfFiles = Get number of strings
for ifile to numberOfFiles
   select Strings list
   fileName$ = Get string... ifile
   Read from file... 'directory$'/'fileName$'

   obj$ = selected$("Sound")

   soundid = selected("Sound")

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


   
   # calculate time correction due to shift in time for Sound object versus
   # intensity object
   timecorrection = originaldur/intdur


   # Insert voiced peaks in second Tier

   select 'soundid' 

   To TextGrid... "syllables" syllables
   textgridid = selected("TextGrid")
   for i from 1 to voicedcount
       position = voicedpeak'i' * timecorrection
       Insert point... 1 position 'i'
   endfor


   # write textgrid to textfile
   Write to text file... 'directory$'/'obj$'.syllables.TextGrid


   # clean up before next sound file is opened

     select 'intid'

     plus 'matid'

     plus 'ppid'

     plus 'sndintid'

     plus 'soundid'

     plus 'textgridid'

     plus 'pitchid'

   Remove


endfor