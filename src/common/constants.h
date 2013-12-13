//===========================================================================
//===========================================================================
//===========================================================================
//==      constants.h                                                      ==
//===========================================================================
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
#ifndef __CONSTANTS__H__
#define __CONSTANTS__H__
//===========================================================================
//===========================================================================
#include "stdafx.h"

// The exit codes returned by BAM modules
struct BAM_EXIT_CODE {
	enum values {
		SUCCESS = 0,			// Actually anything >= 0 is SUCCESS
		READ_ERR = 1,			// Something wrong happened at reading image
		INPUT_IMAGE_ERR = 2,	// Something wrong with the input image
		WRITE_ERR = 3,			// Something wrong happened at writing output
		MEMORY_ERR = 4,			// Not enough memory or memory fragmentation
		TIMEOUT = 5				// Bam allowed time for running was exceeded
	};
};

// Exit codes for VBAM
struct VBAM_EXIT {
	enum values {
		SUCCESS = 0,			// Case of success
		ARGS_ERR = 1,			// Not enough args
		CREATE_PROCESS_ERR = 2, // Could not create process
		WAIT_PROCESS_ERR = 3,	// Could not create process
		GET_EXIT_CODE_ERR = 4	// Could not create process
	};
};

// The maximmum number of characters in a command line
const int MAX_CMD_LINE = 0x200;

static VOID PrintLastError(const TCHAR* message)
{
	TCHAR errBuff[1024];

	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK,
		NULL,
		GetLastError(),
		0,
		errBuff,
		sizeof(errBuff) - 1,
		NULL);

	_ftprintf(stderr, _T("%s: %s\n"), message, errBuff);
}

// useful macro for handling error codes
#define DIE(assertion, callDescription, exitCode)	\
	do {											\
	if (assertion) {							\
	_ftprintf(stderr, _T("(%s, %s, %d): "),	\
	__FILE__, __FUNCTION__, __LINE__);		\
	PrintLastError(callDescription);		\
	exit(exitCode);							\
	}											\
	} while(0)

//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
#endif //__CONSTANTS__H__
//===========================================================================
//===========================================================================
