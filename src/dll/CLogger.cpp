#include "API_Logger.h"
#include "CLogger.h"

//CLogger.cpp
CLogger::CLogger(const char* filename)
{
	strcpy_s(szFilename, 128, filename);
	bLogOn = TRUE;
	bFileCreateError = FALSE;

	init();
}

void CLogger::init()
{
	FILE* f = fopen(szFilename, "w");
	if(f == NULL)
		bFileCreateError = TRUE;
	fclose(f);
}

bool CLogger::WriteLog(const char* format, ...)
{
	if(bFileCreateError)
		return FALSE;

	FILE* file = fopen(szFilename, "a");

	if(file == NULL)
		return FALSE;

	va_list args;
	va_start(args, format);
	vfprintf(file, format, args); 
	fprintf(file, "\n");
	va_end(args);
	fclose(file);
}