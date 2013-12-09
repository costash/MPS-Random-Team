//===========================================================================
//===========================================================================
//===========================================================================
//==   Direct_Access_Image_Sample.cpp  ==  Author: Costin-Anton BOIANGIU   ==
//===========================================================================
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
#include "stdafx.h"
#include "Direct_Access_Image.h"
#include "constants.h"
//===========================================================================
//===========================================================================

void do_magic(int intHeight, int intWidth, BYTE **pDataMatrixGrayscale, BYTE **pDataMatrixConfidence, KImage* pImageBinary) {
	//Apply a threshold at half the grayscale range (0x00 is Full-Black, 0xFF is Full-White, 0x80 is the Middle-Gray)
	for (int y = intHeight - 1; y >= 0; y--)
		for (int x = intWidth - 1; x >= 0; x--)
		{
			//You may use this instead of the line below: 
			//    BYTE PixelAtXY = pImageGrayscale->Get8BPPPixel(x, y)
			BYTE &PixelAtXY = pDataMatrixGrayscale[y][x];
			if (PixelAtXY < 0x80)
				//...if closer to black, set to black
				pImageBinary->Put1BPPPixel(x, y, false);
			else
				//...if closer to white, set to white
				pImageBinary->Put1BPPPixel(x, y, true);
		}
}



//===========================================================================
//===========================================================================
int _tmain(int argc, _TCHAR* argv[])
{
	//Verify command-line usage correctness
	if (argc != 4)
	{
		_tprintf(_T("Use: %s <Input_Image_File_Name (24BPP True-Color)> <Output_Image_File_Name (1BPP)> <Output_Image_Confidence_File_Name> \n"), argv[0]);
		return BAM_EXIT_CODE::READ_ERR;
	}

	//Buffer for the new file names
	TCHAR strNewFileName[0x100];

	//Load and verify that input image is a True-Color one
	KImage *pImage = new KImage(argv[1]);
	if (pImage == NULL || !pImage->IsValid() )
	{
		_tprintf(_T("File %s can't be read!"), argv[0]);
		return BAM_EXIT_CODE::READ_ERR;
	}

	if (pImage->GetBPP() != 24)
	{
		_tprintf(_T("File %s does is not a valid True-Color image!"), argv[0]);
		return BAM_EXIT_CODE::INPUT_IMAGE_ERR;
	}

	//Apply a Gaussian Blur with small radius to remove obvious noise
	pImage->GaussianBlur(0.5);
	if (_DEBUG) {
		_stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), _T("%s_blurred.TIF"), argv[0]);
		pImage->SaveAs(strNewFileName, SAVE_TIFF_LZW);
	}
	//Convert to grayscale
	KImage *pImageGrayscale = pImage->ConvertToGrayscale();


	//Don't forget to delete the original, now useless image
	delete pImage;

	//Verify conversion success...
	if (pImageGrayscale == NULL || !pImageGrayscale->IsValid() || pImageGrayscale->GetBPP() != 8)
	{
		_tprintf(_T("Conversion to grayscale was not successfull!"));
		return BAM_EXIT_CODE::INPUT_IMAGE_ERR;
	}
	KImage *pImageConfidence = new KImage(*pImageGrayscale);


	if (_DEBUG) {
		//... and save grayscale image
		_stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), _T("%s_grayscale.TIF"), argv[0]);
		pImageGrayscale->SaveAs(strNewFileName, SAVE_TIFF_LZW);
	}

	//Request direct access to image pixels in raw format
	BYTE **pDataMatrixGrayscale = NULL;
	BYTE **pDataMatrixConfidence = NULL;
	if (pImageGrayscale->BeginDirectAccess() && (pDataMatrixGrayscale = pImageGrayscale->GetDataMatrix()) != NULL)
	{
		//If direct access is obtained get image attributes and start processing pixels
		int intWidth = pImageGrayscale->GetWidth();
		int intHeight = pImageGrayscale->GetHeight();

		//Create binary image
		KImage *pImageBinary = new KImage(intWidth, intHeight, 1);
		if (pImageBinary->BeginDirectAccess() && pImageConfidence->BeginDirectAccess() && (pDataMatrixConfidence = pImageConfidence->GetDataMatrix()) != NULL)
		{
			do_magic(intHeight, intWidth, pDataMatrixGrayscale, pDataMatrixConfidence, pImageBinary);

			//Close direct access
			pImageBinary->EndDirectAccess();
			pImageConfidence->EndDirectAccess();

			//Save binarized image
			_stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), _T("%s"), argv[2]);
			if (!pImageBinary->SaveAs(strNewFileName, SAVE_TIFF_CCITTFAX4)) {
				_tprintf(_T("Unable to save binary image: %s"), strNewFileName);
				return BAM_EXIT_CODE::WRITE_ERR;
			}

			//Save confidence image
			_stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), _T("%s"), argv[3]);
			if (!pImageConfidence->SaveAs(strNewFileName, SAVE_TIFF_LZW)) {
				_tprintf(_T("Unable to save confidence image: %s"), strNewFileName);
				return BAM_EXIT_CODE::WRITE_ERR;
			}

			//Don't forget to delete the binary image
			delete pImageBinary;
			delete pImageConfidence;
		}
		else
		{
			_tprintf(_T("Unable to obtain direct access in binary image!")); //Nu am idee de alta eroare
			return BAM_EXIT_CODE::MEMORY_ERR;
		}

		//Close direct access
		pImageGrayscale->EndDirectAccess();
	}
	else
	{
		_tprintf(_T("Unable to obtain direct access in grayscale image!")); //Nu am idee de alta eroare
		return BAM_EXIT_CODE::MEMORY_ERR;
	}

	//Don't forget to delete the grayscale image
	delete pImageGrayscale;

	//Return with success
	return BAM_EXIT_CODE::SUCCESS;
}
//===========================================================================
//===========================================================================
