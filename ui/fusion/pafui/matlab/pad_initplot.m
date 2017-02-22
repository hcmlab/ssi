function pad_initplot (ax, dim)
% Inits pad plot.
%
%  pad = pad_decay (pad, time)
%   
%   input:
%   ax          axis handle
%   dim         dimension
%
% Johannes Wagner <johannes.wagner@informatik.uni-augsburg.de>, 2008

if nargin < 2
    help pad_initplot
    error ('not enough arguments')
end

hold (ax, 'on');
grid (ax, 'on');
% box (ax, 'on');
switch dim
    case 2
        axis (ax, [-1.0 1.0 -1.0 1.0]);
        xlabel ('Pleasure');
        ylabel ('Arousal');
    case 3
        axis (ax, [-1.0 1.0 -1.0 1.0 -1.0 1.0 -1.0 1.0]);
        set (ax, 'View', [50, 50]);
        xlabel ('Pleasure');
        ylabel ('Arousal');
        zlabel ('Dominance');
    otherwise
        error ('dimension not supported')
end

pad_clearplot (ax);