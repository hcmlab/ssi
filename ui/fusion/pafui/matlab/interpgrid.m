function [xi,yi,zi] = interpgrid (data, corner, space, resolution, method)
%
% interpolates between given points in a grid
%
% [xi,yi,zi] = interpgrid (data, corner, space, delta, method)
%
% input:
%   data                    given data points in form [x1 y1 z1; x2 y2 z2; ...]
%   corner                  z values quadrant corners in form [I,II,III,IV]
%                           (default: [0 0 0 0])
%   space                   defines space in form [minx maxx miny maxy]
%                           (default: [-1 1 -1 1])
%   resolution              width and height of mesh in form [w h]
%                           (default: [50 50])
%   method                  interpolation method: 'gridfit', 'linear',
%                                                 'cubic', 'v4'
%                           (default: 'gridfit')
%
% output:
%   xi                      matrix with x-values of the grid
%   yi                      matrix with y-values of the grid
%   zi                      matrix with z-values of the grid
%
% by Johannes Wagner <johannes.wagner@informatik.uni-augsburg.de>
% created: 2009/01/09

if nargin < 1
    help interpgrid
    error ('not enough arguments')
end
if nargin < 2
    corner = [0 0 0 0];
end
if nargin < 3 | isempty (space)
    space = [-1 1 -1 1];
end
if nargin < 4 | isempty (resolution)
    resolution = [50 50];
end
if nargin < 5 | isempty (method)
    method = 'gridfit';
end

% create grid
minx = space(1);
maxx = space(2);
miny = space(3);
maxy = space(4);
width = resolution(1);
height = resolution(2);
[xi,yi] = meshgrid(minx:(maxx-minx)/(width-1):maxx,miny:(maxy-miny)/(height-1):maxy);

% add values to corner of grid
if ~isempty (corner)
    A = data(:,1:2);
    if not (hasrow (A, [maxx maxy]))
        data = [data; maxx maxy corner(1)];
    end
    if not (hasrow (A, [minx maxy]))
        data = [data; minx maxy corner(2)];
    end
    if not (hasrow (A, [minx miny]))
        data = [data; minx miny corner(3)];
    end
    if not (hasrow (A, [maxx miny]))
        data = [data; maxx miny corner(4)];
    end
end
    
% apply interpolation
x = data(:,1);
y = data(:,2);
z = data(:,3);
if strcmp (method, 'gridfit') 
    zi = gridfit(x,y,z,minx:delta:maxx,miny:delta:maxy);
else
	zi = griddata(x,y,z,xi,yi,method);
end

% plot
if nargout == 0
    meshc (xi,yi,zi);
end



% returns true if r is a row of A
function answer = hasrow (A, r)

[len dim] = size (A);
check = sum (A == repmat(r,len,1), 2);
answer = not (isempty (find (check == dim)));


