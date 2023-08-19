#pragma once

#define ANALYSIS_HRESULT(InValue)\
{\
	HRESULT HandleResult = InValue;\
	if (FAILED(InValue))\
	{\
		Engine_Log_Error("Error = %i", (int)InValue);\
	}\
	else if (SUCCEEDED(InValue))\
	{\
		Engine_Log_Success("Success");\
	}\
}