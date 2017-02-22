function pad_plot (ax, pad_fused, pad_fused_color, pads, pad_color)
% Update pad plot.
%
%  pad_plot (ax, pad_fused, pad_fused_color, pads, pad_color)
%   
%   input:
%   ax                axis handle
%   pad_fused         the combined vector
%   pad_fused_color   color of combined vector
%   pads              vector with pad entities
%   pad_color         color of pad entities
%
% Johannes Wagner <johannes.wagner@informatik.uni-augsburg.de>, 2008

if nargin < 5
    help pad_plot
    error ('not enough arguments')
end

if isempty (pads)
    return
end

dim = length (pads(1).value);

% axes (ax);
num = length (pads);
for i = 1:num
    pad = pads(i);
    value = pad.value_decay;
    switch dim
        case 2
            plot (ax, [0 value(1)],[0 value(2)], 'LineWidth', 3, 'Color',  pad_color)
        case 3
            plot3 (ax, [0 value(1)], [0 value(2)], [0 value(3)], 'LineWidth', 3, 'Color', pad_color);
            plot3 (ax, [0 value(1)], [value(2) value(2)], [0 0], 'Color', pad_color, 'LineStyle', '--');
            plot3 (ax, [value(1) value(1)], [0 value(2)], [0 0], 'Color', pad_color, 'LineStyle', '--');
            plot3 (ax, [value(1) value(1)], [value(2) value(2)], [0 value(3)], 'Color', pad_color, 'LineStyle', '--');               
            plot3 (ax, [value(1) value(1)], [value(2) value(2)], [0 value(3)], 'Color', pad_color, 'LineStyle', '--');            
        otherwise
            error ('dimension not supported')
    end
end
switch dim
    case 2
        plot (ax, [0 pad_fused(1)], [0 pad_fused(2)], 'LineWidth', 3, 'Color', pad_fused_color);
    case 3
        plot3 (ax, [0 pad_fused(1)], [0 pad_fused(2)], [0 pad_fused(3)], 'LineWidth', 3, 'Color', pad_fused_color);
        plot3 (ax, [0 value(1)], [value(2) value(2)], [0 0], 'Color', pad_fused_color, 'LineStyle', '--');
        plot3 (ax, [value(1) value(1)], [0 value(2)], [0 0], 'Color', pad_fused_color, 'LineStyle', '--');
        plot3 (ax, [value(1) value(1)], [value(2) value(2)], [0 value(3)], 'Color', pad_fused_color, 'LineStyle', '--');
        plot3 (ax, [value(1) value(1)], [value(2) value(2)], [0 value(3)], 'Color', pad_fused_color, 'LineStyle', '--');
    otherwise
        error ('dimension not supported')
end

