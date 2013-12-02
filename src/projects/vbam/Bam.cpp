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


Bam::Bam(const std::wstring& path, const std::wstring& executableName)
	: _path(path), _exeName(executableName)
{
	createFullPath();
	if (_DEBUG)
	{
		_tprintf_s(_T("fullpath={%s}\n"), _fullPath.c_str());
	}
}

void Bam::createFullPath()
{
#ifdef _WIN32
	std::wstring separator(_T("\\"));
#else
	std::wstring separator(_T("/"));
#endif
	_fullPath = _path + separator + _exeName;
}

int Bam::Run(const std::wstring& inputImageName, const std::wstring& outputImageName,
		const std::wstring& confidenceFileName)
{
	TCHAR commandLine[MAX_CMD_LINE];
	_stprintf_s(commandLine, sizeof(commandLine) / sizeof(TCHAR),
		_T("%s %s %s %s"),
		_fullPath.c_str(),
		inputImageName.c_str(),
		outputImageName.c_str(),
		confidenceFileName.c_str());

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

	return exitCode;
}
