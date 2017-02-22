function [pad_fused modvec pads] = pad_combine (pad_fused, speed, delta_t, thres, pads)
% Updates the combined pad vector according to current pad entities.
% This is done in the following way:
%   1. pad entities are updated according to their decay function
%   2. weighted sum of pad entities is calculated = mass center
%   3. the combined pad vector is moved in direction of mass center
%
%  [pad_fused modvec pads] = pad_combine (pad_fused, speed, delta_t, thres, pads)
%   
%   input:
%   pad_fused       the combined pad vector
%   speed           distance the fused vector moves in directio of mass center
%   delta_t         elapsed time since last call
%   thres           entities with a norm below this threshold will be not
%                   taken in account
%   pads            the pad entities
%
%   output:
%   pad_fused       the fused pad vector
%   modvec          the modification vector
%   pads            the updated pad entities
%
% Johannes Wagner <johannes.wagner@informatik.uni-augsburg.de>, 2008

if nargin < 5
    help pad_combine
    error ('not enough arguments')
end

dim = length (pad_fused);
num = length (pads);

% decay values
time = pad_time ();
for i = 1:num
    pads(i) = pad_decay (pads(i), time);
end

values = [pads.value_decay];
norms = vecnorm (values);
weights = [pads.weight];

% find contributing values
inds = logical (zeros (num, 1));
for i = 1:num
    if norms(i) > thres
        inds(i) = 1;
    end
end

% how far did me move since last call
delta = delta_t * speed;

if sum (inds) == 0
    % if no contributing value walk in direction of origin
    modvec = -pad_fused;   
else  
    % determine mass center
    mc = sum (values(:,inds) .* weights(:,inds), 2) ./ sum (weights(:,inds), 2);   
    % determine modification vector
    modvec = mc - pad_fused;
end

% % apply modification modification vector
modvec_norm = vecnorm (modvec);
% if modvec_norm < thres
%     pad_fused = zeros (dim, 1);
%     modvec = zeros (dim, 1);
% else            
if modvec_norm > 0
    delta = min (delta, modvec_norm);
    modvec = modvec * (delta / modvec_norm);
    pad_fused = pad_fused + modvec;
else
    pad_fused = zeros (dim, 1);
    modvec = zeros (dim, 1);
end



% returns quadrant
function q = getq (value)

q = zeros (4, 1);

if value(1) >= 0.1 & value(2) >= 0.1
    q(1) = 1;
end
if value(1) <= 0.1 & value(2) >= 0.1
    q(2) = 1;
end
if value(1) <= 0.1 & value(2) <= 0.1
    q(3) = 1;
end
if value(1) >= 0.1 & value(2) <= 0.1
    q(4) = 1;
end