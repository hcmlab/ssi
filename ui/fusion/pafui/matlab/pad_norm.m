function [nvalue, param] = pad_norm (value, param)

value = value(:)';

mavgcd_nshort = param.n_short;
mavgcd_nlong = param.n_long;
mavgcd_hist = param.mavgcd_hist;
mvgnorm_hist = param.mvgnorm_hist;
mvgnorm_n = param.mvgnorm_n;
mvgnorm_k = param.mvgnorm_k;

[nvalue, mavgcd_hist] = mavgcd (value, @expmavg, mavgcd_nshort, mavgcd_nlong, mavgcd_hist);
[minval, maxval, mvgnorm_hist] = mvgminmax (nvalue, mvgnorm_n, mvgnorm_k, mvgnorm_hist);

range = maxval - minval;
if range > 0
    nvalue = 2.0 .* (nvalue - minval) ./ range - 1.0;
end

param.mavgcd_hist = mavgcd_hist;
param.mvgnorm_hist = mvgnorm_hist;