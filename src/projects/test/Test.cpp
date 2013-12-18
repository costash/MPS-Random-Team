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

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>

using namespace std;

wstring TESSERACT(_T("tesseract\\tesseract"));

wstring ORIGINAL_IMG_DIR(_T("images\\"));
wstring ORIGINAL_OUTPUT_DIR(_T("test\\tesseract\\"));
wstring REFERENCE_TEXT_DIR(_T("test\\reference\\"));

wstring BAM_SOURCE_DIR(_T("output\\"));
wstring BAM_OUTPUT_DIR(_T("test\\bam\\"));

Test::Test(const wstring& path)
	: _path(path)
{
}

int Test::runOCR(SOURCE source)
{
	wstring IMG_DIR;
	wstring OUTPUT_DIR;

	if (source == ORIGINAL_IMG) {
		IMG_DIR = ORIGINAL_IMG_DIR;
		OUTPUT_DIR = ORIGINAL_OUTPUT_DIR;
	}
	else {
		IMG_DIR = BAM_SOURCE_DIR;
		OUTPUT_DIR = BAM_OUTPUT_DIR;
	}

	TCHAR commandLine[MAX_CMD_LINE];
	_stprintf_s(commandLine, sizeof(commandLine) / sizeof(TCHAR),
		_T("%s %s%s.tiff %s%s"), TESSERACT.c_str(),
		IMG_DIR.c_str(), _path.c_str(),
		OUTPUT_DIR.c_str(), _path.c_str());

	#ifdef _DEBUG
		_tprintf_s(_T("command: %s\n"), commandLine);
	#endif

	cout << "=========================================================" << endl;
	cout << "Analysing images with TESSERACT" << endl;

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

void Test::computeScore() {

	char word[100];
	int size = 0;
	int correct = 0;
	int wrong = 0;
	map<string, int> dict;
	map<string, int>::iterator it;

	TCHAR reference_filename[MAX_CMD_LINE];
	TCHAR bam_filename[MAX_CMD_LINE];

	_stprintf_s(reference_filename, sizeof(reference_filename) / sizeof(TCHAR),
		_T("%s%s.txt"), REFERENCE_TEXT_DIR.c_str(), _path.c_str());

	_stprintf_s(bam_filename, sizeof(bam_filename) / sizeof(TCHAR),
		_T("%s%s.txt"), BAM_OUTPUT_DIR.c_str(), _path.c_str());


	FILE *FO, *FB;
	_wfopen_s(&FO, reference_filename, _T("r"));
	_wfopen_s(&FB, bam_filename, _T("r"));

	if (FO && FB) {
		cout << "Computing image scores" << endl;
		cout << "---------------------------------------------------------" << endl;
		_ftprintf(stderr, _T("\tCompare %s with %s\n"), reference_filename, bam_filename);
	}
	else {
		if (!FO)
			_ftprintf(stderr, _T("\tFile not found: %s \n"), reference_filename);
		if (!FB)
			_ftprintf(stderr, _T("\tFile not found: %s \n"), bam_filename);
		return;
	}
	cout << "---------------------------------------------------------" << endl;

	while (!feof(FO)) {
		fscanf_s(FO, "%s", word, 100);
		dict[word]++;
		size++;
	}

	while (!feof(FB)) {
		fscanf_s(FB, "%s", word, 100);
		it = dict.find(word);
		if (it != dict.end() && it->second > 0) {
			it->second--;
			correct++;
		}
		else {
			wrong++;
		}
	}

	//for (it=dict.begin(); it!=dict.end(); it++) {
	//	if (it->second)
	//	cout << it->first << " = " << it->second << endl;
	//}

	int success = (correct * 100) / size;
	printf("\tcorrect: \t %d%% (%d / %d words)\n", success, correct, size);
	cout << endl;

}