function [success, id, time, dur, events] = osc_recv_event (server, timeout)

id = '';
time = 0;
dur = 0;
events = {};

msg = osc_recv (server, timeout);
success = false;

if length (msg) > 0
    if strcmp (msg{1}.path, '/evnt')
        m = msg{1}.data;
        id = m{1};
        time = m{2};
        dur = m{3};
        num = m{4};
        events = cell (num, 2);
        for i = 1:num
            events{i,1} = m{3+i*2};
            events{i,2} = m{4+i*2};            
        end
        success = true;
    end
end