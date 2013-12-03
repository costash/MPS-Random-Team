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

	return returnCode;
}

void BamPool::SpawnAll()
{
	for (unsigned int i = 0; i < _bamNames.size(); ++i)
	{
		_bams[_bamNames[i]].reset(new Bam(_bamsFolder, _bamNames[i]));
		_bams[_bamNames[i]].get()->Run(_inputImageName);
	}
}

void BamPool::Vote()
{

}

