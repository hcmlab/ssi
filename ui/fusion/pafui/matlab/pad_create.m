function pad = pad_create (weight, param, type, dim)
% Creates a new pad entity.
%
%  pad = pad_create (weight, param, type, dim)
%   
%   input:
%   weight          sum weight
%   param           time in s required to converge a gain of 1 to 0
%                   (optionally a second value can be given which
%                   influences the gradient in case of expotential
%                   or hyperbolic decay function, default: 0.5)
%   type            'lin' for linear decay function
%                   'exp' for expotential decay function
%                   'hyp' for hyperbolic decay function    
%   dim             dimension of input (usually 3)
%                
%   output:
%   pad             pad entity
%
% Johannes Wagner <johannes.wagner@informatik.uni-augsburg.de>, 2008

if nargin < 4
    help pad_create
    error ('not enough arguments')
end

% create neutral pads
pad.value = zeros (dim, 1);
pad.value_decay = zeros (dim, 1);
pad.weight = repmat (weight, dim, 1);
pad.param = param;
pad.type = type;
pad.time = 0;
