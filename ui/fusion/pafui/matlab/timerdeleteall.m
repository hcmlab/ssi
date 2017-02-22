function timerdeleteall ()
% Delets all timer.
%
%  timerdeleteall ()
%
% Johannes Wagner <johannes.wagner@informatik.uni-augsburg.de>, 2008

ts = timerfindall ();
for i = 1:length (ts)
    delete (ts(i));
end