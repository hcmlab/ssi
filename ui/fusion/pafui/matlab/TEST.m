dim = 3;
num = 3;
param = [100.0 0.5];
speed = 0.1;
delta_t = 0.02;
thres = 0.01;

% init timer
pad_init ();

% create pad entities
for i = 1:num
    pads(i) = pad_create (1.0, param, 'exp', dim);
    pads(i) = pad_update (pads(i), 2*rand (dim,1)-1);
end

% create combined pad vector
pad_fused = zeros (dim, 1);

% run fusion
ax = gca;
pad_initplot (ax, dim);
disp ('press ctrl+c to stop..')
while 1
    pause (delta_t)    
    [pad_fused modvec pads] = pad_combine (pad_fused, speed, delta_t, thres, pads);
    pad_clearplot (ax);    
    pad_plot (ax, pad_fused, 'r', pads, 'b');
end