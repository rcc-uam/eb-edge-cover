fgetl(stdin);
points = dlmread(stdin);
[vx, vy] = voronoi(points(:,1), points(:,2));
printf('%f %f\n%f %f\n\n', [vx(:), vy(:)]');
