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
#include <cstring>
#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\photo\photo.hpp>
//===========================================================================
//===========================================================================

using namespace cv;

void nosiecomp(Mat im, double k, int nscale, double mult, int norient, double softness) {
	
	int minWaveLength = 2;
	double sigmaOnf = 0.55;
	double dThetaOnSigma = 1.;
	double epsilon = .00001;
	double thetaSigma = 3.1415926535/(double)norient/dThetaOnSigma;
	Size sz = im.size();
	Mat imagefft(sz, 0);
	dft(im, imagefft);




}

Mat anisodiff(Mat im, int niter, double kappa, double lambda, int option) {

	Size sz = im.size();
	Mat diff = im.clone();
	for (size_t i = 0; i < niter; ++i) {
		Mat diffl(sz.height+2,sz.width+2,CV_64FC1, Scalar(0.0));
		Mat tt = diffl(Range(1, sz.height+1), Range(1, sz.width+1)); //Will this actually work?
		im.copyTo(tt);
		Mat deltaN(sz, CV_64FC1); 
		deltaN = diffl(Range(0,sz.height), Range(1,sz.width+1)).clone() -diff;
		Mat deltaS(sz, CV_64FC1);
		deltaS = diffl(Range(2,sz.height+2), Range(1,sz.width+1)).clone() - diff;
		Mat deltaE(sz, CV_64FC1);
		deltaE = diffl(Range(1,sz.height+1), Range(2,sz.width+2)).clone() - diff;
		Mat deltaW(sz, CV_64FC1);
		deltaW = diffl(Range(1,sz.height+1), Range(0,sz.width)).clone() - diff;

		Mat temp(sz, CV_64FC1);
		Mat cN(sz, CV_64FC1); pow(-(deltaN/kappa), 2, temp); exp(temp, cN);
		Mat cS(sz, CV_64FC1); pow(-(deltaS/kappa), 2, temp); exp(temp, cS);
		Mat cE(sz, CV_64FC1); pow(-(deltaE/kappa), 2, temp); exp(temp, cE);
		Mat cW(sz, CV_64FC1); pow(-(deltaW/kappa), 2, temp); exp(temp, cW);

		diff += lambda*(cN.mul(deltaN)+ cS.mul(deltaS)+cE.mul(deltaE)+ cW.mul(deltaW));
	}
	return diff;
}

void do_magic(int intHeight, int intWidth, BYTE **pDataMatrixGrayscale, BYTE **pDataMatrixConfidence, KImage* pImageBinary) {
	BYTE* t = (BYTE*)calloc(intHeight*intWidth,sizeof(unsigned char));
	for (int y = 0; y < intHeight; ++y)
		for (int x = 0; x < intWidth; ++x)
			t[y*intWidth+x] = pDataMatrixConfidence[y][x];
	Mat te(intHeight, intWidth, CV_8UC1, t);
	Mat te2(intHeight, intWidth, CV_8UC1);
	Mat img(intHeight, intWidth, CV_64FC1);
	Mat out3(intHeight, intWidth, CV_64FC1);
	Mat out_savu(intHeight, intWidth, CV_64FC1);
	Mat out2(intHeight, intWidth, CV_64FC1);
	Mat out(intHeight, intWidth, CV_64FC1);
	Mat temp(intHeight, intWidth, CV_64FC1);
	
	Mat kern(21,21, CV_64FC1, Scalar((double)1./441));
	
	double mx, mi;
	minMaxLoc(te, &mi, &mx);
	
	te.convertTo(img, CV_64FC1, 1./mx);

	img = anisodiff(img, 5, 20, .2, 1);
	
	///Mat kern2(11,11, CV_64FC1, Scalar(1./121));
	//kern.at<float>(5,5) = 1;
	//normalize(img,img);
	filter2D(img, out, -1, kern);
	
	pow(img, 2, temp);
	filter2D(temp, out2, -1, kern);
	sqrt(out2, out2);
	double M, R;
	minMaxIdx(img, &M, &R);
	compare(out*0.5 + 0.5*M+0.5*(out2 * (1/R)).mul(out-M), img, out3, CMP_LE);
	compare(out.mul(1+0.3*(out2/128-1)), img, out_savu, CMP_LE);
	//erode(out_savu, out3, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	//threshold(img, out,2,255, THRESH_BINARY|THRESH_OTSU);
#ifdef _DEBUG
	namedWindow("Display Window", CV_WINDOW_AUTOSIZE);
	imshow("Display Window", out3);
	waitKey(0);
#endif
	free(t);
	for (int i = 0 ; i < out3.rows; ++i)
		for (int j = 0; j < out3.cols; ++j)
			pImageBinary->Put1BPPPixel(j,i,out3.at<bool>(i,j));
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
	//pImage->GaussianBlur(0.5);
#ifdef _DEBUG
	_stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), _T("%s_blurred.TIF"), argv[0]);
	pImage->SaveAs(strNewFileName, SAVE_TIFF_LZW);
#endif
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


#ifdef _DEBUG
		//... and save grayscale image
		_stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), _T("%s_grayscale.TIF"), argv[0]);
		pImageGrayscale->SaveAs(strNewFileName, SAVE_TIFF_LZW);
#endif

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
