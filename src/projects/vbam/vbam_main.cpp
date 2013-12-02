//===========================================================================
//===========================================================================
//===========================================================================
//==   vbam.cpp                                                            ==
//===========================================================================
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
#include "stdafx.h"
#include "Direct_Access_Image.h"
#include "constants.h"
#include "Bam.h"
#include "BamPool.h"
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
int _tmain(int argc, _TCHAR* argv[])
{
	// Verify command-line usage correctness
	if (argc != 7)
	{
		_tprintf(_T("Use: %s <timeout (miliseconds/pixel t1)>\n")
			_T("\t\t<timeout for init and destroy (t2)>\n")
			_T("\t\t<folder where BAM executables are found>\n")
			_T("\t\t<input image name for BAM> <BAM output folder>\n")
			_T("\t\t<output image name>\n"), argv[0]);
		return VBAM_EXIT::ARGS_ERR;
	}

	// Buffer for the new file names
	TCHAR outputImageName[0x100];
	TCHAR outputConfidenceName[0x100];

	/*Bam bam1(argv[3], _T("bam1.exe"));
	// Create the output name for bam1
	_stprintf_s(outputImageName, sizeof(outputImageName) / sizeof(TCHAR), _T("bam1.exe_%s.TIF"), argv[6]);
	_stprintf_s(outputConfidenceName, sizeof(outputConfidenceName) / sizeof(TCHAR), _T("bam1.exe_%s_conf.TIF"), argv[6]);
	bam1.Run(argv[4], outputImageName, outputConfidenceName);
	*/
	BamPool pool(argv[3], argv[4], argv[5], argv[6]);
	pool.Init(argv[0]);

	// Return with success
	return VBAM_EXIT::SUCCESS;
}
//===========================================================================
//===========================================================================
