//===========================================================================
//===========================================================================
//===========================================================================
//==      constants.cpp                                                    ==
//===========================================================================
//===========================================================================
//===========================================================================
#include "stdafx.h"
#include "Test.h"
#include "constants.h"

wstring TESSERACT(L"..\\..\\tesseract\\tesseract");
wstring TEST_IMG_DIR(L"..\\..\\images\\");
wstring OUTPUT_DIR(L"..\\..\\output\\");

Test::Test(const wstring& path)
	: _path(path)
{
}

int Test::Run()
{

	TCHAR commandLine[MAX_CMD_LINE];
	_stprintf_s(commandLine, sizeof(commandLine) / sizeof(TCHAR),
		_T("%s %s%s %s%s"), TESSERACT.c_str(),
		TEST_IMG_DIR.c_str(), _path.c_str(),
		OUTPUT_DIR.c_str(), _path.c_str());

	if (_DEBUG)
		_tprintf_s(_T("command: %s\n"), commandLine);

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