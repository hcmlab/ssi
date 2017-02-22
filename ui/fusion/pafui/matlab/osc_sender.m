% creating a server -- just specify what port to listen on
s = osc_new_address ('127.0.0.1', 1111);

m = struct('path', '/foobar', 'data', {{logical(0), int32(1), 3.14159, logical(1), 'hello world'}});
if (osc_send (s, m) == 0)
    disp ('warning osc_send() failed');
end

osc_free_address (s);