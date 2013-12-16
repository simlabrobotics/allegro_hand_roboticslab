/* RoboticsLab, Copyright 2008-2011 SimLab Co., Ltd. All rights reserved.
 *
 * This library is commercial and cannot be redistributed, and/or modified
 * WITHOUT ANY ALLOWANCE OR PERMISSION OF SimLab Co., LTD.
 */
#include "rDeviceERHandRemoteTP_TCPIP.h"

rDeviceERHandRemoteTP_TCPIP::NetworkServer::NetworkServer(rDeviceERHandRemoteTP_TCPIP* pDevice)
: _pDeviceServer(pDevice)
{	
}
rDeviceERHandRemoteTP_TCPIP::NetworkServer::~NetworkServer()
{
}

void rDeviceERHandRemoteTP_TCPIP::NetworkServer::onAccept(int id)
{
	TCHAR msg_buffer[MAX_MESSAGE_SIZE];
	_stprintf_s(msg_buffer, _T("New client is accepted.. id: %d"), id);
	_tprintf(msg_buffer);
	
	kaiSocket* sock = findClient(id);
	if (sock)
		clients.insert(std::pair<int, kaiSocket*>(id, sock));

	/*kaiMsg msg_nty;
	msg_nty.allocateMemory();
	msg_nty.reset();
	msg_nty.begin();
	msg_nty.id(CTS_MSG_NTY_MODE);
	msg_nty << _pDeviceServer->_modeDriving;
	msg_nty.end();
	sock->send(msg_nty);*/
	
}

void rDeviceERHandRemoteTP_TCPIP::NetworkServer::onClose(int id)
{
	TCHAR msg_buffer[MAX_MESSAGE_SIZE];
	_stprintf_s(msg_buffer, _T("Client is closed.. id: %d"), id);
	_tprintf(msg_buffer);

	clients.erase(id);
}

void rDeviceERHandRemoteTP_TCPIP::NetworkServer::onMessage(int id, kaiMsg& msg)
{
	kaiSocket* client = findClient(id);
	
	int size = msg.size();

	switch(msg.id())
	{
	case RTP_TYPE_COMMAND_EVENT:
		{
			MessageRemoteTP_t message;

			message.type	= (unsigned char)msg.id();
			message.size	= (unsigned char)msg.size();
			msg.begin();
			msg >> message.cmd;
			msg >> message.data;

			_pDeviceServer->_lock.lock();
			_pDeviceServer->_msg_queue_cmd.push(message);
			_pDeviceServer->_lock.unlock();
		}
		break;

	default:
		break;
	}
}

// Device
RD_IMPLE_FACTORY(ERHandRemoteTP_TCPIP)

rDeviceERHandRemoteTP_TCPIP::rDeviceERHandRemoteTP_TCPIP() 
: _pNetworkServer(NULL)
{
	userType = RD_TYPE_USER;
}

rDeviceERHandRemoteTP_TCPIP::~rDeviceERHandRemoteTP_TCPIP()
{
	if(_pNetworkServer)
		delete _pNetworkServer;
}

void rDeviceERHandRemoteTP_TCPIP::onCreate(const rDeviceContext& rdc)
{
	RD_DEVICE_CLASS(Base)::onCreate(rdc);
	// add your code for creating your own device. 

	kaiInitialize();

	_port_client = DEFAULT_PORT_NUM;
	const TCHAR* prop = getProperty(_T("port_client"));
	if (prop)
		_port_client = _tstoi(prop);
}

void rDeviceERHandRemoteTP_TCPIP::onInit()
{
	RD_DEVICE_CLASS(Base)::onInit();
	// add code for initializing your device. 

	_sz2read_message	= sizeof(_msg_progress);

	command(CMD_SERVER_RUN);
}

void rDeviceERHandRemoteTP_TCPIP::onTerminate()
{
	command(CMD_SERVER_STOP);

	RD_DEVICE_CLASS(Base)::onTerminate();
}

int rDeviceERHandRemoteTP_TCPIP::readDeviceValue(void* buffer, int len, int port)
{
	if (len >= _sz2read_message && port == DEVPORT_FETCH_MESSAGE && !_msg_queue_cmd.empty())
	{	
		_lock.lock();
		// add	your code that fill value with your device parameters. 
		_msg_progress = _msg_queue_cmd.front();
		_msg_queue_cmd.pop();
		memcpy(buffer, &_msg_progress, _sz2read_message);
		_lock.unlock();
		// return read size.
		return _sz2read_message;
	}
	else
		return 0;	
}

int rDeviceERHandRemoteTP_TCPIP::writeDeviceValue(void* buffer, int len, int port)
{
	if (len > 0)
	{
		_lock.lock();
		// add your code that fill your device parameters with value. 
		_lock.unlock();
		// return write size.
		return 0;
	}
	else
		return 0;
}

int rDeviceERHandRemoteTP_TCPIP::monitorDeviceValue(void* buffer, int len, int port)
{
	if (len > 0)
	{	
		_lock.lock();
		// add	your code that fill value with values that you want to monitor. this method is used in data acquisition module.
		_lock.unlock();
		// return read size.
		return 0;
	}
	else
		return 0;
}

void rDeviceERHandRemoteTP_TCPIP::importDevice(rTime time, void* mem)
{
	_lock.lock();
	// add your code that imports necessary values for your device from the world.
	_lock.unlock();

	// add implementation for your device.
}

void rDeviceERHandRemoteTP_TCPIP::exportDevice(rTime time, void* mem)
{
	// add implementation for your device. 

	_lock.lock();
	// add your code that exports your device values to the world. 
	_lock.unlock();
}

void rDeviceERHandRemoteTP_TCPIP::updateWriteValue(rTime time)
{
	//_lock.lock();

	//_lock.unlock();
}

void rDeviceERHandRemoteTP_TCPIP::updateReadValue(rTime time)
{
	//_lock.lock();

	//_lock.unlock();
}

int rDeviceERHandRemoteTP_TCPIP::command(int cmd, int arg, void* udata, int port)
{
	int res = CMD_RESULT_FAIL;

	switch(cmd)
	{
	case CMD_SERVER_RUN:
		{
			if(_pNetworkServer)
				delete _pNetworkServer;

			_pNetworkServer = new NetworkServer(this);
			_pNetworkServer->create(_port_client);
			_pNetworkServer->run(true);
			res = CMD_RESULT_SUCCESS;
		}
		break;

	case CMD_SERVER_STOP:
		{
			_pNetworkServer->stop();
			res = CMD_RESULT_SUCCESS;
		}
		break;

	case CMD_CLEAR_QUEUE:
		{
			for(unsigned int i = 0; i < _msg_queue_cmd.size(); i++)
				_msg_queue_cmd.pop();

			res = CMD_RESULT_SUCCESS;
		}
		break;
	}

	return res;
}

const TCHAR* rDeviceERHandRemoteTP_TCPIP::author() const
{
	return _T("your name");
}

const TCHAR* rDeviceERHandRemoteTP_TCPIP::version() const
{
	return _T("1.0");
}