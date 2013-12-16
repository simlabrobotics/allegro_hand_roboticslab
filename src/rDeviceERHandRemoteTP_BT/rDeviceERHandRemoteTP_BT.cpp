/* RoboticsLab, Copyright 2008-2011 SimLab Co., Ltd. All rights reserved.
 *
 * This library is commercial and cannot be redistributed, and/or modified
 * WITHOUT ANY ALLOWANCE OR PERMISSION OF SimLab Co., LTD.
 */
#include "rDeviceERHandRemoteTP_BT.h"

unsigned int __stdcall ioThread(void* inst)
{
	rDeviceERHandRemoteTP_BT* rs232 = (rDeviceERHandRemoteTP_BT*)inst;

	rs232->_ccomm.OpenComport();

	while (rs232->_ioThreadRun)
	{
		if(!rs232->enabled())
			break;

		int size = rs232->_ccomm.GetReadData(rs232->_read_buffer, MAXBUFFER);

		rs232->_lock.lock();

		for(int i = 0; i < size; i++)
		{
			rs232->_data_queue.push(rs232->_read_buffer[i]);
		}

		rs232->_lock.unlock();

		Sleep(100);
	}

	return 0;
}

RD_IMPLE_FACTORY(ERHandRemoteTP_BT)

rDeviceERHandRemoteTP_BT::rDeviceERHandRemoteTP_BT() 
: _ioThreadRun(false)
{
	_packet_size = 0;
}

rDeviceERHandRemoteTP_BT::~rDeviceERHandRemoteTP_BT()
{
	//_ccomm.DestroyComm();
}

void rDeviceERHandRemoteTP_BT::onCreate(const rDeviceContext& rdc)
{
	RD_DEVICE_CLASS(Base)::onCreate(rdc);
	
	_ioEvent		= CreateEvent(NULL, FALSE, FALSE, NULL);
	_ioThread		= 0;
}

void rDeviceERHandRemoteTP_BT::onInit()
{
	RD_DEVICE_CLASS(Base)::onInit();
	
	const TCHAR* port = getProperty(_T("COM Port"));
	if(port)
		_port = _tstoi(port);

	const TCHAR* baudrate = getProperty(_T("Baud Rate"));
	if(baudrate)
		_baudrate = _tstoi(baudrate);

	const TCHAR* packet_size = getProperty(_T("packet_size"));
	if(packet_size)
		_packet_size = _tstoi(packet_size);
	else
		_packet_size = sizeof(MessageRemoteTP_t);

	enable(true);
}

void rDeviceERHandRemoteTP_BT::onTerminate()
{
	enable(false);

	//_ccomm.CloseConnection();
	//_ccomm.DestroyComm();

	RD_DEVICE_CLASS(Base)::onTerminate();
}

int rDeviceERHandRemoteTP_BT::readDeviceValue(void* buffer, int len, int port)
{
	if (len >= _packet_size && (int)_command_queue.size() > 0 && port == DEVPORT_FETCH_MESSAGE)
	{
		_lock.lock();

		MessageRemoteTP_t tmpRTP = _command_queue.front();
		_command_queue.pop();
		memcpy(buffer, &tmpRTP, _packet_size);

		_lock.unlock();
		// return read size.
		return _packet_size;
	}
	else
		return 0;
}

int rDeviceERHandRemoteTP_BT::writeDeviceValue(void* buffer, int len, int port)
{
	if(!enabled())
		return 0;

	if (len > 0 && len <= MAX_MESSAGE)
	{
		_lock.lock();
		
		memset(_tmp_buffer, 0, MAX_MESSAGE);
		memcpy(_tmp_buffer, buffer, len);

		_str_buffer2write += _tmp_buffer;

		_lock.unlock();
		
		return len;
	}
	else if(len > MAX_MESSAGE)
	{
		_lock.lock();
		
		memset(_tmp_buffer, 0, MAX_MESSAGE);
		memcpy(_tmp_buffer, buffer, MAX_MESSAGE);
		_str_buffer2write += _tmp_buffer;

		_lock.unlock();
		
		return MAX_MESSAGE;
	}
	else
		return 0;
}

void rDeviceERHandRemoteTP_BT::updateWriteValue(rTime time)
{
}

void rDeviceERHandRemoteTP_BT::updateReadValue(rTime time)
{
	while((int)_data_queue.size() >= _packet_size)
	{
		_lock.lock();

		memset(_msg_buffer, 0, MAX_MESSAGE);

		for(int i = 0; i < _packet_size; i++)
		{
			_msg_buffer[i] = _data_queue.front();
			_data_queue.pop();
		}

		MessageRemoteTP_t msgRTP;
		memcpy(&msgRTP, _msg_buffer, _packet_size);
		_command_queue.push(msgRTP);

		_lock.unlock();
	}
}

int rDeviceERHandRemoteTP_BT::command(int cmd, int arg, void* udata, int port)
{
	switch(cmd)
	{
	case CMD_CONNECT:
		_ccomm.SetComport(_port, _baudrate, 8, 0, 0);		//port, baudrate, databit, stopbit, paritybit
		_ccomm.CreateCommInfo();
		break;

	case CMD_DISCONNECT:
		_ccomm.DestroyComm();
		break;

	}

	return 0;
}

void rDeviceERHandRemoteTP_BT::enable(bool b)
{
	rDeviceBase::enable(b);

	if (b && !_ioThreadRun)
	{
		command(CMD_CONNECT);

		_ioThreadRun = true;
		_ioThread = _beginthreadex(NULL, 0, ioThread, (void*)this, 0, NULL);
	}
	else if(!b && _ioThreadRun)
	{
		_ioThreadRun = false;
		WaitForSingleObject((HANDLE)_ioThread, WAITING_TIME);
        CloseHandle((HANDLE)_ioThread);
        _ioThread = 0;

		command(CMD_DISCONNECT);		
	}
}

const TCHAR* rDeviceERHandRemoteTP_BT::author() const
{
	return _T("SimLab");
}

const TCHAR* rDeviceERHandRemoteTP_BT::version() const
{
	return _T("1.0");
}