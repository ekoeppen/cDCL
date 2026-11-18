// Manual hardware and interactive communication layer tests.
// Usage: DCLManualTests <cl-cad-bsd|cl-cad-fdserial|cl-sast-fdserial> [device]

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Exceptions/TDCLException.h>

#include "UTestCommLayers.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char* argv[])
{
	const char* argument = nil;
	if (argc == 3)
	{
		argument = argv[2];
	}
	else if (argc != 2)
	{
		(void) ::printf("Syntax error.\n");
		(void) ::printf("%s <cl-cad-bsd|cl-cad-fdserial|cl-sast-fdserial> [device]\n", argv[0]);
		return 1;
	}

	const char* testName = argv[1];
	try
	{
		if (::strcmp(testName, "cl-cad-bsd") == 0)
		{
			UTestCommLayers::ConnectAndDisconnectBSDSocket();
		}
		else if (::strcmp(testName, "cl-cad-fdserial") == 0)
		{
			UTestCommLayers::ConnectAndDisconnectFDSerialPort(argument);
		}
		else if (::strcmp(testName, "cl-sast-fdserial") == 0)
		{
			UTestCommLayers::StartAndStopTwiceFDSerialPort(argument);
		}
		else
		{
			(void) ::printf("%s is an unknown manual test.\n", testName);
			return 1;
		}
	}
	catch (TDCLException& anException)
	{
		(void) ::printf("A TDCLException occurred.\n");
#if kDCLDebugFILEandLINE
		(void) ::printf(">> %s (code: %u, error: %i) [%s:%u]\n",
			anException.GetExceptionName(),
			(unsigned int) anException.GetExCode(),
			(int) anException.GetError(),
			anException.GetFileName(),
			(unsigned int) anException.GetLine());
#else
		(void) ::printf(">> %s (code: %u, error: %i)\n",
			anException.GetExceptionName(),
			(unsigned int) anException.GetExCode(),
			(int) anException.GetError());
#endif
		return 1;
	}
	catch (...)
	{
		(void) ::printf("An unknown exception occurred.\n");
		return 1;
	}

	return 0;
}
