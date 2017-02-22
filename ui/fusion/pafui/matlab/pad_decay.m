function pad = pad_decay (pad, time)
% Updates pad entity.
%
%  pad = pad_decay (pad, time)
%   
%   input:
%   pad          	pad entity
%   time            timestamp
%
%   output:
%   pad             updatet pad entity
%
% Johannes Wagner <johannes.wagner@informatik.uni-augsburg.de>, 2008

if nargin < 2
    help pad_decay
    error ('not enough arguments')
end

% calcualte elapsed time since last update
dt = time - pad.time;

% calculate norm
n = vecnorm (pad.value);

% calculate decay factor
if n > 0
    f = decay (n, dt, 0, pad.param, pad.type) / n;
else
    f = 0;
end

% apply decay factor
pad.value_decay = pad.value .* f;
