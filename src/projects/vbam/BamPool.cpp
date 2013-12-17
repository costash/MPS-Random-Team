//===========================================================================
//===========================================================================
//===========================================================================
//==      BamPool.cpp                                                      ==
//===========================================================================
//===========================================================================
//===========================================================================
#include "stdafx.h"

#include <algorithm>

#include "constants.h"
#include "BamPool.h"
#include "FileUtil.h"
#include "Matrix.h"

BamPool::BamPool(const TCHAR* bamsFolder, const TCHAR* inputImageName,
				 const TCHAR* outputFolder, const TCHAR* outputName)
				 : _bamsFolder(bamsFolder), _inputImageName(inputImageName),
				 _outputFolder(outputFolder), _outputName(outputName)
{
}

int BamPool::Init(const TCHAR* vbamExecutableName)
{
	int returnCode = FileUtil::GetFilesInDir(_bamsFolder.c_str(), _T(".exe"), _bamNames);
	if (returnCode == FileUtil::SUCCESS)
	{
		// Skip my executable if bams are in the current folder
#ifdef _DEBUG
		_ftprintf_s(stderr, _T("bam names size before erase = %d\n"), _bamNames.size());
		for (unsigned int i = 0; i < _bamNames.size(); ++i)
		{
			_ftprintf_s(stderr, _T("%s "), _bamNames[i].c_str());
		}
		_ftprintf_s(stderr, _T("\n"));
#endif
		auto it = find(_bamNames.begin(), _bamNames.end(), std::wstring(vbamExecutableName));
		if (it != _bamNames.end())
		{
			_bamNames.erase(it);
		}
#ifdef _DEBUG
		_ftprintf_s(stderr, _T("bam names size after erase = %d\n"), _bamNames.size());
		for (unsigned int i = 0; i < _bamNames.size(); ++i)
		{
			_ftprintf_s(stderr, _T("%s "), _bamNames[i].c_str());
		}
		_ftprintf_s(stderr, _T("\n"));
#endif
	}

	return returnCode;
}

void BamPool::SpawnAll(const TCHAR* processingTimeout, const TCHAR* initTimeout)
{
	unsigned int timeout = composeTimeout(processingTimeout, initTimeout);
	for (unsigned int i = 0; i < _bamNames.size(); ++i)
	{
		_bams[_bamNames[i]].reset(new Bam(_bamsFolder, _bamNames[i]));
		_bams[_bamNames[i]].get()->Run(_inputImageName, timeout);
	}
}

void BamPool::Vote()
{
	std::unique_ptr<Matrix<int>> zeroConfidences;
	std::unique_ptr<Matrix<int>> oneConfidences;
	int width = 0;
	int height = 0;

	for (unsigned int i = 0; i < _bamNames.size(); ++i)
	{
		Bam* bam = _bams[_bamNames[i]].get();
		if (bam->LastRunStatus() != Bam::EXECUTED_SUCCESSFULLY)
		{
			continue;
		}

		std::unique_ptr<KImage> binarizedImage(new KImage(bam->LastRunOutputImageName().c_str()));
		if (binarizedImage.get() == NULL || !binarizedImage.get()->IsValid() )
		{
			_tprintf(_T("File %s can't be read!"), bam->LastRunOutputImageName().c_str());
			continue;
		}

		if (binarizedImage.get()->GetBPP() != 1)
		{
			_tprintf(_T("File %s is not a valid 1BPP image!"), bam->LastRunOutputImageName().c_str());
			continue;
		}
		std::unique_ptr<KImage> confidenceImage(new KImage(bam->LastRunConfidenceFileName().c_str()));
		if (confidenceImage.get() == NULL || !confidenceImage.get()->IsValid())
		{
			_tprintf(_T("File %s can't be read!"), bam->LastRunConfidenceFileName().c_str());
			continue;
		}
		if (confidenceImage.get()->GetBPP() != 8)
		{
			_tprintf(_T("File %s is not a valid 8BPP image!"), bam->LastRunConfidenceFileName().c_str());
			continue;
		}
		if (!binarizedImage.get()->BeginDirectAccess() || !confidenceImage->BeginDirectAccess())
		{
			_tprintf(_T("Files %s or %s could not begin direct access!"),
				bam->LastRunOutputImageName(), bam->LastRunConfidenceFileName().c_str());
			continue;
		}

		// Create the accumulator matrices
		if (i == 0)
		{
			width = confidenceImage.get()->GetWidth();
			height = confidenceImage.get()->GetHeight();
			zeroConfidences.reset(new Matrix<int>(width, height));
			oneConfidences.reset(new Matrix<int>(width, height));
		}

		for (int c = 0; c < height; ++c)
		{
			for (int r = 0; r < width; ++r)
			{
				if (binarizedImage.get()->Get1BPPPixel(r, c) == false)
				{
					int confAccum = confidenceImage.get()->Get8BPPPixel(r, c) + zeroConfidences.get()->Get(r, c);
					zeroConfidences.get()->Set(r, c, confAccum);
				}
				else
				{
					int confAccum = confidenceImage.get()->Get8BPPPixel(r, c) + oneConfidences.get()->Get(r, c);
					oneConfidences.get()->Set(r, c, confAccum);
				}
			}
		}

		binarizedImage.get()->EndDirectAccess();
		confidenceImage.get()->EndDirectAccess();
	}

	// Create the final output image
	std::unique_ptr<KImage> votedOutput(new KImage(width, height, 1));
	if (votedOutput.get()->BeginDirectAccess())
	{
		for (int c = 0; c < height; ++c)
		{
			for (int r = 0; r < width; ++r)
			{
				bool pixel = zeroConfidences.get()->Get(r, c) < oneConfidences.get()->Get(r, c);
				votedOutput.get()->Put1BPPPixel(r, c, pixel);
			}
		}
	}

	// Save output image
	if (!votedOutput.get()->SaveAs((_outputName + std::wstring(_T(".TIFF"))).c_str(), SAVE_TIFF_CCITTFAX4))
	{
		_tprintf(_T("Unable to save confidence image: %s"), (_outputName + std::wstring(_T(".TIFF"))).c_str());
	}

	votedOutput.get()->EndDirectAccess();
}

unsigned int BamPool::composeTimeout(const TCHAR* processingTimeout, const TCHAR* initTimeout)
{
	std::wstring processing(processingTimeout);
	std::wstring init(initTimeout);

	float processingMilisec = stof(processingTimeout);
	unsigned int initMilisec = stoul(initTimeout);

	std::unique_ptr<KImage> input(new KImage(_inputImageName.c_str()));
	int pixels = input.get()->GetWidth() * input.get()->GetHeight();

	return (unsigned int) (pixels * processingMilisec) + initMilisec;
}
