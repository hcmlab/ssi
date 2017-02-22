function [success] = osc_send_event (server, id, time, dur, event_names, event_values)

num = length (event_names);

data = cell (4 + num, 1);
data{1} = id;
data{2} = single (time);
data{3} = single (dur);
data{4} = int32 (num);
for i = 1:num
    data{4+2*i-1} = event_names{i};
    data{4+2*i} = single (event_values{i});
end
msg = struct ('path', '/evnt', 'data', {data});

success = osc_send (server, msg);
