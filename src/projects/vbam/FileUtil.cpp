//===========================================================================
//===========================================================================
//===========================================================================
//==      FileUtil.cpp                                                     ==
//===========================================================================
//===========================================================================
//===========================================================================
#include "stdafx.h"
#include "FileUtil.h"

#include "constants.h"

int FileUtil::GetFilesInDir(const TCHAR* dirName, const TCHAR* extension,
							vector<wstring>& fileNames)
{
	WIN32_FIND_DATAW findData;
	HANDLE hFind;
	TCHAR fullPath[MAX_PATH];
	int errorCode = SUCCESS;

	// Check that the input path plus 3 is not longer than MAX_PATH.
	// Three characters are for the "\*" plus NULL appended below.
	int length_of_dir = wcsnlen_s(dirName, MAX_PATH);
	int length_of_ext = wcsnlen_s(extension, MAX_PATH);

	if (length_of_dir + length_of_ext > (MAX_PATH - 3))
	{
		_ftprintf(stderr, _T("Directory path is too long.\n"));
		return PATH_TOO_LONG_ERR;
	}

	// Prepare string for use with FindFile functions. First, copy the
	// string to a buffer, then append '\*' to the directory name, followed
	// by the extension name
	_stprintf_s(fullPath, sizeof(fullPath) / sizeof(TCHAR), _T("%s\\*%s"), dirName, extension);

	// Find the first file in the directory.
	hFind = FindFirstFile(fullPath, &findData);

	if (INVALID_HANDLE_VALUE == hFind) 
	{
		PrintLastError(_T("FindFirstFile"));
		return GetLastError();
	}

	// Traverse all the files in the directory.
	do
	{
		// We don't care about folders
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			continue;
		}
		else
		{
			fileNames.push_back(std::wstring(findData.cFileName));
		}
	}while (FindNextFile(hFind, &findData) != 0);

	errorCode = GetLastError();
	if (errorCode != ERROR_NO_MORE_FILES) 
	{
		PrintLastError(_T("FindNextFile"));
		return errorCode;
	}

	FindClose(hFind);
	return SUCCESS;
}

