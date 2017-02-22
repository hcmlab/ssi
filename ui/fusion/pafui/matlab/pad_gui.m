function varargout = pad_gui(varargin)
% PAD_GUI M-file for pad_gui.fig
%      PAD_GUI, by itself, creates a new PAD_GUI or raises the existing
%      singleton*.
%
%      H = PAD_GUI returns the handle to a new PAD_GUI or the handle to
%      the existing singleton*.
%
%      PAD_GUI('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in PAD_GUI.M with the given input arguments.
%
%      PAD_GUI('Property','Value',...) creates a new PAD_GUI or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before pad_gui_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to pad_gui_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help pad_gui

% Last Modified by GUIDE v2.5 28-Nov-2008 17:28:23

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @pad_gui_OpeningFcn, ...
                   'gui_OutputFcn',  @pad_gui_OutputFcn, ...
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


% --- Executes just before pad_gui is made visible.
function pad_gui_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to pad_gui (see VARARGIN)

% Choose default command line output for pad_gui
handles.output = hObject;

% closing function
set (handles.figure_main, 'CloseRequestFcn', @pad_gui_CloseFcn);

% Update handles structure
handles.pads = [];
handles.padc = [];
handles.dim = 2;
handles.isrunning = false;
handles.timer = 0;
guidata(hObject, handles);

adjust_gui (handles);

% UIWAIT makes pad_gui wait for user response (see UIRESUME)
% uiwait(handles.figure_main);


% --- Outputs from this function are returned to the command line.
function varargout = pad_gui_OutputFcn (hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;

% --- Executes when attempt is made to close gui.
function pad_gui_CloseFcn (hObject, eventdata)
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

% create pads
pad_init ();
weight = str2num (get (handles.control_edit_cweight, 'String'));
lambda = str2num (get (handles.control_edit_clambda, 'String'));
padc = pad_create (weight, lambda, handles.dim);
padc = pad_update (padc, 1, zeros (handles.dim, 1));
pads = handles.pads;
for i = 1:length (pads)
    pads(i).time = toc;
end

% create timer
period = str2double (get (handles.control_edit_interval, 'String'));
t = timer ('TimerFcn', {@timer_callback, gcbo}, 'Period', period, 'ExecutionMode', 'fixedRate');

% Update handles structure
handles.isrunning = true;
handles.timer = t;
handles.padc = padc;
handles.pads = pads;
guidata(hObject, handles);

% init plot
pad_initplot (handles.pad_axes, handles.dim);

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

disp ('attempt to update pad model..')

% get gui data
handles = guidata (arg);
padc = handles.padc;
pads = handles.pads;

if isempty (pads) | length (pads) == 0
    disp ('empty pad model')
    return
end

% update data
pads = pad_decay (pads);
padc = pad_combine (padc, pads);

% update plot
update_pad_plot (handles.pad_axes, padc, pads);

% Update handles structure
handles.padc = padc;
handles.pads = pads;
guidata(arg, handles);

disp ('updated pad model')


% --- Update pad plot
function update_pad_plot (ax, padc, pads)

pad_clearplot (ax);
pad_plot (ax, pads, 'b');
pad_plot (ax, padc, 'r');



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
prompt = {'name:','value:','weight:','lambda:'};
dlg_title = 'Input new pad value';
num_lines = 1;
rand_value = num2str (-1 + 2 .* rand (1,handles.dim));
rand_weight = num2str (rand ());
rand_lambda = num2str (rand ());
def = {'name', rand_value, rand_weight, rand_lambda};
answer = inputdlg (prompt, dlg_title, num_lines, def);
if isempty (answer)
    disp ('canceled')
    return
end

name = answer{1};
value = str2num (answer{2});
weight = str2num (answer{3});
lambda = str2num (answer{4});

% add to pads
pads = handles.pads;
pad = pad_create (weight, lambda, handles.dim);
pad = pad_update (pad, 1, value);
if isempty (pads)
    pads = pad;
else
    pads(length (pads) + 1) = pad;
end

% add to list
vars = get (handles.control_listbox_pad, 'String');
vars{length (vars) + 1} = name;
set (handles.control_listbox_pad, 'String', vars);
select_pad (handles, pads(length (vars)));
set (handles.control_listbox_pad, 'Value', length (vars));

% udpate gui data
handles.pads = pads;
guidata(hObject, handles);

disp ('pad added')



% --- Set selected pad
function select_pad (handles, pad)

set (handles.control_edit_value, 'String', num2str (pad.value'));
set (handles.control_edit_weight, 'String', num2str (pad.weight(1)));
set (handles.control_edit_lambda, 'String', num2str (pad.lambda));



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
if length (pads) > 0
    select_pad (handles, pads(1));
end
set (handles.control_listbox_pad, 'Value', 1);
set (handles.control_listbox_pad, 'String', vars);

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
select_pad (handles, pad);

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



function control_edit_weight_Callback(hObject, eventdata, handles)
% hObject    handle to control_edit_weight (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of control_edit_weight as text
%        str2double(get(hObject,'String')) returns contents of control_edit_weight as a double


disp ('attempt to change weight')

pads = handles.pads;
if isempty (pads)
    disp ('empty pad list')
    return
end

index = get (handles.control_listbox_pad, 'Value');
value = str2num (get (handles.control_edit_weight, 'String'));
pads(index).weight = value;

% udpate gui data
handles.pads = pads;
guidata(hObject, handles);

disp ('weight changed')




% --- Executes during object creation, after setting all properties.
function control_edit_weight_CreateFcn(hObject, eventdata, handles)
% hObject    handle to control_edit_weight (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
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
    set (handles.control_edit_weight, 'Enable', 'on');
    set (handles.control_edit_lambda, 'Enable', 'on');    
    set (handles.control_edit_value, 'Enable', 'on');        
else
    set (handles.control_button_stop, 'Enable', 'off');
    set (handles.control_button_start, 'Enable', 'on');
    set (handles.control_edit_cweight, 'Enable', 'on');
    set (handles.control_edit_clambda, 'Enable', 'on');
    set (handles.control_edit_interval, 'Enable', 'on');
    set (handles.control_button_add, 'Enable', 'off');
    set (handles.control_button_remove, 'Enable', 'off');    
    set (handles.control_edit_weight, 'Enable', 'off');
    set (handles.control_edit_lambda, 'Enable', 'off');    
    set (handles.control_edit_value, 'Enable', 'off');            
end









% --- Executes on key press over control_edit_lambda with no controls selected.
function control_edit_lambda_KeyPressFcn(hObject, eventdata, handles)
% hObject    handle to control_edit_lambda (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)




% --- Executes on key press over control_edit_value with no controls selected.
function control_edit_value_KeyPressFcn(hObject, eventdata, handles)
% hObject    handle to control_edit_value (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on key press over control_edit_weight with no controls selected.
function control_edit_weight_KeyPressFcn(hObject, eventdata, handles)
% hObject    handle to control_edit_weight (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


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
pads = pad_update (pads, index, [x y]);

% udpate gui data
handles.pads = pads;
guidata(hObject, handles);

