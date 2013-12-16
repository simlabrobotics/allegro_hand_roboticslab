/* RoboticsLab, Copyright 2008-2011 SimLab Co., Ltd. All rights reserved.
 *
 * This library is commercial and cannot be redistributed, and/or modified
 * WITHOUT ANY ALLOWANCE OR PERMISSION OF SimLab Co., LTD.
 */

#ifndef __RDEVICEERHANDSOFTINGCAN_H__
#define __RDEVICEERHANDSOFTINGCAN_H__

#include "rMath/rMath.h"
using namespace rMath;

#include "rDevice/rDeviceBase.h"

class rDeviceERHandSoftingCAN : public RD_DEVICE_CLASS(Base)
{
	RD_VERSION(1.0)
	RD_AUTHOR(SimLab)

public:
	RD_DECLARE_CTOR(ERHandSoftingCAN);
	RD_DECLARE_DTOR(ERHandSoftingCAN);

	RD_DECLARE_createDevice;
	RD_DECLARE_initDevice;
	RD_DECLARE_terminateDevice;
	RD_DECLARE_readDeviceValue;
	RD_DECLARE_writeDeviceValue;
	RD_DECLARE_updateWriteValue;
	RD_DECLARE_updateReadValue;
	RD_DECLARE_enable_set;
	RD_DECLARE_command;

private:
	void	initParams();

private:
	enum eServoMode
	{
		eServoMode_Position = 0,
		eServoMode_Velocity,
		eServoMode_Torque,
		eServoMode_None
	};

private:
	unsigned char* _memory;
	int _noj; // number of joints
	eServoMode _servoMode;

	bool _readyToServoOn;	// Whether homing is finished or not. 
							// Before homing is accomplished, 
							// Servo On command would not be executed.
	
	int		_jdof;			// dimension of joints
	int		_errCount;		// CAN I/O error count

	// control variables
	ERHand_DeviceMemory_t* _vars;

	friend unsigned int __stdcall ioThread(void* inst);
	uintptr_t			_ioThread;
	bool				_ioThreadRun;
	HANDLE				_ioEvent;
	HANDLE				_ioQuitEvent;

	int					_CAN_Ch;
};

#endif // __RDEVICEERHANDSOFTINGCAN_H__
