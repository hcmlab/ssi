function pad_clearplot (ax)
% Clears pad plot.
%
%  pad_clearplot (ax)
%   
%   input:
%   ax            axis handle
%
% Johannes Wagner <johannes.wagner@informatik.uni-augsburg.de>, 2008

if nargin < 1
    help pad_clearplot
    error ('not enough arguments');
end

cla (ax);
plot3 (ax, [-1 1], [0 0], [0 0], 'Color', 'k', 'LineWidth', 2);
plot3 (ax, [0 0], [-1 1], [0 0], 'Color', 'k', 'LineWidth', 2);
plot3 (ax, [0 0], [0 0], [-1 1], 'Color', 'k', 'LineWidth', 2);    
