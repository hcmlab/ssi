function pad_saveaumap (filename, space, resolution, names, origin)

num = length (names);

fid = fopen (filename, 'w');
fwrite (fid, space, 'float32');
fwrite (fid, resolution, 'uint32');
fwrite (fid, num, 'uint32');
for i = 1:num
    len = length (names{i});
    fwrite (fid, len, 'uint32');
    fwrite (fid, names{i}, 'char');
    fwrite (fid, origin(i,:), 'float32');
end
fclose (fid); 