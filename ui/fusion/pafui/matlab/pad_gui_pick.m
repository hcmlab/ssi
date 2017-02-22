function varargout = pad_gui_pick(varargin)
% PAD_GUI_PICK M-file for pad_gui_pick.fig
%      PAD_GUI_PICK, by itself, creates a new PAD_GUI_PICK or raises the existing
%      singleton*.
%
%      H = PAD_GUI_PICK returns the handle to a new PAD_GUI_PICK or the handle to
%      the existing singleton*.
%
%      PAD_GUI_PICK('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in PAD_GUI_PICK.M with the given input arguments.
%
%      PAD_GUI_PICK('Property','Value',...) creates a new PAD_GUI_PICK or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before pad_gui_pick_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to pad_gui_pick_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help pad_gui_pick

% Last Modified by GUIDE v2.5 15-Dec-2008 15:41:17

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @pad_gui_pick_OpeningFcn, ...
                   'gui_OutputFcn',  @pad_gui_pick_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT


% --- Executes just before pad_gui_pick is made visible.
function pad_gui_pick_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to pad_gui_pick (see VARARGIN)

% Choose default command line output for pad_gui_pick
handles.output = hObject;

% closing function
set (handles.figure_main, 'CloseRequestFcn', @pad_gui_pick_CloseFcn);

% Update handles structure
handles.pads = [];
handles.dim = 2;
handles.emo = [];
handles.emo_weight = 0.5;
handles.isrunning = false;
handles.timer = 0;
handles.server = 0;
handles.thres = 0.1;
guidata(hObject, handles);

adjust_gui (handles);

% UIWAIT makes pad_gui_pick wait for user response (see UIRESUME)
% uiwait(handles.figure_main);


% --- Outputs from this function are returned to the command line.
function varargout = pad_gui_pick_OutputFcn (hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;

% --- Executes when attempt is made to close gui.
function pad_gui_pick_CloseFcn (hObject, eventdata)
%src is the handle of the object generating the callback (the source of the event)
%evnt is the The event data structure (can be empty for some callbacks)

disp ('attempt to close gui..')

selection = questdlg('Do you want to close the GUI?',...
    'Close Request Function',...
    'Yes','No','Yes');
switch selection,
    case 'Yes',
        handles = guidata (gcbo);
        if handles.isrunning
            try
                stop (handles.timer);
                delete (handles.timer);
                handles.timer = 0;
            catch
                warning ('could not stop timer');
            end
        end
        delete (gcf);
        disp ('gui closed')
    case 'No'
        disp ('aborted')
        return
end

function control_edit_num_Callback(hObject, eventdata, handles)
% hObject    handle to control_edit_num (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of control_edit_num as text
%        str2double(get(hObject,'String')) returns contents of control_edit_num as a double


% --- Executes during object creation, after setting all properties.
function control_edit_num_CreateFcn(hObject, eventdata, handles)
% hObject    handle to control_edit_num (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in control_button_start.
function control_button_start_Callback(hObject, eventdata, handles)
% hObject    handle to control_button_start (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

disp ('attempt to start simulation..')

if handles.isrunning
    warning ('already running')
    return
end

% get objects
dim = handles.dim;
pads = handles.pads;

% create pads
emo = zeros (dim, 1);
for i = 1:length (pads)
    pads(i).time = 0;
end
pad_init ();

% create timer
period = str2double (get (handles.control_edit_interval, 'String'));
emo_weight = str2double (get (handles.control_edit_cweight, 'String'));
t = timer ('TimerFcn', {@timer_callback, gcbo}, 'Period', period, 'ExecutionMode', 'fixedRate');

% create server
host = get (handles.server_edit_host, 'String');
port = str2num (get (handles.server_edit_port, 'String'));
s = 0;
try
    s = osc_new_address (host, port);
catch
    warning ('could not start server');
end

% Update handles structure
handles.isrunning = true;
handles.timer = t;
handles.timercount = 0;
handles.emo = emo;
handles.emo_weight = emo_weight;
handles.pads = pads;
handles.server = s;
guidata(hObject, handles);

% init plot
pad_initplot (handles.pad_axes, dim);

% start timer
start (t);

adjust_gui (handles);
disp ('simulation started')


% --- Executes on button press in control_button_stop.
function control_button_stop_Callback(hObject, eventdata, handles)
% hObject    handle to control_button_stop (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

disp ('attempt to stop simulation..')

if ~handles.isrunning
    warning ('not running')
    return
end

stop (handles.timer);
delete (handles.timer);
try
    osc_free_address (handles.server);
catch
    warning ('could not stop server');
end
handles.isrunning = false;

% Update handles structure
guidata(hObject, handles);

adjust_gui (handles);
disp ('simulation stopped')


% --- Executes on selection change in control_menu_select.
function control_menu_select_Callback(hObject, eventdata, handles)
% hObject    handle to control_menu_select (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns control_menu_select contents as cell array
%        contents{get(hObject,'Value')} returns selected item from control_menu_select


% --- Executes during object creation, after setting all properties.
function control_menu_select_CreateFcn(hObject, eventdata, handles)
% hObject    handle to control_menu_select (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function control_edit_value_Callback(hObject, eventdata, handles)
% hObject    handle to control_edit_value (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of control_edit_value as text
%        str2double(get(hObject,'String')) returns contents of control_edit_value as a double

disp ('attempt to change value')

pads = handles.pads;
if isempty (pads)
    disp ('empty pad list')
    return
end

index = get (handles.control_listbox_pad, 'Value');
value = str2num (get (handles.control_edit_value, 'String'));
pads = pad_update (pads, index, value);

% udpate gui data
handles.pads = pads;
guidata(hObject, handles);

disp ('value changed')


% --- Executes during object creation, after setting all properties.
function control_edit_value_CreateFcn(hObject, eventdata, handles)
% hObject    handle to control_edit_value (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in control_button_update.
function control_button_update_Callback(hObject, eventdata, handles)
% hObject    handle to control_button_update (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)




% --- Callback timer function to update pad model.
function timer_callback (obj, event, arg)

%disp ('attempt to update pad model..')

% get gui data
handles = guidata (arg);
emo = handles.emo;
emo_weight = handles.emo_weight;
pads = handles.pads;
dim = handles.dim;
thres = handles.thres;
period = get (obj, 'Period');
ax = handles.pad_axes;

if isempty (pads) | length (pads) == 0
    disp ('empty pad model')
    return
end

% update data
[emo modvec pads] = pad_combine (emo, emo_weight, period, thres, pads);

% update plot
update_pad_plot (ax, emo, modvec, pads, thres);
update_pad_list (handles, pads);

% Update handles structure
handles.emo = emo;
handles.pads = pads;
guidata(arg, handles);

%disp ('updated pad model')


% --- Update pad plot
function update_pad_plot (ax, emo, modvec, pads, thres)

pad_clearplot (ax);
pad_plot (ax, emo, 'r', pads, 'b');
plot (ax, [emo(1)-modvec(1) emo(1)], [emo(2)-modvec(2) emo(2)], 'LineWidth', 3, 'Color', 'g');
circle (ax, [0 0], thres, 200, '--k');


function control_edit_interval_Callback(hObject, eventdata, handles)
% hObject    handle to control_edit_interval (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of control_edit_interval as text
%        str2double(get(hObject,'String')) returns contents of control_edit_interval as a double


% --- Executes during object creation, after setting all properties.
function control_edit_interval_CreateFcn(hObject, eventdata, handles)
% hObject    handle to control_edit_interval (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end




% --- Executes on button press in control_button_add.
function control_button_add_Callback(hObject, eventdata, handles)
% hObject    handle to control_button_add (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

disp ('attempt to add pad')

% ask for user input
prompt = {'weight:','type:','param:'};
dlg_title = 'Input new pad value';
num_lines = 1;
def = {'1.0', 'hyp', '10.0 0.5'};
answer = inputdlg (prompt, dlg_title, num_lines, def);
if isempty (answer)
    disp ('canceled')
    return
end

weight = str2num (answer{1});
type = answer{2};
param = str2num (answer{3});

% add to pads
pads = handles.pads;
dim = handles.dim;
pad = pad_create (weight, param, type, dim);
pad = pad_update (pad, zeros (handles.dim, 1));
if isempty (pads)
    pads = pad;
else
    pads(length (pads) + 1) = pad;
end

% add to list
update_pad_list (handles, pads);
set (handles.control_listbox_pad, 'Value', length (pads));

% udpate gui data
handles.timercount = 0;
handles.pads = pads;
guidata(hObject, handles);

disp ('pad added')




% --- Executes on button press in control_button_remove.
function control_button_remove_Callback(hObject, eventdata, handles)
% hObject    handle to control_button_remove (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

disp ('attempt to remove pad')

pads = handles.pads;
if isempty (pads)
    disp ('empty pad list')
	return
end

vars = get (handles.control_listbox_pad, 'String');
index = get (handles.control_listbox_pad, 'Value');
vars(index) = [];
set (handles.control_listbox_pad, 'Value', 1);
set (handles.control_listbox_pad, 'String', vars);
pads(index) = [];

% udpate gui data
handles.pads = pads;
guidata(hObject, handles);

disp ('removed pad')


% --- Executes on selection change in control_listbox_pad.
function control_listbox_pad_Callback(hObject, eventdata, handles)
% hObject    handle to control_listbox_pad (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns control_listbox_pad contents as cell array
%        contents{get(hObject,'Value')} returns selected item from control_listbox_pad

index = get (hObject, 'Value');
pad = handles.pads(index);


% --- Executes during object creation, after setting all properties.
function control_listbox_pad_CreateFcn(hObject, eventdata, handles)
% hObject    handle to control_listbox_pad (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: listbox controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function control_edit_lambda_Callback(hObject, eventdata, handles)
% hObject    handle to control_edit_lambda (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of control_edit_lambda as text
%        str2double(get(hObject,'String')) returns contents of control_edit_lambda as a double

disp ('attempt to change lambda')

pads = handles.pads;
if isempty (pads)
    disp ('empty pad list')
    return
end

index = get (handles.control_listbox_pad, 'Value');
value = str2num (get (handles.control_edit_lambda, 'String'));
pads(index).lambda = value;

% udpate gui data
handles.pads = pads;
guidata(hObject, handles);

disp ('lambda changed')





% --- Executes during object creation, after setting all properties.
function control_edit_lambda_CreateFcn(hObject, eventdata, handles)
% hObject    handle to control_edit_lambda (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end





function control_edit_cweight_Callback(hObject, eventdata, handles)
% hObject    handle to control_edit_cweight (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of control_edit_cweight as text
%        str2double(get(hObject,'String')) returns contents of control_edit_cweight as a double


% --- Executes during object creation, after setting all properties.
function control_edit_cweight_CreateFcn(hObject, eventdata, handles)
% hObject    handle to control_edit_cweight (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function control_edit_clambda_Callback(hObject, eventdata, handles)
% hObject    handle to control_edit_clambda (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of control_edit_clambda as text
%        str2double(get(hObject,'String')) returns contents of control_edit_clambda as a double


% --- Executes during object creation, after setting all properties.
function control_edit_clambda_CreateFcn(hObject, eventdata, handles)
% hObject    handle to control_edit_clambda (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- update pad list
function update_pad_list (handles, pads)

len = length (pads);
str = [num2str((1:len)') repmat('  ',len,1)  num2str([pads.value]', '% .2f  ') num2str([pads.value_decay]', '% .2f  ')];
set (handles.control_listbox_pad, 'String', cellstr (str));




% --- Adjust gui.
function adjust_gui (handles)

if (handles.isrunning)
    set (handles.control_button_stop, 'Enable', 'on');
    set (handles.control_button_start, 'Enable', 'off');
    set (handles.control_edit_cweight, 'Enable', 'off');
    set (handles.control_edit_clambda, 'Enable', 'off');
    set (handles.control_edit_interval, 'Enable', 'off');
    set (handles.control_button_add, 'Enable', 'on');
    set (handles.control_button_remove, 'Enable', 'on');        
else
    set (handles.control_button_stop, 'Enable', 'off');
    set (handles.control_button_start, 'Enable', 'on');
    set (handles.control_edit_cweight, 'Enable', 'on');
    set (handles.control_edit_clambda, 'Enable', 'on');
    set (handles.control_edit_interval, 'Enable', 'on');
    set (handles.control_button_add, 'Enable', 'on');
    set (handles.control_button_remove, 'Enable', 'on');               
end



% --- Executes on button press in control_button_pick.
function control_button_pick_Callback(hObject, eventdata, handles)
% hObject    handle to control_button_pick (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

disp ('attempt to pick value')

index = get (handles.control_listbox_pad, 'Value');
if index < 1
    disp ('select pad first')
    return;
end

% pick value
[x,y] = ginput (1);

% update value
pads = handles.pads;
if length (pads) < 1
    disp ('empty pad list')
    return;
end
pads(index) = pad_update (pads(index), [x y]);

% send value
if handles.server ~= 0
    osc_send_event (handles.server, ['id_' num2str(index)], toc, 0.0, {'p', 'a'}, {x, y});
end

% udpate gui data
handles.pads = pads;
guidata(hObject, handles);



% --- Executes on button press in axes_button_save.
function axes_button_save_Callback (hObject, eventdata, handles)
% hObject    handle to axes_button_save (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% save axes to to file
timercount = handles.timercount;
ax = handles.pad_axes;
str = num2str (timercount);
if timercount < 10
    filename = ['pad_0' str];
else
    filename = ['pad_' str];    
end
saveax (ax, filename, 'jpeg');



function server_edit_port_Callback(hObject, eventdata, handles)
% hObject    handle to server_edit_port (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of server_edit_port as text
%        str2double(get(hObject,'String')) returns contents of server_edit_port as a double


% --- Executes during object creation, after setting all properties.
function server_edit_port_CreateFcn(hObject, eventdata, handles)
% hObject    handle to server_edit_port (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function server_edit_host_Callback(hObject, eventdata, handles)
% hObject    handle to server_edit_host (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of server_edit_host as text
%        str2double(get(hObject,'String')) returns contents of server_edit_host as a double


% --- Executes during object creation, after setting all properties.
function server_edit_host_CreateFcn(hObject, eventdata, handles)
% hObject    handle to server_edit_host (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


