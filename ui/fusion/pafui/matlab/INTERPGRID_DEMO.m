space = [-1 1 -1 1];  % die größe des grid
delta = 0.05;         % auflösung des grid
data = [0 0 0;        % die datenpunkte [p a wert]
        0.5 0.5 0.8; 
        -0.5 0.6 0.7;
        -0.6 -0.7 0.4];
corner = [1.0 0.8 0.5 0]; % die werte an den eckpunkten
% corner = [];
labels = {'neutral', 'joy', 'anger', 'sadness'}; % die label namen

subplot 221;
[xi,yi,zi] = interpgrid (data, corner, space, delta, 'gridfit');
zi(zi < 0) = 0; zi(zi > 1) = 1;
meshc(xi,yi,zi);
xlabel ('p'); ylabel('a'); title ('gridfit')
for i = 1:length (labels)
    text (data(i,1),data(i,2),data(i,3)+0.1,labels{i});
end

subplot 222;
[xi,yi,zi] = interpgrid (data, corner, space, delta, 'linear');
zi(zi < 0) = 0; zi(zi > 1) = 1;
meshc(xi,yi,zi);
xlabel ('p'); ylabel('a'); title ('griddata (linear)')
for i = 1:length (labels)
    text (data(i,1),data(i,2),data(i,3)+0.1,labels{i});
end

subplot 223;
[xi,yi,zi] = interpgrid (data, corner, space, delta, 'cubic');
zi(zi < 0) = 0; zi(zi > 1) = 1;
meshc(xi,yi,zi);
xlabel ('p'); ylabel('a'); title ('griddata (cubic)')
for i = 1:length (labels)
    text (data(i,1),data(i,2),data(i,3)+0.1,labels{i});
end

subplot 224;
[xi,yi,zi] = interpgrid (data, corner, space, delta, 'v4');
zi(zi < 0) = 0;
meshc(xi,yi,zi);
xlabel ('p'); ylabel('a'); title ('griddata (v4)')
for i = 1:length (labels)
    text (data(i,1),data(i,2),data(i,3)+0.1,labels{i});
end



