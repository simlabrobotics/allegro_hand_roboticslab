/* RoboticsLab, Copyright 2008-2011 SimLab Co., Ltd. All rights reserved.
 *
 * This library is commercial and cannot be redistributed, and/or modified
 * WITHOUT ANY ALLOWANCE OR PERMISSION OF SimLab Co., LTD.
 */

#ifndef __RDEVICEERHANDENCODER_H__
#define __RDEVICEERHANDENCODER_H__

#include "rMath/rMath.h"
using namespace rMath;

#include "rDevice/rDeviceBase.h"

class rDeviceERHandEncoder : public RD_DEVICE_CLASS(Base)
{
public:
	RD_VERSION(1.0)
	RD_AUTHOR(SimLab)

public:
	RD_DECLARE_CTOR(ERHandEncoder);
	RD_DECLARE_DTOR(ERHandEncoder);

	RD_DECLARE_createDevice;
	RD_DECLARE_initDevice;
	RD_DECLARE_terminateDevice;
	RD_DECLARE_readDeviceValue;
	RD_DECLARE_updateReadValue;
	RD_DECLARE_command;

private:
	void printProps();

private:
	int _channel;
	TCHAR _systemDeviceName[MAX_PATH];
	rID _systemDevice;
	rID _joint;
	int* _deviceMem;

	float _q;
	float _q_old;
	float _q_filtered;
	float _q_filtered_old;
	float _q_filtered_render;
	float _q_filtered_render_old;
	
	int _enc;
	int _enc_offset;

	int _direction;
};
#endif