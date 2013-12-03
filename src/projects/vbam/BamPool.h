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
#include <map>

class BamPool
{
public:
	BamPool(
		const TCHAR* bamsFolder,
		const TCHAR* inputImageName,
		const TCHAR* outputFolder,
		const TCHAR* outputName);

	int Init(const TCHAR* vbamExecutableName);

	void SpawnAll();
	void Vote();

private:
	std::map<std::wstring, std::unique_ptr<Bam>> _bams;
	std::vector<std::wstring> _bamNames;
	std::wstring _bamsFolder;
	std::wstring _inputImageName;
	std::wstring _outputFolder;
	std::wstring _outputName;
};

#endif // __POOL_POOL__H__
