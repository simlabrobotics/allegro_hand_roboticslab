
#include "comm.h"

static uintptr_t s_updater = 0;

CComm::CComm()
{
	hComm=NULL;				// Initialization the Handle for Comport
	bFlowCtrl=FC_XONXOFF;	// Setup the flow_control
	fConnected=FALSE;		// thread stop

	memset(_data_buffer, 0, MAXBUFFER);
	_index_buffer = 0;

	_closinState = 0;
}

CComm::~CComm()
{
	//DestroyComm();
}

// Communication procedure, observation routine, this routine connected to procedure when OpenComPort function is executed.
unsigned __stdcall CommWatchProc(void *udata)
{
	DWORD dwEvtMask;
	CComm *npComm = (CComm *)udata;	// Declaration the CComm class pointer
	char InData[MAXBLOCK+1];			// Array for receive data
	int nLength;						// Variable for length for receive data
	if(npComm==NULL) return -1;			// if npComm handle doesn't have no comports attached, it returns error

	//waiting for EVENT when fConnectedis TRUE
	while(npComm->fConnected) {
		COMSTAT comstat;
		DWORD dwErrorFlag;

		ClearCommError(npComm->hComm, &dwErrorFlag, &comstat);
		if(comstat.cbInQue > 0)
		{
			do {
				memset(InData, 0, MAXBLOCK); // Initialize the array InData to 0
				
				if(nLength = npComm->ReadCommBlock((char*)InData, MAXBLOCK)) {	//Check the buffer has any data
					npComm->SetReadData(InData, nLength);		// Receive data
				}
			}
			while(nLength>0);	
		}

		Sleep(100);
	}

	npComm->_closinState = 0x01;

	return TRUE;
}

// copy receive data to 'data'
void CComm::SetReadData(char* data, int size)
{
	if(_index_buffer + size == MAXBUFFER)
	{
		memset(_data_buffer, 0, MAXBUFFER);
		_index_buffer = 0;
	}

	memcpy(_data_buffer + _index_buffer, data, size);
	_index_buffer += size;
}

int CComm::GetReadData(char* buffer, int size)
{
	if(_index_buffer > size)
		return -1;

	memcpy(buffer, _data_buffer, _index_buffer);
	memset(_data_buffer, 0, MAXBUFFER);

	int len =_index_buffer;
	_index_buffer = 0;

	return len;
}

// set up comport
void CComm::SetComport(int port, DWORD rate, BYTE byteSize, BYTE stop, BYTE parity)
{
	bPort=port;
	dwBaudRate=rate;
	bByteSize=byteSize;
	bStopBits=stop;
	bParity=parity;
}

void CComm::SetXonOff(BOOL chk)
{
	fXonXoff=chk;
}

void CComm::SetDtrRts(BYTE chk)
{
	bFlowCtrl=chk;
}

// create comport information
// set up after doing SetComport()->SetXonOff()->SetDtrRts()
BOOL CComm::CreateCommInfo()
{
	osWrite.Offset=0;
	osWrite.OffsetHigh=0;
	osRead.Offset=0;
	osRead.OffsetHigh=0;

	// creat EVENT. manual reset event, initial no-signal condition
	osRead.hEvent=CreateEvent(NULL, TRUE, FALSE, NULL);
	if(osRead.hEvent=NULL) {

		return FALSE;
	}
	osWrite.hEvent=CreateEvent(NULL, TRUE, FALSE, NULL);
	if(osWrite.hEvent=NULL) {
		CloseHandle(osRead.hEvent);

		return FALSE;
	}

	return TRUE;
}

// attemp to connect after opening comport
BOOL CComm::OpenComport()
{
	TCHAR szPort[15];
	BOOL fRetVal;
	COMMTIMEOUTS CommTimeOuts;
	if(bPort>10)
		_stprintf_s(szPort, _T("\\\\.\\COM%d"), bPort);
	else 
		_stprintf_s(szPort, _T("COM%d"), bPort);

	if((hComm=CreateFile(szPort, GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,//FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		NULL)) == (HANDLE)-1)
		return FALSE;
	else {
		// Set data exchange method in comport to char unit to a default
		SetCommMask(hComm, EV_RXCHAR | EV_BREAK | EV_CTS | EV_DSR | EV_ERR | EV_RING | EV_RLSD | EV_RXCHAR | EV_RXFLAG | EV_TXEMPTY);
		SetupComm(hComm, 4096, 4096);
		// clean completely in case device has wastes
		PurgeComm(hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
		CommTimeOuts.ReadIntervalTimeout=0xFFFFFFFF;
		CommTimeOuts.ReadTotalTimeoutMultiplier=0;
		CommTimeOuts.ReadTotalTimeoutConstant=1000;
		CommTimeOuts.WriteTotalTimeoutMultiplier=0;
		CommTimeOuts.WriteTotalTimeoutConstant=1000;
		SetCommTimeouts(hComm, &CommTimeOuts);
	}

	fRetVal = SetupConnection();
	if(fRetVal) {			// When connected, fRetVal is TRUE that
		fConnected=TRUE;	// tell connection has been succeeded
		s_updater = _beginthreadex(NULL, 0, CommWatchProc, this, 0, NULL);
		//AfxBeginThread((AFX_THREADPROC)CommWatchProc,(LPVOID)this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
	}
	else {
		fConnected=FALSE;
		_tprintf(_T("Problem occured with Communication Port."));
		CloseHandle(hComm);
	}

	return fRetVal;
}

//connect 'filed set comport' and acual port 
//must do SetupConnection befor CreateComport
BOOL CComm::SetupConnection()
{
	BOOL fRetVal;
	DCB dcb;
	dcb.DCBlength=sizeof(DCB);
	GetCommState(hComm, &dcb);		// receive dcb's default value.

	dcb.BaudRate=dwBaudRate;
	dcb.ByteSize=bByteSize;
	dcb.Parity=bParity;
	dcb.StopBits=bStopBits;
	
	fRetVal=SetCommState(hComm, &dcb);

	return fRetVal;
}

// read data from comprot
int CComm::ReadCommBlock(char* lpszBlock, int nMaxLength)
{
	BOOL fReadStat;
	COMSTAT ComStat;
	DWORD dwErrorFlags;
	DWORD dwLength;

	//only try to read number of bytes in queue
	ClearCommError(hComm, &dwErrorFlags, &ComStat);
	dwLength=min( (DWORD)nMaxLength, ComStat.cbInQue);
	if (dwLength>0) {
		fReadStat=ReadFile(hComm, lpszBlock, dwLength, &dwLength, &osRead);
		if (!fReadStat) {
			//Error Message
		}
	}
	return dwLength;
}

// remove comport completely
BOOL CComm::DestroyComm()
{
	if(fConnected) CloseConnection();
	CloseHandle(osRead.hEvent);
	CloseHandle(osWrite.hEvent);

	return TRUE;
}

// close connection
BOOL CComm::CloseConnection()
{
	//set connected flag to FALSE;
	fConnected=FALSE;
	WaitForSingleObject((HANDLE)s_updater, 3000);
	CloseHandle((HANDLE)s_updater);
	s_updater = 0;
	//disable event notification and wait for thread to halt

	if(hComm)
	{
		SetCommMask(hComm, 0);
		EscapeCommFunction(hComm, CLRDTR);
		PurgeComm(hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
		CloseHandle(hComm);
		hComm=NULL;
	}
	return TRUE;
}

BOOL CComm::WriteCommBlock(const char* lpByte, DWORD dwBytesToWrite)
{
	BOOL fWriteStat;
	DWORD dwBytesWritten;
	fWriteStat=WriteFile(hComm, lpByte, dwBytesToWrite, &dwBytesWritten, &osWrite);
	if(!fWriteStat) {
		//Error Message
	}

	return TRUE;
}

