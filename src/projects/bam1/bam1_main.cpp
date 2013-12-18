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
#include <queue>
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
		Mat diffl(sz.height+2,sz.width+2,CV_32FC1, Scalar(0.0));
		Mat tt = diffl(Range(1, sz.height+1), Range(1, sz.width+1)); //Will this actually work?
		im.copyTo(tt);
		Mat delta(sz, CV_32FC1); 
		Mat c(sz, CV_32FC1);
		Mat temp(sz, CV_32FC1);
		//Doing N
		delta = diffl(Range(0,sz.height), Range(1,sz.width+1)).clone() -diff;
		pow(-(delta/kappa),2,temp); exp(temp, c); 
		diff+=lambda*c.mul(delta);

		//Doing S
		delta = diffl(Range(2,sz.height+2), Range(1,sz.width+1)).clone() -diff;
		pow(-(delta/kappa),2,temp); exp(temp, c); 
		diff+=lambda*c.mul(delta);
		
		//Doing E
		delta = diffl(Range(1,sz.height+1), Range(2,sz.width+2)).clone() -diff;
		pow(-(delta/kappa),2,temp); exp(temp, c); 
		diff+=lambda*c.mul(delta);
		
		//Doing W
		delta = diffl(Range(1,sz.height+1), Range(0,sz.width)).clone() -diff;
		pow(-(delta/kappa),2,temp); exp(temp, c); 
		diff+=lambda*c.mul(delta);
		
		/*
		Mat deltaS(sz, CV_32FC1);
		deltaS = diffl(Range(2,sz.height+2), Range(1,sz.width+1)).clone() - diff;
		Mat deltaE(sz, CV_32FC1);
		deltaE = diffl(Range(1,sz.height+1), Range(2,sz.width+2)).clone() - diff;
		Mat deltaW(sz, CV_32FC1);
		deltaW = diffl(Range(1,sz.height+1), Range(0,sz.width)).clone() - diff;

		
		Mat cN(sz, CV_32FC1); pow(-(deltaN/kappa), 2, temp); exp(temp, cN);
		Mat cS(sz, CV_32FC1); pow(-(deltaS/kappa), 2, temp); exp(temp, cS);
		Mat cE(sz, CV_32FC1); pow(-(deltaE/kappa), 2, temp); exp(temp, cE);
		Mat cW(sz, CV_32FC1); pow(-(deltaW/kappa), 2, temp); exp(temp, cW);

		diff += lambda*(cN.mul(deltaN)+ cS.mul(deltaS)+cE.mul(deltaE)+ cW.mul(deltaW));
		*/
	}
	return diff;
}

void do_magic(int intHeight, int intWidth, BYTE **pDataMatrixGrayscale, BYTE **pDataMatrixConfidence, KImage* pImageBinary) {
	BYTE* t = (BYTE*)calloc(intHeight*intWidth,sizeof(unsigned char));
	for (int y = 0; y < intHeight; ++y)
		for (int x = 0; x < intWidth; ++x)
			t[y*intWidth+x] = pDataMatrixGrayscale[y][x];
	Mat te(intHeight, intWidth, CV_8UC1, t);
	//Mat te2(intHeight, intWidth, CV_8UC1);
	Mat img(intHeight, intWidth, CV_32FC1);
	//Mat out_wolf(intHeight, intWidth, CV_8UC1)
	
	Mat kern(15,15, CV_32FC1, Scalar((double)1./225));
	//Mat kern(21,21, CV_32FC1, Scalar((double)1./441));
	double mx, mi;
	minMaxLoc(te, &mi, &mx);
	
	te.convertTo(img, CV_32FC1, 1./mx);
	
	free(t); //te is unsafe to be used further

	Mat final;
	
	Mat out2(intHeight, intWidth, CV_32FC1);
	Mat out(intHeight, intWidth, CV_32FC1);
	Mat temp(intHeight, intWidth, CV_32FC1);
	//dilate(img, img, getStructuringElement(MORPH_CROSS, Size(3, 3)));
	img = anisodiff(img, 5, 20, .2, 1);
	//erode(img, img, getStructuringElement(MORPH_CROSS, Size(3, 3)));
	///Mat kern2(11,11, CV_32FC1, Scalar(1./121));
	//kern.at<float>(5,5) = 1;
	//normalize(img,img);
	filter2D(img, out, -1, kern);
	
	pow(img, 2, temp);
	filter2D(temp, out2, -1, kern);
	sqrt(out2, out2);
	double M, R;
	minMaxIdx(img, &M, &R);
	Mat out_savu(intHeight, intWidth, CV_8UC1);
	Mat thresh = out.mul(1+0.3*(out2/128-1));
	//compare(out*0.5 + 0.5*M+0.5*(out2 * (1/R)).mul(out-M), img, out_wolf, CMP_LE);
	compare(thresh, img, out_savu, CMP_LE);
	//erode(out_savu, out3, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	//dilate(out_savu, out_savu, getStructuringElement(MORPH_CROSS, Size(3, 3)));
	//threshold(img, out,2,255, THRESH_BINARY|THRESH_OTSU);

	
	//Contour trying 1
	/*
	const int dx[] = {0, 0, 1, -1};
	const int dy[] = {1, -1, 0, 0};
	for (int i = 0 ; i < out_wolf.rows; ++i)
		for (int j = 0; j < out_wolf.cols; ++j)
			if (out_wolf.at<bool>(i,j)) {
				std::queue<std::pair < int, int > > q;
				std::queue<std::pair < int, int > > s;
				int saved = 0;
				q.push(std::make_pair(i,j));
				out_wolf.at<bool>(i,j) = false;
				while (!q.empty()) {
					std::pair <int, int>  x = q.front(); q.pop();
					s.push(x);
					if (out_savu.at<bool>(x.first,x.second))
						++saved;
					
					for (int d = 0; d< 4; ++d) {
						int xx = x.first+dx[d];
						int yy = x.second+dy[d];
						if (0<= xx && xx < out_wolf.rows && 0<=yy && yy<out_wolf.cols && out_wolf.at<bool>(xx,yy)){
							q.push(std::make_pair(xx,yy));
							out_wolf.at<bool>(xx, yy) = false;
						}
					}
				}
				if (saved > 0.1*s.size() && s.size() > 5)
					while (!s.empty()) {
						std::pair <int, int>  x = s.front(); s.pop();
						final.at<bool>(x.first, x.second) = true;
					}
				else
					while (!s.empty())
						s.pop();
				
			}
	*/
	final = out_savu;
#ifdef _DEBUG
	namedWindow("Display Window", CV_WINDOW_AUTOSIZE);
	imshow("Display Window", final);
	waitKey(0);
#endif

	for (int i = 0 ; i < final.rows; ++i)
		for (int j = 0; j < final.cols; ++j) {
			pImageBinary->Put1BPPPixel(j,i,final.at<bool>(i,j));
			//pDataMatrixConfidence[i][j] = abs(1-abs(thresh.at<double>(i,j)*255 - pDataMatrixConfidence[i][j]))*255;
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
