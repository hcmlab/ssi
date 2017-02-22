% creating a server -- just specify what port to listen on
s = osc_new_server (1111);

try
    [id, time, dur, events] = osc_recv_event (s, 1.0)
catch
    err = lasterror
    warning ('receive failed')
end

osc_free_server (s);



