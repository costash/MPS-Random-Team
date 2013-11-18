//===========================================================================
//===========================================================================
//===========================================================================
//==     Direct_Access_Image.cpp   ==   Author: Costin-Anton BOIANGIU      ==
//===========================================================================
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
#include "stdafx.h"
#include "Direct_Access_Image.h"
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
#pragma comment(lib, "./FreeImage/FreeImage.lib")
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
void KImage::SaveAs(const TCHAR* strFileName, unsigned intFormatType)
{
    static int flag_vect[] = 
    {
        BMP_DEFAULT, BMP_SAVE_RLE,
        EXR_DEFAULT, EXR_FLOAT, EXR_NONE, EXR_ZIP, EXR_PIZ,
        EXR_PXR24, EXR_B44, EXR_LC, 

        J2K_DEFAULT, 

        JP2_DEFAULT,

        JPEG_DEFAULT, JPEG_QUALITYSUPERB, JPEG_QUALITYGOOD,
        JPEG_QUALITYNORMAL, JPEG_QUALITYAVERAGE, JPEG_QUALITYBAD,
        JPEG_PROGRESSIVE, JPEG_SUBSAMPLING_411, JPEG_SUBSAMPLING_420, 
        JPEG_SUBSAMPLING_422, JPEG_SUBSAMPLING_444, JPEG_OPTIMIZE,
        JPEG_BASELINE, 

        PNG_DEFAULT, PNG_Z_BEST_SPEED, PNG_Z_DEFAULT_COMPRESSION,
        PNG_Z_BEST_COMPRESSION, PNG_Z_NO_COMPRESSION, PNG_INTERLACED,

        PNM_DEFAULT, PNM_SAVE_RAW, PNM_SAVE_ASCII,

        TIFF_DEFAULT, TIFF_CMYK, TIFF_PACKBITS,
        TIFF_DEFLATE, TIFF_ADOBE_DEFLATE, TIFF_NONE,
        TIFF_CCITTFAX3, TIFF_CCITTFAX4, TIFF_LZW,
        TIFF_JPEG, TIFF_LOGLUV, 

        TARGA_DEFAULT, TARGA_SAVE_RLE};

        static FREE_IMAGE_FORMAT format_vect[] = 
        {
            FIF_BMP, FIF_BMP,
            FIF_EXR, FIF_EXR, FIF_EXR, FIF_EXR, FIF_EXR, FIF_EXR, FIF_EXR, FIF_EXR,
            FIF_J2K,
            FIF_JP2,
            FIF_JPEG, FIF_JPEG, FIF_JPEG, FIF_JPEG, FIF_JPEG, FIF_JPEG, FIF_JPEG, FIF_JPEG,
            FIF_JPEG, FIF_JPEG, FIF_JPEG, FIF_JPEG, FIF_JPEG,
            FIF_PNG, FIF_PNG, FIF_PNG, FIF_PNG, FIF_PNG, FIF_PNG,
            FIF_PPM, FIF_PPM, FIF_PPM,
            FIF_TIFF, FIF_TIFF, FIF_TIFF, FIF_TIFF, FIF_TIFF, FIF_TIFF, FIF_TIFF, FIF_TIFF, 
            FIF_TIFF, FIF_TIFF, FIF_TIFF,
            FIF_TARGA, FIF_TARGA
        };


        for (int intY = intHeight - 1; intY >= 0; intY--)
        {  
            BYTE* crtLine = FreeImage_GetScanLine(this->fbit, intHeight - 1 - intY);
            memcpy(crtLine, pDataMatrix[intY], intLineRasterSize);
        }

        assert(intFormatType < SAVE_NO_FORMAT);
        // check for incorrect parameter
        if (intFormatType >= SAVE_NO_FORMAT)
            return;

        int flag = flag_vect[intFormatType];
        FREE_IMAGE_FORMAT fif = format_vect[intFormatType];

        BOOL res;
        WORD bpp = WORD(FreeImage_GetBPP(this->fbit));
        if(FreeImage_FIFSupportsWriting(fif) && FreeImage_FIFSupportsExportBPP(fif, bpp))
            res = FreeImage_Save_Wrapper(fif, this->fbit, strFileName, flag);
}
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
void KImage::__GaussianBlurOneChannel(int intImageWidth, int intImageHeight, 
    BYTE ** pLineInput, BYTE ** pLineOutput, double dblRadius) 
{
    ///////////////////////////////////////////////
    // do the gaussian blur
    ///////////////////////////////////////////////
    /*
    For a symmetrical gaussian blur, instead of doing a Radius*Radius matrix lookup,
    you get the EXACT same results by doing a Radius*1 transform, followed by a 1*Radius transform.
    This reduces the number of lookups exponentially (10 lookups per pixel for a Radius of 5
    instead of 25 lookups).  So, we blur the lines first, then we blur the resulting columns.
    */

    dblRadius *= 2.0;
    int intRadius = int(ceil(dblRadius));
    int intDiameter = intRadius * 2 + 1;
    double dblRemainder = ceil(dblRadius) - dblRadius;

    // create the pMaskData for the gaussian curve
    double *pMaskData = new double[intDiameter];
    double dblStandardDeviance = dblRadius / 2.0;
    for (int intX = -intRadius; intX <= intRadius; intX++) 
    {
        pMaskData[intX + intRadius] = 1.0 / sqrt(2.0 * 3.1415926 * dblStandardDeviance * dblStandardDeviance) * 
            exp(- double(intX * intX) / (2.0 * dblStandardDeviance * dblStandardDeviance));
    }

    // if there's any remainder, multiply the first/last values in MaskData it.
    // this allows us to support float radius values.
    if (dblRemainder > 0.0) 
    {
        pMaskData[0] *= dblRemainder;
        pMaskData[intDiameter - 1] *= dblRemainder;
    }

    double dblMaskSum = 0.0;
    for (int x = 0; x < intDiameter; x++) 
    {
        // this is done separately now due to the correction for float radius values above
        dblMaskSum += pMaskData[x];
    }

    for (int i = 0; i < intDiameter; i++) 
    {
        pMaskData[i] /= dblMaskSum;
    }

    // create a temporary memory pImageBuffer for the data for the first pass
    float **pImageBuffer = new float* [intImageHeight];  // don't bother about alpha/padding
    for (int intY = 0; intY < intImageHeight; intY++)
    {
        pImageBuffer[intY] = new float[intImageWidth];
        memset(pImageBuffer[intY], 0, intImageWidth * sizeof(float));
    }

    // perform a blur on each line, and place in the temporary storage
    for (int intY = 0; intY < intImageHeight; intY++) 
    {
        BYTE *pLineBuffer = pLineInput[intY];
        for (int intX = 0; intX < intImageWidth; intX++) 
        {
            // for each neighbor pixel, factor in its value/weighting to the current pixel
            for (int intPixel = 0; intPixel < intDiameter; intPixel++) 
            {
                // figure the offset of this neighbor pixel
                int intOffset = intPixel - intRadius;
                if (intX + intOffset < 0) 
                    intOffset = -intX;
                else 
                    if (intX + intOffset >= intImageWidth) 
                        intOffset = intImageWidth - intX - 1;

                // add (neighbor pixel value * pMaskData[intPixel]) to the current pixel value
                pImageBuffer[intY][intX] += float(pMaskData[intPixel] * pLineBuffer[intX + intOffset]);
            }
        }
    }

    // perform a blur on each column in the pImageBuffer, and place in the output image
    for (int intX = 0; intX < intImageWidth; intX++)
    {
        for (int intY = 0; intY < intImageHeight; intY++) 
        {
            double dblPixel = 0.0;
            // for each neighbor pixel, factor in its value/weighting to the current pixel
            for (int intPixel = 0; intPixel < intDiameter; intPixel++) 
            {
                // figure the offset of this neighbor pixel
                int intOffset = intPixel - intRadius;
                if (intY + intOffset < 0) 
                    intOffset = -intY;
                else 
                    if (intY + intOffset >= intImageHeight) 
                        intOffset = intImageHeight - intY - 1;

                // add (neighbor pixel value * pMaskData[intPixel]) to the current pixel value
                dblPixel += (pMaskData[intPixel] * pImageBuffer[intY + intOffset][intX]);
            }

            pLineOutput[intY][intX] = dblPixel < 0.0 ? BYTE(0) : (dblPixel > 255.0 ? BYTE(255) : BYTE(dblPixel + 0.5));
        }
    }

    // free the pImageBuffer
    for (int intY = 0; intY < intImageHeight; intY++)
        delete [] pImageBuffer[intY];
    delete [] pImageBuffer;

    delete [] pMaskData;
}
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
bool KImage::GaussianBlur(double dblRadius)
{
    bool boolError = false;
    if (BeginDirectAccessOnLines())
    {
        switch(GetPixelBits())
        {
        case 1:
            assert(false);
            boolError = true;
            break;
        case 8:
            {
                BYTE **pLineInput = new BYTE *[intHeight];
                for (int intY = intHeight - 1; intY >= 0; intY--)
                    pLineInput[intY] = (BYTE *)(GetLinePtr(intY));

                __GaussianBlurOneChannel(intWidth, intHeight, pLineInput, pLineInput, dblRadius);

                delete [] pLineInput;
                break;
            }
        case 24:
            {
                BYTE **pLineInput = new BYTE *[intHeight];
                for (int intY = intHeight - 1; intY >= 0; intY--)
                    pLineInput[intY] = new BYTE[intWidth];

                for (int intChannel = 0; intChannel < 3; intChannel++)
                {
                    for (int intY = intHeight - 1; intY >= 0; intY--)
                    {
                        BYTE *line = (BYTE *)GetLinePtr(intY);
                        for (int intX = 0; intX < intWidth; intX++)
                            pLineInput[intY][intX] = line[3 * intX + intChannel];
                    }

                    __GaussianBlurOneChannel(intWidth, intHeight, pLineInput, pLineInput, dblRadius);

                    for (int intY = intHeight - 1; intY >= 0; intY--)
                    {
                        BYTE *line = (BYTE *)GetLinePtr(intY);
                        for (int intX = 0; intX < intWidth; intX++)
                            line[3 * intX + intChannel] = pLineInput[intY][intX];
                    }
                }

                for (int intY = intHeight - 1; intY >= 0; intY--)
                    delete [] pLineInput[intY];
                delete [] pLineInput;

                break;
            }
        default:
            assert(false);
            boolError = true;
            break;
        }

        EndDirectAccessOnLines();
    }

    return !boolError;
}
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
//!Legacy code; will simply save a JPEG_QUALITYSUPERB
bool KImage::Reset_JP2K_Codec()
{ 
    return false;
}
//===========================================================================
//===========================================================================
