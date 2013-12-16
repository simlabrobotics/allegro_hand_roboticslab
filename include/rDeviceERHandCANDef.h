#ifndef __RDEVICEERHANDCANDEF_H__
#define __RDEVICEERHANDCANDEF_H__

#define MAX_DOF 16

typedef struct tagDeviceMemory_ERHand
{
	int enc_actual[MAX_DOF];
	short pwm_actual[MAX_DOF];
	short pwm_demand[MAX_DOF];
} ERHand_DeviceMemory_t;

#endif // __RDEVICEERHANDCANDEF_H__
