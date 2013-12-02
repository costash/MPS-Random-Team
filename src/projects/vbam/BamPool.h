//===========================================================================
//===========================================================================
//===========================================================================
//==      BamPool.h                                                      ==
//===========================================================================
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
#ifndef __BAM_POOL__H__
#define __POOL_POOL__H__
//===========================================================================
//===========================================================================
#include <stdafx.h>

#include "Bam.h"
#include <vector>

class BamPool
{
public:
	BamPool(
		const TCHAR* bamsFolder,
		const TCHAR* inputImageName,
		const TCHAR* outputFolder,
		const TCHAR* outputImageName);

	int Init(const TCHAR* vbamExecutableName);

private:
	std::vector<Bam> _bams;
	std::vector<std::wstring> _bamNames;
	std::wstring _bamsFolder;
	std::wstring _inputImageName;
	std::wstring _outputFolder;
	std::wstring _outputImageName;
};

#endif // __POOL_POOL__H__
