function y = vecnorm (x)
% Return norm of a vector.
%
%  pad = pad_update (pad, value)
%   
%   input:
%   x           vector
%
%   output:
%   y           vector norm
%
% Johannes Wagner <johannes.wagner@informatik.uni-augsburg.de>, 2008

if nargin < 1
    help vecnorm
    error ('not enough arguments')
end

y = sqrt (sum (x.^2, 1));