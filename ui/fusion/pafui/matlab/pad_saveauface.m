function pad_saveauface (filename, au_id, face)

[rows cols] = size (face);

fid = fopen (filename, 'w');
fwrite (fid, au_id, 'uint32');
fwrite (fid, rows, 'uint32');
fwrite (fid, cols, 'uint32');
fwrite (fid, face, 'float32');
fclose (fid); 