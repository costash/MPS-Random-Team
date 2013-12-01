//===========================================================================
//===========================================================================
//===========================================================================
//==      constants.cpp                                                    ==
//===========================================================================
//===========================================================================
//===========================================================================
#include "stdafx.h"
#include "Bam.h"
#include "constants.h"


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
	TCHAR commandLine[MAX_CMD_LINE];
	_stprintf_s(commandLine, sizeof(commandLine) / sizeof(TCHAR),
		_T("%s %s %s %s"),
		_fullPath,
		inputImageName,
		outputImageName,
		confidenceFileName);

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	BOOL returnError = CreateProcess(
			NULL,
			commandLine,
			NULL,
			NULL,
			TRUE,
			0,
			NULL,
			NULL,
			&si,
			&pi);

	DIE(!returnError, _T("Process could not be created"), VBAM_EXIT::CREATE_PROCESS_ERR);

	DWORD waitResult = WaitForSingleObject(pi.hProcess, INFINITE);
	DIE(waitResult == WAIT_FAILED, _T("Waiting for process failed"), VBAM_EXIT::WAIT_PROCESS_ERR);

	DWORD exitCode;
	returnError = GetExitCodeProcess(pi.hProcess, &exitCode);
	DIE(returnError == FALSE, _T("Could not get exit code for process"), VBAM_EXIT::GET_EXIT_CODE_ERR);

	return 0;
}
