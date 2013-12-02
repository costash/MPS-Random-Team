//===========================================================================
//===========================================================================
//===========================================================================
//==      BamPool.cpp                                                      ==
//===========================================================================
//===========================================================================
//===========================================================================
#include "stdafx.h"
#include "BamPool.h"
#include "FileUtil.h"
#include "constants.h"

#include <algorithm>

BamPool::BamPool(const TCHAR* bamsFolder, const TCHAR* inputImageName,
				 const TCHAR* outputFolder, const TCHAR* outputImageName)
				 : _bamsFolder(bamsFolder), _inputImageName(inputImageName),
				 _outputFolder(outputFolder), _outputImageName(outputImageName)
{
}

int BamPool::Init(const TCHAR* vbamExecutableName)
{
	int returnCode = FileUtil::GetFilesInDir(_bamsFolder.c_str(), _T(".exe"), _bamNames);
	if (returnCode == FileUtil::SUCCESS)
	{
		if (_DEBUG)
		{
			_ftprintf_s(stderr, _T("bam names size before erase = %d\n"), _bamNames.size());
		}
		_bamNames.erase(find(_bamNames.begin(), _bamNames.end(), std::wstring(vbamExecutableName)));
		if (_DEBUG)
		{
			_ftprintf_s(stderr, _T("bam names size after erase = %d\n"), _bamNames.size());
		}
	}

	return FileUtil::SUCCESS;
}

