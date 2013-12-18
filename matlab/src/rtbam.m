function bw = rtbam(im)




% Parameters:
%              k - No of standard deviations of noise to reject 2-3
%              nscale - No of filter scales to use (5-7) - the more scales used
%                       the more low frequencies are covered
%              mult   - multiplying factor between scales  (2.5-3)
%              norient - No of orientations to use (6)
%              softness - degree of soft thresholding (0-hard  1-soft)

% imshow(im, []);

im = noisecomp(im, 2, 6, 2.5, 6, 0);

% figure, imshow(im, []);
                         
%  diff = anisodiff(im, niter, kappa, lambda, option)
%
% Arguments:
%         im     - input image
%         niter  - number of iterations.
%         kappa  - conduction coefficient 20-100 ?
%         lambda - max value of .25 for stability
%         option - 1 Perona Malik diffusion equation No 1
%                  2 Perona Malik diffusion equation No 2


im = anisodiff(im, 5, 20, .2, 1);

% figure, imshow(im, []);

filt_radius = 20;
im = im / max(im(:)); % normalyze to [0, 1] range
%im = imfilter(im, fspecial('gaussian'));
%% build filter
fgrid = -filt_radius : filt_radius;
[x, y] = meshgrid(fgrid);
filt = sqrt(x .^ 2 + y .^ 2) <= filt_radius;
filt = filt / sum(filt(:));
%% calculate mean, and std
local_mean = imfilter(im, filt, 'symmetric');
local_std = sqrt(imfilter(im .^ 2, filt, 'symmetric'));
% new_local_std = sqrt(imfilter(im .^ 2, filt, 'symmetric') + local_mean.^2);
%% calculate binary image
% im_nic = im >= local_mean+ -0.2 * local_std;
% im_sauv = im >= local_mean.*(1 + 0.5 * ( local_std/128 -1));
R = max(im(:));
M = min(im(:));
im_wolf = im >= (1-0.5)*local_mean+0.5*M+0.5*(local_std/R).*(local_mean-M);
 
% im_nick = im >= local_mean - 0.2 * new_local_std;

%% plot
% figure; ax = zeros(4,1);
% ax(1) = subplot(2,2,1); imshow(im); title('original image');
% ax(2) = subplot(2,2,2); imshow(im_nic); title('Niblack');
% %ax(3) = subplot(2,2,3); imshow(im_nick); title('Nick');
% ax(3) = subplot(2,2,3); imshow(im_sauv); title('Sauvola');
% ax(4) = subplot(2,2,4); imshow(im_wolf); title('Wolf');
% linkaxes(ax, 'xy');si


bw = logical(im_wolf);

% figure, imshow(bw);

regan = 0;
if (regan)

    s = regionprops(bw, 'Orientation', 'MajorAxisLength', ...
        'MinorAxisLength', 'Eccentricity', 'Centroid');

    
    hold on

    phi = linspace(0,2*pi,50);
    cosphi = cos(phi);
    sinphi = sin(phi);

    for k = 1:length(s)
        xbar = s(k).Centroid(1);
        ybar = s(k).Centroid(2);

        a = s(k).MajorAxisLength/2;
        b = s(k).MinorAxisLength/2;

        theta = pi*s(k).Orientation/180;
        R = [ cos(theta)   sin(theta)
             -sin(theta)   cos(theta)];

        xy = [a*cosphi; b*sinphi];
        xy = R*xy;

        x = xy(1,:) + xbar;
        y = xy(2,:) + ybar;

        plot(x,y,'r','LineWidth',2);
    end
    hold off

end
