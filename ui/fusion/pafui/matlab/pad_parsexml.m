function [names, origin, au_ids, au_vals] = pad_parsexml (filename)

% read xml file
root = xmlread(filename);
document = root.getDocumentElement; 
list = document.getElementsByTagName('FacialExpression'); 

% parse xml file
data = struct ('name', {}, 'pad', {}, 'aus', {});
for i = 0:list.getLength-1
    item = list.item(i);
    name = item.getAttribute ('name');
    pad = zeros(3,1);
    item_pad = item.getElementsByTagName('PAD').item(0);
    pad(1) = str2num (item_pad.getAttribute ('p'));
    pad(2) = str2num (item_pad.getAttribute ('a'));
    pad(3) = str2num (item_pad.getAttribute ('d')); 
    item_au = item.getElementsByTagName('AU');
    aus = zeros (item_au.getLength-1,2);
    for j = 0:item_au.getLength-1
        aus(j+1,1) = str2num (item_au.item(j).getAttribute ('id'));
        aus(j+1,2) = str2num (item_au.item(j).getAttribute ('value'));        
    end
    data(i+1).name = name;    
    data(i+1).pad = pad;
    data(i+1).aus = aus;
end

% convert name to cell array
names = cellstr(char([data.name]));

% store p and a values
origin = horzcat (data.pad)';
origin(:,3) = [];

% find ids of action units
au_ids = vertcat (data.aus);
au_ids = unique (au_ids(:,1))';

% create matrix with values of action units
au_vals = zeros (length (names), length (au_ids));
for i = 1:size (au_vals, 1)
    aus = data(i).aus;
    for j = 1:size (aus)
        for k = 1:length (au_ids)
            if au_ids(k) == aus(j,1)
                au_vals(i,k) = aus(j,2);
                break;
            end            
        end
    end
end





