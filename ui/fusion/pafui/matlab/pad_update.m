function pad = pad_update (pad, value)
% Updates pad entity.
%
%  pad = pad_update (pad, value)
%   
%   input:
%   pad             pad entity
%   value           new value
%
%   output:
%   pad             updated pad entity
%
% Johannes Wagner <johannes.wagner@informatik.uni-augsburg.de>, 2008

if nargin < 2
    help pad_update
    error ('not enough arguments')
end

pad.value_decay = value(:);
pad.value = value(:);
pad.time = pad_time ();