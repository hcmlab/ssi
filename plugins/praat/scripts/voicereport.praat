form Voice report
text soundFileName
endform
sound = Read from file... 'soundFileName$'
pitch = To Pitch (cc)... 0.01 50 15 no 0.03 0.45 0.01 0.35 0.14 300
plus sound
pulses = To PointProcess (cc)
plus sound
plus pitch
voiceReport$ = Voice report... 0 0 50 300 1.3 1.6 0.03 0.45
writeInfoLine (voiceReport$)