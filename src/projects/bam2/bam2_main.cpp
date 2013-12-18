//===========================================================================
//===========================================================================
//===========================================================================
//==   Direct_Access_Image_Sample.cpp  ==  Author: Costin-Anton BOIANGIU   ==
//===========================================================================
//===========================================================================
//===========================================================================


//===========================================================================
//===========================================================================
#include <iostream>
#include "stdafx.h"
#include "Direct_Access_Image.h"
#include "constants.h"
#include <cstring>
#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>
//===========================================================================
//===========================================================================

using namespace cv;
using namespace std;

#define uget(x,y)    at<unsigned char>(y,x)
#define uset(x,y,v)  at<unsigned char>(y,x)=v;
#define fget(x,y)    at<float>(y,x)
#define fset(x,y,v)  at<float>(y,x)=v;

void do_magic(int intHeight, int intWidth, BYTE **pDataMatrixGrayscale, BYTE **pDataMatrixConfidence, KImage* pImageBinary) {
	BYTE* t = (BYTE*)calloc(intHeight*intWidth,sizeof(unsigned char));
	for (int y = 0; y < intHeight; ++y)
		for (int x = 0; x < intWidth; ++x)
			t[y*intWidth+x] = pDataMatrixConfidence[y][x];
	Mat te(intHeight, intWidth, CV_8UC1, t);

	Mat img(intHeight, intWidth, CV_64FC1);
	Mat out3(intHeight, intWidth, CV_64FC1);
	Mat out2(intHeight, intWidth, CV_64FC1);
	Mat out(intHeight, intWidth, CV_64FC1);
	Mat temp(intHeight, intWidth, CV_64FC1);
	Mat kern(41,41, CV_64FC1, Scalar((double)1./1681));
	//Mat kern(21,21, CV_64FC1, Scalar((double)1./441));
	double mx, mi;
	minMaxLoc(te, &mi, &mx);
	te.convertTo(img, CV_64FC1, 1./mx);

	filter2D(img, out, -1, kern);
	
	pow(img, 2, temp);
	filter2D(temp, out2, -1, kern);
	sqrt(out2, out2);
	double M, R;
	minMaxIdx(img, &M, &R);
	compare(out*0.5 + 0.5*M+0.5*(out2 * (1/R)).mul(out-M), img, out3, CMP_LE);

	free(t);
	for (int i = 0 ; i < out3.rows; ++i)
		for (int j = 0; j < out3.cols; ++j)
			pImageBinary->Put1BPPPixel(j,i,out3.at<bool>(i,j));
}

BYTE *get_histogram(int intHeight, int intWidth, BYTE** img)
{
	BYTE* t = (BYTE*)calloc(256, sizeof(unsigned char));

	for (int i = 0; i < intHeight; i++) {
		for (int j = 0; j < intWidth; j++) {
			t[img[i][j]]++;
		}
	}

	return t;
}

int get_otsu_threshold(BYTE *histogram, int total) {
	int sum = 0;
	int sumB = 0;
	int wB = 0, wF = 0;
	double mB = 0, mF = 0;
	double max = 0, between;
	int threshold = 0;

	for (int i = 1; i < 256; i++) {
		sum += i * histogram[i];
	}

	for (int i = 0; i < 256; i++) {
		wB += histogram[i];
		if (wB ==0)
			continue;
		wF = total - wB;
		if (wF == 0)
			break;
		sumB += i * histogram[i];
		mB = (double)sumB / wB;
		mF = (double)(sum - sumB) / wF;
		between = wB * wF * pow(mB - mF, 2);
		if (between > max) {
			max = between;
			threshold = i;
		}
	}

	return threshold;
}

void do_otsu_magic(int intHeight, int intWidth, BYTE **pDataMatrixGrayscale, BYTE **pDataMatrixConfidence, KImage* pImageBinary)
{
	BYTE* histogram = get_histogram(intHeight, intWidth, pDataMatrixGrayscale);
	int threshold = get_otsu_threshold(histogram, intHeight * intWidth);
#ifdef _DEBUG
	cout << "otsu threshold = " << threshold;
#endif
	for (int i = 0; i < intHeight; i++)
		for (int j = 0; j < intWidth; j++)
			pImageBinary->Put1BPPPixel(j, i, pDataMatrixGrayscale[i][j] > threshold ? true : false);

	free(histogram);
}

Mat get_mat(int intHeight, int intWidth, BYTE **matrix)
{
	BYTE* t = (BYTE*)calloc(intHeight*intWidth, sizeof(unsigned char));
	for (int y = 0; y < intHeight; ++y)
		for (int x = 0; x < intWidth; ++x)
			t[y*intWidth+x] = matrix[y][x];
	Mat img(intHeight, intWidth, CV_8UC1, t);
	free(t);
	return img;
}

void do_otsu_magic2(int intHeight, int intWidth, BYTE **pDataMatrixGrayscale, BYTE **pDataMatrixConfidence, KImage* pImageBinary)
{
	BYTE* t = (BYTE*)calloc(intHeight*intWidth, sizeof(unsigned char));
	for (int y = 0; y < intHeight; ++y)
		for (int x = 0; x < intWidth; ++x)
			t[y*intWidth+x] = pDataMatrixGrayscale[y][x];
	Mat img(intHeight, intWidth, CV_8UC1, t);

	Mat out(intHeight, intWidth, CV_64FC1);

	threshold(img, out, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

	for (int i = 0 ; i < out.rows; ++i)
		for (int j = 0; j < out.cols; ++j)
			pImageBinary->Put1BPPPixel(j,i,out.at<bool>(i,j));

	free(t);
}

// *************************************************************
// code adapted from http://liris.cnrs.fr/christian.wolf/software/binarize/index.html
// *************************************************************
void calcLocalStats (Mat &im, Mat &map_m, Mat &map_B, int winx, int winy) {

	double m, B, sum, sum_sq, foo;
	int wxh	= winx/2;
	int wyh	= winy/2;
	int x_firstth= wxh;
	int y_lastth = im.rows-wyh-1;
	int y_firstth= wyh;
	double winarea = winx*winy;

	for	(int j = y_firstth ; j<=y_lastth; j++) 
	{
		// Calculate the initial window at the beginning of the line
		sum = sum_sq = 0;
		for	(int wy=0 ; wy<winy; wy++)
			for	(int wx=0 ; wx<winx; wx++) {
				foo = im.uget(wx,j-wyh+wy);
				sum    += foo;
				sum_sq += foo*foo;
			}
		m  = sum / winarea;
		B  = (sum_sq - (sum*sum) / winarea) / winarea;

		map_m.fset(x_firstth, j, m);
		map_B.fset(x_firstth, j, B);

		// Shift the window, add and remove	new/old values to the histogram
		for	(int i=1 ; i <= im.cols-winx; i++) {

			// Remove the left old column and add the right new column
			for (int wy=0; wy<winy; ++wy) {
				foo = im.uget(i-1,j-wyh+wy);
				sum    -= foo;
				sum_sq -= foo*foo;
				foo = im.uget(i+winx-1,j-wyh+wy);
				sum    += foo;
				sum_sq += foo*foo;
			}
			m  = sum / winarea;
			B  = (sum_sq - (sum*sum) / winarea) / winarea;

			map_m.fset(i+wxh, j, m);
			map_B.fset(i+wxh, j, B);
		}
	}
}


/**********************************************************
 * code adapted from http://liris.cnrs.fr/christian.wolf/software/binarize/index.html
 * modified to perform NICK binarization
 * The binarization routine
 **********************************************************/
void NICK (Mat im, Mat output, int winx, int winy, double k)
{
	double m, B;
	double th=0;
	double min_I, max_I;
	int wxh	= winx/2;
	int wyh	= winy/2;
	int x_firstth= wxh;
	int x_lastth = im.cols-wxh-1;
	int y_lastth = im.rows-wyh-1;
	int y_firstth= wyh;
	int mx, my;

	// Create local statistics and store them in a double matrices
	Mat map_m = Mat::zeros (im.rows, im.cols, CV_32F);
	Mat map_B = Mat::zeros (im.rows, im.cols, CV_32F);
	calcLocalStats (im, map_m, map_B, winx, winy);
	
	minMaxLoc(im, &min_I, &max_I);
			
	Mat thsurf (im.rows, im.cols, CV_32F);
			
	// Create the threshold surface, including border processing
	// ----------------------------------------------------

	for	(int j = y_firstth ; j<=y_lastth; j++) {

		// NORMAL, NON-BORDER AREA IN THE MIDDLE OF THE WINDOW:
		for	(int i=0 ; i <= im.cols-winx; i++) {

			m  = map_m.fget(i+wxh, j);
			B  = map_B.fget(i+wxh, j);

			// Calculate the threshold
			th = m + k * sqrt(B + m * m);

			thsurf.fset(i+wxh,j,th);

			if (i==0) {
				// LEFT BORDER
				for (int i=0; i<=x_firstth; ++i)
					thsurf.fset(i,j,th);

				// LEFT-UPPER CORNER
				if (j==y_firstth)
					for (int u=0; u<y_firstth; ++u)
					for (int i=0; i<=x_firstth; ++i)
						thsurf.fset(i,u,th);

				// LEFT-LOWER CORNER
				if (j==y_lastth)
					for (int u=y_lastth+1; u<im.rows; ++u)
					for (int i=0; i<=x_firstth; ++i)
						thsurf.fset(i,u,th);
			}

			// UPPER BORDER
			if (j==y_firstth)
				for (int u=0; u<y_firstth; ++u)
					thsurf.fset(i+wxh,u,th);

			// LOWER BORDER
			if (j==y_lastth)
				for (int u=y_lastth+1; u<im.rows; ++u)
					thsurf.fset(i+wxh,u,th);
		}

		// RIGHT BORDER
		for (int i=x_lastth; i<im.cols; ++i)
			thsurf.fset(i,j,th);

		// RIGHT-UPPER CORNER
		if (j==y_firstth)
			for (int u=0; u<y_firstth; ++u)
			for (int i=x_lastth; i<im.cols; ++i)
				thsurf.fset(i,u,th);

		// RIGHT-LOWER CORNER
		if (j==y_lastth)
			for (int u=y_lastth+1; u<im.rows; ++u)
			for (int i=x_lastth; i<im.cols; ++i)
				thsurf.fset(i,u,th);
	}
	//cerr << "surface created" << endl;
	
	
	for	(int y=0; y<im.rows; ++y) 
	for	(int x=0; x<im.cols; ++x) 
	{
		if (im.uget(x,y) >= thsurf.fget(x,y))
		{
			output.uset(x,y,255);
		}
		else
		{
			output.uset(x,y,0);
		}
	}
}

void do_NICK_magic(int intHeight, int intWidth, BYTE **pDataMatrixGrayscale, BYTE **pDataMatrixConfidence, KImage* pImageBinary)
{
	BYTE* t = (BYTE*)calloc(intHeight*intWidth, sizeof(unsigned char));
	for (int y = 0; y < intHeight; ++y)
		for (int x = 0; x < intWidth; ++x)
			t[y*intWidth+x] = pDataMatrixGrayscale[y][x];
	Mat img(intHeight, intWidth, CV_8U, t);

	Mat out(intHeight, intWidth, CV_8U);

	NICK(img, out, 40, 40, -0.2);

	for (int i = 0 ; i < out.rows; ++i)
		for (int j = 0; j < out.cols; ++j)
			pImageBinary->Put1BPPPixel(j,i,out.at<bool>(i,j));
	
	free(t);
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
			do_NICK_magic(intHeight, intWidth, pDataMatrixGrayscale, pDataMatrixConfidence, pImageBinary);

			// create 50-50 confidence img
			for (int i = 0 ; i < intHeight; ++i)
				for (int j = 0; j < intWidth; ++j)
					pImageConfidence->Put8BPPPixel(j, i, 128);

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
