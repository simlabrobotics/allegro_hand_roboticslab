/* RoboticsLab, Copyright 2008-2011 SimLab Co., Ltd. All rights reserved.
 *
 * This library is commercial and cannot be redistributed, and/or modified
 * WITHOUT ANY ALLOWANCE OR PERMISSION OF SimLab Co., LTD.
 */

#ifndef __RDEVICECOMMTCPIPSERVER_H__
#define __RDEVICECOMMTCPIPSERVER_H__

#include "rMath/rMath.h"
using namespace rMath;

#include "kai2/kai.h"

#include "rDeviceERHandRemoteCmd.h"

#include "rDevice/rDeviceBase.h"

typedef std::queue<MessageRemoteTP_t>				queue_message;

#define MAX_SIZE_MESSAGE_BUFFER							100	// in byte
#define MAX_MESSAGE_SIZE								1000

class rDeviceERHandRemoteTP_TCPIP : public RD_DEVICE_CLASS(Base)
{
public:
	class NetworkServer : public kaiServer
	{
		rDeviceERHandRemoteTP_TCPIP*	_pDeviceServer;
	public:
		NetworkServer(rDeviceERHandRemoteTP_TCPIP* pDevice);
		~NetworkServer();
		virtual void onMessage(int id, kaiMsg& msg);
		virtual void onAccept(int id);
		virtual void onClose(int id);

		std::map<int, kaiSocket*> clients;
	};

	rDeviceERHandRemoteTP_TCPIP();
	~rDeviceERHandRemoteTP_TCPIP();

	virtual void			onCreate(const rDeviceContext& rdc);
	virtual void			onInit();
	virtual void			onTerminate();
	virtual int				readDeviceValue(void* buffer, int len, int port = 0);
	virtual int				writeDeviceValue(void* buffer, int len, int port = 0);
	virtual int				monitorDeviceValue(void* buffer, int len, int port = 0);
	virtual void			importDevice(rTime time, void* mem = NULL);
	virtual void			exportDevice(rTime time, void* mem = NULL);
	virtual void			updateWriteValue(rTime time);
	virtual void			updateReadValue(rTime time);
	virtual int				command(int cmd, int arg = 0, void* udata = NULL, int port = 0);
	virtual const TCHAR*	author() const;
	virtual const TCHAR*	version() const;

	// TODO: add your methods here.
	NetworkServer			*_pNetworkServer;
	queue_message			_msg_queue_cmd;
	MessageRemoteTP_t		_msg_progress;
private:
	short					_port_client;
	int						_sz2read_message;
};
#endif