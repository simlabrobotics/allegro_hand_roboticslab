/* RoboticsLab, Copyright 2008-2011 SimLab Co., Ltd. All rights reserved.
 *
 * This library is commercial and cannot be redistributed, and/or modified
 * WITHOUT ANY ALLOWANCE OR PERMISSION OF SimLab Co., LTD.
 */
#include <process.h>
#include "rDeviceERHandCANDef.h"
#include "rDeviceERHandCANCmd.h"
#include "rDeviceERHandNICAN.h"
#include "canAPI.h"
#include "canDef.h"

static unsigned int __stdcall ioThread(void* inst)
{
	rDeviceERHandNICAN* devCAN = (rDeviceERHandNICAN*)inst;

	HANDLE hEvents[2];
	int eventIdx;
	char id_des;
	char id_cmd;
	char id_src;
	int len;
	unsigned char data[8];
	unsigned char data_return = 0;
    
    // Configure events to be listened on
    hEvents[0] = devCAN->_ioQuitEvent;
	hEvents[1] = devCAN->_ioEvent;
	

	while (devCAN->_ioThreadRun)
	{
		eventIdx = WaitForMultipleObjects(sizeof(hEvents)/sizeof(hEvents[0]), hEvents, FALSE, 300); 

		switch (eventIdx)
		{
		case 0:
			return 0;//break;

		case 1:
			{
				while (0 == CANAPI::get_message(devCAN->_CAN_Ch, &id_cmd, &id_src, &id_des, &len, data, FALSE))
				{
					switch (id_cmd)
					{
					case ID_CMD_QUERY_CONTROL_DATA:
						{
							if (id_src >= ID_DEVICE_SUB_01 && id_src <= ID_DEVICE_SUB_04)
							{
								devCAN->_lock.lock();
								{
									devCAN->_vars->enc_actual[(id_src-ID_DEVICE_SUB_01)*4 + 0] = (int)(data[0] | (data[1] << 8));
									devCAN->_vars->enc_actual[(id_src-ID_DEVICE_SUB_01)*4 + 1] = (int)(data[2] | (data[3] << 8));
									devCAN->_vars->enc_actual[(id_src-ID_DEVICE_SUB_01)*4 + 2] = (int)(data[4] | (data[5] << 8));
									devCAN->_vars->enc_actual[(id_src-ID_DEVICE_SUB_01)*4 + 3] = (int)(data[6] | (data[7] << 8));
									data_return |= (0x01 << (id_src-ID_DEVICE_SUB_01));
								}
								devCAN->_lock.unlock();
							}
							if (data_return == (0x01 | 0x02 | 0x04 | 0x08))
							{
								devCAN->_lock.lock();
								{
									// send torques
									for (int i=0; i<4;i++)
									{
										CANAPI::write_current(devCAN->_CAN_Ch, i, &devCAN->_vars->pwm_demand[4*i]);
										for(int k=0; k<100000; k++);
									}
								}
								devCAN->_lock.unlock();

								data_return = 0;
							}
						}
						break;
					}
				}
			}
			break;
		}
	}

	return 0;
}

RD_IMPLE_FACTORY(ERHandNICAN)

rDeviceERHandNICAN::rDeviceERHandNICAN() 
: _jdof(-1)
, _errCount(0)
, _ioThread(0)
, _ioThreadRun(false)
, _ioEvent(NULL)
, _ioQuitEvent(NULL)
{
	_enabled = false;
	_vars = NULL;
	_CAN_Ch = 0;
}


rDeviceERHandNICAN::~rDeviceERHandNICAN()
{
	CloseHandle(_ioEvent);
	CloseHandle(_ioQuitEvent);
}

void rDeviceERHandNICAN::onCreate(const rDeviceContext& rdc)
{
	RD_DEVICE_CLASS(Base)::onCreate(rdc);

	_ioEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	_ioQuitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	const TCHAR* prop = getProperty(_T("CAN_Ch"));
	if (prop)
		_CAN_Ch = _tstoi(prop);
}

void rDeviceERHandNICAN::onInit()
{
	RD_DEVICE_CLASS(Base)::onInit();

	TCHAR device_mem_name[256];
	_stprintf(device_mem_name, _T("%s::%s"), _rdc.m_robotname, name);
	_vars = (ERHand_DeviceMemory_t*)_rdc.m_deviceAPI->getMemory(device_mem_name);
	//_vars = (ERHand_DeviceMemory_t*)_rdc.m_deviceAPI->getMemory(name);
}

void rDeviceERHandNICAN::onTerminate()
{
	enable(false);
	RD_DEVICE_CLASS(Base)::onTerminate();
}

int rDeviceERHandNICAN::readDeviceValue(void* buffer, int len, int port)
{
	if (!_vars)
		return 0;

	if (len == sizeof(ERHand_DeviceMemory_t))
	{
		_lock.lock();
		memcpy(buffer, _vars, sizeof(ERHand_DeviceMemory_t));
		_lock.unlock();
		return sizeof(ERHand_DeviceMemory_t);
	}
	else
		return 0;
}

int rDeviceERHandNICAN::writeDeviceValue(void* buffer, int len, int port)
{
	if (!_vars)
		return 0;

	if (len == sizeof(ERHand_DeviceMemory_t))
	{
		_lock.lock();
		memcpy(_vars, buffer, sizeof(ERHand_DeviceMemory_t));
		_lock.unlock();
		return sizeof(ERHand_DeviceMemory_t);
	}
	else
		return 0;
}

void rDeviceERHandNICAN::updateWriteValue(rTime time)
{
	if (!_vars)
		return;

	//_lock.lock();
	//{
	//	// send torques
	//	for (int i=0; i<4;i++)
	//	{
	//		CANAPI::write_current(_CAN_Ch, i, &_vars->pwm_demand[4*i]);
	//		for(int k=0; k<100000; k++);
	//	}
	//}
	//_lock.unlock();
}

void rDeviceERHandNICAN::updateReadValue(rTime time)
{
	if (!_vars)
		return;

	SetEvent(_ioEvent);
}

void rDeviceERHandNICAN::enable(bool b)
{
	int ret;

	if (b == rDeviceBase::enabled())
		return;

	rDeviceBase::enable(b);

	if (b)
	{
		printf(">CAN: open\n");
		ret = CANAPI::command_can_open(_CAN_Ch);
		if(ret < 0)
		{
			printf("ERROR command_canopen !!! \n");
			_enabled = false;
			return;
		}

//		ret = CANAPI::command_can_reset(_CAN_Ch);
//		if(ret < 0)
//		{
//			printf("ERROR command_can_reset !!! \n");
//			CANAPI::command_can_close();
//			_enabled = false;
//			return;
//		}

		_ioThreadRun = true;
		_ioThread = _beginthreadex(NULL, 0, ioThread, (void*)this, 0, NULL);
		printf(">CAN: starts listening CAN frames\n");

		printf(">CAN: system init\n");
		ret = CANAPI::command_can_sys_init(_CAN_Ch, 3/*msec*/);
		if(ret < 0)
		{
			printf("ERROR command_can_sys_init !!! \n");
			CANAPI::command_can_close(_CAN_Ch);
			_enabled = false;
			return;
		}

		printf(">CAN: start periodic communication\n");
		ret = CANAPI::command_can_start(_CAN_Ch);
		if(ret < 0)
		{
			printf("ERROR command_can_start !!! \n");
			CANAPI::command_can_stop(_CAN_Ch);
			CANAPI::command_can_close(_CAN_Ch);
			_enabled = false;
			return;
		}
	}
	else
	{
		printf(">CAN: stop periodic communication\n");
		ret = CANAPI::command_can_stop(_CAN_Ch);
		if(ret < 0)
		{
			printf("ERROR command_can_stop !!! \n");
		}

		printf(">CAN: stoped listening CAN frames\n");
		_ioThreadRun = false;
		//SetEvent(_ioEvent);
		SetEvent(_ioQuitEvent);
		WaitForSingleObject((HANDLE)_ioThread, INFINITE);
        CloseHandle((HANDLE)_ioThread);
        _ioThread = 0;
		
		printf(">CAN: close\n");
		ret = CANAPI::command_can_close(_CAN_Ch);
		if(ret < 0) printf("ERROR command_canclose !!! \n");
	}
}

int rDeviceERHandNICAN::command(int cmd, int arg, void* udata, int port)
{
	switch (cmd)
	{
	case CAN_CMD_ENABLE:
		enable(true);
		break;
	
	case CAN_CMD_DISABLE:
		enable(false);
		break;

	case CAN_CMD_INIT_SYS:
		{
			if (port >= 0 && port < _jdof)
			{
				printf(">CAN: system init(%d)\n", port);
				//if (0 != CANAPI::command_system_init(_bus, ID_DEVICE_01 + port))
					printf("\tfailed.\n");
			}
			else if (port == _jdof)
			{
				printf(">CAN: system init\n");
				//if (0 != CANAPI::command_system_init(_bus, ID_DEVICE_ALL))
					printf("\tfailed.\n");
			}
		}
		break;

	case CAN_CMD_RESET_ENC:
		{
			if (port >= 0 && port < _jdof)
			{
				printf(">CAN: position init(%d)\n", port);
				//if (0 != CANAPI::command_position_init(_bus, ID_DEVICE_01 + port))
					printf("\tfailed.\n");
			}
			else if (port == _jdof)
			{
				printf(">CAN: position init\n");
				//if (0 != CANAPI::command_position_init(_bus, ID_DEVICE_ALL))
					printf("\tfailed.\n");
			}
		}
		break;

	case CAN_CMD_MOTOR_ON:
		{
			if (port >= 0 && port < _jdof)
			{
				printf(">CAN: motor on(%d)\n", port);
				//if (0 != CANAPI::command_motor_on(_bus, ID_DEVICE_01 + port))
					printf("\tfailed.\n");
			}
			else if (port == _jdof)
			{
				printf(">CAN: motor on\n");
				//if (0 != CANAPI::command_motor_on(_bus, ID_DEVICE_ALL))
					printf("\tfailed.\n");
			}
		}
		break;

	case CAN_CMD_MOTOR_OFF:
		{
			if (port >= 0 && port < _jdof)
			{
				printf(">CAN: motor off(%d)\n", port);
				//if (0 != CANAPI::command_motor_off(_bus, ID_DEVICE_01 + port))
					printf("\tfailed.\n");
			}
			else if (port == _jdof)
			{
				printf(">CAN: motor off\n");
				//if (0 != CANAPI::command_motor_off(_bus, ID_DEVICE_ALL))
					printf("\tfailed.\n");
			}
		}
		break;

	case CAN_CMD_REQ_SEND_POS:
		{
			if (port >= 0 && port < _jdof)
			{
				printf(">CAN: request send position(%d)\n", port);
				//if (0 != CANAPI::command_request_send_pos_data(_bus, ID_DEVICE_01 + port))
					printf("\tfailed.\n");
			}
			else if (port == _jdof)
			{
				printf(">CAN: request send position\n");
				//if (0 != CANAPI::command_request_send_pos_data(_bus, ID_DEVICE_ALL))
					printf("\tfailed.\n");
			}
		}
		break;

	case CAN_CMD_CANCEL_SEND_POS:
		{
			if (port >= 0 && port < _jdof)
			{
				printf(">CAN: cancel send position(%d)\n", port);
				//if (0 != CANAPI::command_request_nosend_pos_data(_bus, ID_DEVICE_01 + port))
					printf("\tfailed.\n");
			}
			else if (port == _jdof)
			{
				printf(">CAN: cancel send position\n");
				//if (0 != CANAPI::command_request_nosend_pos_data(_bus, ID_DEVICE_ALL))
					printf("\tfailed.\n");
			}
		}
		break;

	case CAN_CMD_SET_PROP:
		{
			return -1; // failed to set property
		}
		break;

	case CAN_CMD_GET_PROP:
		{
			return -1; // failed to get property
		}
		break;

	case CAN_CMD_UPDATE_READ:
		updateWriteValue(0);
		break;

	case CAN_CMD_UPDATE_WRITE:
		updateReadValue(0);
		break;
	}
	return 0;
}

void rDeviceERHandNICAN::initParams()
{
	const TCHAR* prop;

	prop = getProperty(_T("JDOF"));
	if (prop)
		_jdof = _tstoi(prop);

	_errCount = 0;
}
