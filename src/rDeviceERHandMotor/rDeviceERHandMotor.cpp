/* RoboticsLab, Copyright 2008-2011 SimLab Co., Ltd. All rights reserved.
 *
 * This library is commercial and cannot be redistributed, and/or modified
 * WITHOUT ANY ALLOWANCE OR PERMISSION OF SimLab Co., LTD.
 */
#include "rDeviceERHandMotor.h"
#include "rDeviceERHandCANDef.h"
#include <float.h>

RD_IMPLE_FACTORY(ERHandMotor)

rDeviceERHandMotor::rDeviceERHandMotor()
: _channel(-1)
, _systemDevice(INVALID_RID)
, _joint(INVALID_RID)
, _deviceMem(NULL)
, _tau_des(0.0f)
, _cur_des(0.0f)
, _torque_const(1.0f)
, _torque_const_inv(1.0f)
, _torque_scale(1.0f)
, _gear_ratio(1.0f)
, _gear_ratio_inv(1.0f)
, _current_upper(FLT_MAX)
, _current_lower(-FLT_MAX)
, _direction(1)
{
}

rDeviceERHandMotor::~rDeviceERHandMotor()
{
}

void rDeviceERHandMotor::onCreate(const rDeviceContext& rdc)
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

	prop = getProperty(_T("torque constant"));
	if (prop)
	{
		_torque_const = (float)_tstof(prop);
		_torque_const_inv = 1.0f / _torque_const;
	}

	prop = getProperty(_T("torque scale"));
	if (prop)
	{
		_torque_scale = (float)_tstof(prop);
	}

	prop = getProperty(_T("gear ratio"));
	if (prop)
	{
		_gear_ratio = (float)_tstof(prop);
		_gear_ratio_inv = 1.0f / _gear_ratio;
	}

	prop = getProperty(_T("current upper"));
	if (prop)
		_current_upper = (float)_tstof(prop);

	prop = getProperty(_T("current lower"));
	if (prop)
		_current_lower = (float)_tstof(prop);

	prop = getProperty(_T("direction"));
	if (prop && prop[0] == _T('-'))
		_direction = -1;
	else
		_direction = 1;
}

void rDeviceERHandMotor::onInit()
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
	_deviceMem = &(mem->pwm_demand[_channel]);

	// print out device properties
	printProps();
}

void rDeviceERHandMotor::onTerminate()
{
	RD_DEVICE_CLASS(Base)::onTerminate();
}

int rDeviceERHandMotor::writeDeviceValue(void* buffer, int len, int port)
{
	if (len == sizeof(float))
	{	
		_lock.lock();
		_tau_des = *(float*)buffer;
		_lock.unlock();
		return sizeof(float);
	}
	else
		return 0;
}

int rDeviceERHandMotor::monitorDeviceValue(void* buffer, int len, int port)
{
	if (len == sizeof(float))
	{	
		_lock.lock();
		*(float*)buffer = _cur_des;
		_lock.unlock();
		return sizeof(float);
	}
	else
		return 0;
}

void rDeviceERHandMotor::updateWriteValue(rTime time)
{
	if (_systemDevice == INVALID_RID) return;

	_lock.lock();
	{
		_cur_des = _tau_des * _torque_const_inv * _gear_ratio_inv * _direction;
		_cur_des = RD_BOUND(_cur_des, _current_lower, _current_upper);
		////////////////////////////////////////////////////
		// XXX : DISABLE TORQUE INPUT
		////////////////////////////////////////////////////
		//_cur_des = 0.0f;
		//if (_channel == 15)
		//	_cur_des = 50.0f / 800.0f;
		////////////////////////////////////////////////////
		*_deviceMem = (short)(_cur_des*_torque_scale*800.0f);
	}
	_lock.unlock();
}

void rDeviceERHandMotor::printProps()
{
	printf("---------- motor(%d) -----------\n", _channel);
	printf("gear ratio = %.3f\n", _gear_ratio);
	printf("torque constant = %.3f (Nm/A)\n", _torque_const);
	printf("current lower = %.3f (A)\n", _current_lower);
	printf("current upper = %.3f (A)\n", _current_upper);
	printf("direction = %s\n", (_direction > 0 ? "+" : "-"));
	//printf("-------------------------------\n");
}
