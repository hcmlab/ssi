function y = decay (x, dt, ground, param, type)
% Decays x in direction of ground depending on time elapsed since last
% update. three difference decay functions are available, namely linear,
% expotentional or hyperbolic.
%
% y = decay (x, dt, ground, dur, type)
%
% input:
%   x                       value since last update
%   dt                      time elapsed since last update
%   ground                  value to which function converges
%   param                   time in s required to converge abs(x-ground)=1 to ground
%                           (optionally a second value can be given which
%                           influences the gradient in case of expotential
%                           or hyperbolic decay function, default: 0.5)
%   type                    'lin' for linear decay function
%                           'exp' for expotential decay function
%                           'hyp' for hyperbolic decay function
%
% output:
%   y                       output of decay function
%
%
% Johannes Wagner <johannes.wagner@informatik.uni-augsburg.de>, 2008

if nargin < 5
    help decay;
    error ('not enough arguments');
end

dur = param(1);
fac = abs (x-ground);
if fac == 0
    lambda = 0;
else
    if length (param) > 1
        lambda = (10*param(2))/(fac*dur);
    else
        lambda = 5/(fac*dur);
    end
end

% if dt >= dur
%     y = ground;
%     return;
% end

switch type
    
    case 'lin'
        
        if x > ground
            tmp = x - dt * (1/dur);
            if tmp < ground, y = ground; else y = tmp; end        
        else
            tmp = x + dt * (1/dur);
            if tmp > ground, y = ground; else y = tmp; end        
        end
        
    case 'exp'
        
        y = ground + (x - ground) * (exp(1).^(-lambda * dt));
        
    case 'hyp'
        
        dur = 0.5 * fac * dur;
        y = ground + ((x-ground)/2) * (1 - tanh (lambda * (dt-dur)));
        
    otherwise
        error ('unknown decay function')
end


