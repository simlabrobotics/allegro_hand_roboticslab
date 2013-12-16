/* RoboticsLab, Copyright 2008-2011 SimLab Co., Ltd. All rights reserved.
 *
 * This library is commercial and cannot be redistributed, and/or modified
 * WITHOUT ANY ALLOWANCE OR PERMISSION OF SimLab Co., LTD.
 */
#include "rDeviceERHandEncoder.h"
#include "rDeviceERHandCANDef.h"
#include "rDeviceERHandCANCmd.h"

RD_IMPLE_FACTORY(ERHandEncoder)

rDeviceERHandEncoder::rDeviceERHandEncoder()
: _channel(-1)
, _systemDevice(INVALID_RID)
, _joint(INVALID_RID)
, _deviceMem(NULL)
, _q(0.0f)
, _q_old(0.0f)
, _q_filtered(0.0f)
, _q_filtered_old(0.0f)
, _q_filtered_render(0.0f)
, _q_filtered_render_old(0.0f)
, _enc(0)
, _enc_offset(0)
, _direction(1)
{
}

rDeviceERHandEncoder::~rDeviceERHandEncoder()
{
}

void rDeviceERHandEncoder::onCreate(const rDeviceContext& rdc)
{
	RD_DEVICE_CLASS(Base)::onCreate(rdc);
	
	const TCHAR* prop;
	
	// channel
	prop = getProperty(_T("channel"));
	if (prop)
		_channel = _tstoi(prop);

	// system device name
	prop = getProperty(_T("system device name"));
	if (prop)
		_tcscpy_s(_systemDeviceName, MAX_PATH, prop);
	else
		_systemDeviceName[0] = 0;

	prop = getProperty(_T("direction"));
	if (prop && prop[0] == _T('-'))
		_direction = -1;
	else
		_direction = 1;

	prop = getProperty(_T("offset"));
	if (prop)
		_enc_offset = _ttoi(prop);
	else
		_enc_offset = 0;
}

void rDeviceERHandEncoder::onInit()
{
	RD_DEVICE_CLASS(Base)::onInit();
	
	// find joint
	_joint = _rdc.m_deviceAPI->getJointID(_rdc.m_robotname, _rdc.m_nodename);

	// find system device and set device buffer
	TCHAR device_mem_name[256];
	_stprintf(device_mem_name, _T("%s::%s"), _rdc.m_robotname, _systemDeviceName);
	ERHand_DeviceMemory_t* mem = (ERHand_DeviceMemory_t*)(_rdc.m_deviceAPI->getMemory(device_mem_name));
	//ERHand_DeviceMemory_t* mem = (ERHand_DeviceMemory_t*)(_rdc.m_deviceAPI->getMemory(_systemDeviceName));
	_systemDevice = _rdc.m_deviceAPI->getDeviceID(_rdc.m_robotname, _systemDeviceName);
	_deviceMem = &(mem->enc_actual[_channel]);

	// print out device properties
	printProps();
}

void rDeviceERHandEncoder::onTerminate()
{
	RD_DEVICE_CLASS(Base)::onTerminate();
}

int rDeviceERHandEncoder::readDeviceValue(void* buffer, int len, int port)
{
	if (len >= sizeof(float))
	{	
		_lock.lock();

		//printf("port: %i\n",port);
		if (port==0) {
			*(float*)buffer = _q_filtered;
		}
		else if (port == 1) {
			*(float*)buffer = _q;
		}
		else {
			*(float*)buffer = _enc;
		}
		
		
		_lock.unlock();
		return sizeof(float);
	}
	else
		return 0;
}

void rDeviceERHandEncoder::updateReadValue(rTime time)
{
	if (_systemDevice == INVALID_RID) return;

	_lock.lock();
	{
		_enc = *_deviceMem * _direction;
		_q_old = _q;
		_q_filtered_old = _q_filtered;
		_q_filtered_render_old = _q_filtered_render;
		_q = (float)(_enc-32768-_enc_offset)*(333.3f/65536.0f)*(3.141592f/180.0f);

		// Two seperate fiters for Allegro Hand 1.0 and 2.0:
		// _q_filtered = (0.6f*_q_filtered_old) + (0.198f*_q_old) + (0.198f*_q); // for Allegro Hand 1.0
//.3
		_q_filtered = _q_filtered_old + 0.5*(_q - _q_filtered_old);						// for Allegro Hand 2.0
		_q_filtered_render = _q_filtered_render_old + 0.03*(_q - _q_filtered_render_old);	// for Allegro Hand 2.0

		if (_joint != INVALID_RID/* && _joint < 100*/)
			_rdc.m_deviceAPI->setJointPositionKinematic(_joint, &_q_filtered_render);
	}
	_lock.unlock();
}

int rDeviceERHandEncoder::command(int cmd, int arg, void* udata, int port)
{
	switch (cmd)
	{
	case CAN_CMD_RESET_ENC:
		{
			printf("offset[%d] = enc[%d]-32768 = %d\n", _channel, _channel, (_enc-32768));
		}
		break;
	}
	return 0;
}


void rDeviceERHandEncoder::printProps()
{
	printf("---------- encoder(%d) -----------\n", _channel);
	//printf("encoder lower limit = %d (ticks)\n", _encoder_lower);
	//printf("encoder upper limit = %d (ticks)\n", _encoder_upper);
	printf("offset = %d\n", _enc_offset);
	printf("direction = %s\n", (_direction > 0 ? "+" : "-"));
	//printf("-------------------------------\n");
}
