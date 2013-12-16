/* RoboticsLab, Copyright 2008-2011 SimLab Co., Ltd. All rights reserved.
 *
 * This library is commercial and cannot be redistributed, and/or modified
 * WITHOUT ANY ALLOWANCE OR PERMISSION OF SimLab Co., LTD.
 */

#ifndef __RDEVICERS232_H__
#define __RDEVICERS232_H__

#define MAX_MESSAGE		100
#define WAITING_TIME	3000//INFINITE

#include "rMath/rMath.h"
using namespace rMath;

#include "rDevice/rDeviceBase.h"
#include "Comm.h"
#include <queue>
#include "rDeviceERHandRemoteCmd.h"

typedef std::queue<unsigned char>	data_queue;
typedef std::queue<MessageRemoteTP_t>	command_queue;

class rDeviceERHandRemoteTP_BT : public RD_DEVICE_CLASS(Base)
{
public:
	rDeviceERHandRemoteTP_BT();
	~rDeviceERHandRemoteTP_BT();

	virtual void			onCreate(const rDeviceContext& rdc);
	virtual void			onInit();
	virtual void			onTerminate();
	virtual int				readDeviceValue(void* buffer, int len, int port = 0);
	virtual int				writeDeviceValue(void* buffer, int len, int port = 0);
	virtual void			updateWriteValue(rTime time);
	virtual void			updateReadValue(rTime time);
	virtual int				command(int cmd, int arg = 0, void* udata = NULL, int port = 0);
	virtual void			enable(bool b);
	virtual const TCHAR*	author() const;
	virtual const TCHAR*	version() const;

	// TODO: add your methods here.
private:
	CComm					_ccomm;
	uintptr_t				_ioThread;
	HANDLE					_ioEvent;
	bool					_ioThreadRun;

	UINT					_port;
	UINT					_baudrate;

	int						_packet_size;

	data_queue				_data_queue;
	char					_read_buffer[MAXBUFFER];

	std::string				_str_buffer2write;
	char					_tmp_buffer[MAX_MESSAGE + 1];

	int						_sz2write;

	command_queue			_command_queue;
	unsigned char			_msg_buffer[MAX_MESSAGE + 1];
	
	friend unsigned int __stdcall ioThread(void* inst);
};
#endif