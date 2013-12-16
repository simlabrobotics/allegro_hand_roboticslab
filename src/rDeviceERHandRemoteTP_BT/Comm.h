
#define MAXBLOCK 1024
#define MAXPORT 4

//Flow Control flags
#define FC_DTRDSR 0x01
#define FC_RTSCTS 0x02
#define FC_XONXOFF 0x04

#define MAXBUFFER 50000

#include <windows.h>
#include <string>
#include <TCHAR.h>
#include <process.h>

class CComm
{	
public:
	CComm();
	~CComm();
	void SetXonOff(BOOL chk);			// XonOff Setup
	void SetComport(int port, DWORD rate, BYTE bytesize, BYTE stop, BYTE parity);	// Comport Setup
	void SetDtrRts(BYTE chk);			//D tr Rts Setup
	BOOL CreateCommInfo();				// Create Comport
	BOOL DestroyComm();					// Destory Comport
	int  ReadCommBlock(char*, int);		// Receive Data from Comport
	BOOL WriteCommBlock(const char*, DWORD);	// Write data to Comport
	BOOL OpenComport();					// Open Comport and try to connect
	BOOL SetupConnection();				// Open Connection
	BOOL CloseConnection();				// Close Connection
	void SetReadData(char* data, int size);		// Save read data to the buffer. 
	int GetReadData(char* buffer, int size);
	
public:
	BYTE bPort;
	BOOL fXonXoff;
	BYTE bByteSize, bFlowCtrl, bParity, bStopBits;
	DWORD dwBaudRate;
	HANDLE hWatchThread;
	DWORD dwThreadID;
	OVERLAPPED osWrite, osRead;
	
public:
	HANDLE hComm;
	BOOL fConnected;
	BYTE abIn[MAXBLOCK+1];
		
public:
	unsigned char _data_buffer[MAXBUFFER];
	int				_index_buffer;
	std::string _strBuffer;

	int _closinState;
};
unsigned __stdcall CommWatchProc(void *udata);
