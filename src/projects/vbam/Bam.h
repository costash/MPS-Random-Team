//===========================================================================
//===========================================================================
//===========================================================================
//==      bam.h                                                      ==
//===========================================================================
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
#ifndef __BAM__H__
#define __BAM__H__
//===========================================================================
//===========================================================================
#include "Direct_Access_Image.h"

#include <string>

// Class that holds the name of a BAM executable and can run it multiple
// times on different images.
class Bam
{
public:
	Bam(const std::wstring& path, const std::wstring& executableName);

	int Run(const std::wstring& inputImageName);

	const std::wstring& ExecutableName();
	
	enum LastRunStatus {
		NOT_EXECUTED = -1,
		EXECUTED_SUCCESSFULLY = 0,
		EXECUTED_WITH_ERROR = 1
	};

	int LastRunStatus();

	const std::wstring& LastRunOutputImageName();
	const std::wstring& LastRunConfidenceFileName();

private:
	void createFullPath();
	void createOutputNames(const std::wstring& inputName);

private:
	std::wstring _fullPath;
	std::wstring _exeName;
	std::wstring _path;
	std::wstring _lastRunOutputImageName;
	std::wstring _lastRunConfidenceFileName;
	int _lastRunStatus;
};

//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
#endif //__BAM__H__
//===========================================================================
//===========================================================================
