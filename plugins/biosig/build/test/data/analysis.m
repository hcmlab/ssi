clc;
clear all;

% load ssi streams
[gsr, sr] = ssi_signal_read('gsr.stream');
[gsr_lowpass, xxx] = ssi_signal_read('gsr_lowpass.stream');
[gsr_detrend, xxx] = ssi_signal_read('gsr_detrend.stream');
[gsr_detrend_norm, xxx] = ssi_signal_read('gsr_detrend_norm.stream');

% plot
upper = sr*1;
lower = sr*180;

subplot 411
ssi_signal_plot (gsr(upper:lower), sr);
  
subplot 412
ssi_signal_plot (gsr_lowpass(upper:lower), sr);

subplot 413
ssi_signal_plot (gsr_detrend(upper:lower), sr);

subplot 414
ssi_signal_plot (gsr_detrend_norm(upper:lower), sr);