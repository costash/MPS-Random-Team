//===========================================================================
//===========================================================================
//===========================================================================
//==      constants.cpp                                                    ==
//===========================================================================
//===========================================================================
//===========================================================================
#include "stdafx.h"
#include "Bam.h"


Bam::Bam(const TCHAR* path, const TCHAR* executableName)
{
	createFullPath(path, executableName);
	_tprintf_s(_T("fullpath={%s}\n"), _fullPath);
}

void Bam::createFullPath(const TCHAR* path, const TCHAR* name)
{
#ifdef _WIN32
	TCHAR separator[2] = _T("\\");
#else
	TCHAR separator[2] = _T("/");
#endif
	_stprintf_s(_fullPath, sizeof(_fullPath) / sizeof(TCHAR), _T("%s%s%s"),
		path, separator, name);
}

int Bam::Run(const TCHAR* inputImageName, const TCHAR* outputImageName,
		const TCHAR* confidenceFileName)
{
	return 0;
}
