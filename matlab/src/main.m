function ret = main(fin, foutbw, foutconf)

im = double(rgb2gray(imread(fin)));
[bwim, conf] = rtbam(im);

imshow(im, []);
figure, imshow(conf, []);

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

t = Tiff(foutbw, 'w');
t.setTag(tags)
t.write(bwim);
t.close();

tags.Compression = Tiff.Compression.LZW;
tags.BitsPerSample = 8;
t = Tiff(foutconf, 'w');
t.setTag(tags);
t.write(conf);
t.close();

% imshow(imread(fout), []);

ret = 0;