/* RoboticsLab, Copyright 2008-2011 SimLab Co., Ltd. All rights reserved.
 *
 * This library is commercial and cannot be redistributed, and/or modified
 * WITHOUT ANY ALLOWANCE OR PERMISSION OF SimLab Co., LTD.
 */

#ifndef __RDEVICEERHANDMOTOR_H__
#define __RDEVICEERHANDMOTOR_H__

#include "rMath/rMath.h"
using namespace rMath;

#include "rDevice/rDeviceBase.h"

class rDeviceERHandMotor : public RD_DEVICE_CLASS(Base)
{
public:
	RD_VERSION(1.0)
	RD_AUTHOR(SimLab)

public:
	RD_DECLARE_CTOR(ERHandMotor);
	RD_DECLARE_DTOR(ERHandMotor);

	RD_DECLARE_createDevice;
	RD_DECLARE_initDevice;
	RD_DECLARE_terminateDevice;
	RD_DECLARE_writeDeviceValue;
	RD_DECLARE_monitorDeviceValue;
	RD_DECLARE_updateWriteValue;

private:
	void printProps();

private:
	int _channel;
	TCHAR _systemDeviceName[MAX_PATH];
	rID _systemDevice;
	rID _joint;
	short* _deviceMem;

	float _tau_des; // user input desired torque.
	float _cur_des; // desired current limited by current lower and upper value.

	float _current_upper;
	float _current_lower;
	float _torque_const;
	float _torque_const_inv;
	float _torque_scale;
	float _gear_ratio;
	float _gear_ratio_inv;
	int _direction;
};
#endif