class CLogger
{
public:
	bool bLogOn;	

private:
	char szFilename[128];
	bool bFileCreateError;

	void init();

public:
	CLogger(const char* filename);

	bool WriteLog(const char* format, ...);
	void SetLoggingState(bool bNewState);
	bool GetLoggingState();
};