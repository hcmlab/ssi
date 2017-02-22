for %%f in (*.wav) do (
  sox %%f -n spectrogram -o %%~nf.png
)

