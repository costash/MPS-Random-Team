function ret = main(fin, fout)

im = double(rgb2gray(imread(fin)));
bwim = rtbam(im);

% imwrite(im, fout);

tags = struct();
tags.BitsPerSample = 1;
tags.SamplesPerPixel = 1;
tags.ImageLength = size(bwim,1);
tags.ImageWidth = size(bwim,2);
tags.RowsPerStrip = 16;
tags.Compression = Tiff.Compression.CCITTFax4;
tags.Photometric = Tiff.Photometric.MinIsBlack;
tags.PlanarConfiguration = Tiff.PlanarConfiguration.Chunky;
tags.Software = 'MATLAB';

t = Tiff(fout, 'w');
t.setTag(tags)
t.write(bwim);
t.close();

% imshow(imread(fout), []);

ret = 0;