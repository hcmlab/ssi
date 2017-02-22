clear all;
close all;

basedir = [pwd '\map\map_2\'];
delete ([basedir '*.map']);

filename = 'expressions.xml';
[names, origin, au_ids, au_vals] = pad_parsexml ([basedir filename]);

space = [-1 1 -1 1];  % die größe des grid
resolution = [50 50];         % auflösung des grid

% corner = [0 0 0 0]; % die werte an den eckpunkten
corner = [];

% store grid
pad_saveaumap ([basedir 'info.map'], space, resolution, names, origin);

for i = 1:length (au_ids)
    
    if (au_ids(i) < 10)
        au_filename = [basedir 'au_0' num2str(au_ids(i)) '.map'];
    else
        au_filename = [basedir 'au_' num2str(au_ids(i)) '.map'];
    end
    
    figure;
    title (au_filename);
    
    % create map
    data = [origin au_vals(:,i)];
    [xi,yi,zi] = interpgrid (data, corner, space, resolution, 'v4');
    zi(zi < 0) = 0; zi(zi > 1) = 1;
    
    % plot map    
    meshc(xi,yi,zi);
    box on;
    xlabel ('p'); ylabel('a'); title (['id = ' num2str(au_ids(i))]);
    
    zmax = max (zi(:));
    for j = 1:length (names)
        line ([data(j,1) data(j,1)], [data(j,2) data(j,2)], [data(j,3) zmax], 'Color', 'r', 'Marker', '.');
        text (data(j,1),data(j,2),zmax+0.1,names{j});
    end
       
    % store action unit
	pad_saveauface (au_filename, au_ids(i), zi);
    
end




