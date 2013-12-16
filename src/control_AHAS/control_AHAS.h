/* RoboticsLab, Copyright 2008-2010 SimLab Co., Ltd. All rights reserved.
 *
 * This library is commercial and cannot be redistributed, and/or modified
 * WITHOUT ANY ALLOWANCE OR PERMISSION OF SimLab Co., LTD.
 */
#ifndef __CONTROL_AHAS_H__
#define __CONTROL_AHAS_H__

#include <list>
#include "rControlAlgorithm/rControlAlgorithm.h"
#include "BHand/BHand.h"
#include "rDeviceERHandRemoteCmd.h"

#define JDOF 16

class REXPORT control_AHAS : public rControlAlgorithmEx
{
public:
	control_AHAS(rDC rdc);
	~control_AHAS();

	virtual void init(int mode = 0);
	virtual void update(const rTime& t);
	virtual void setNominalSystem(const TCHAR* path, const TCHAR* aml, const HTransform& T0, const dVector& q0);
	virtual void setPeriod(const rTime& dT);
	virtual int command(const short& cmd, const int& arg = 0);
	virtual void datanames(vector<string_type>& names, int channel = -1);
	virtual void collect(vector<double>& data, int channel = -1);
	virtual void onSetInterestFrame(const TCHAR* name, const HTransform& T);

private:
	virtual void _estimate();
	virtual void _readDevices();
	virtual void _writeDevices();

	virtual void _reflect();
	virtual void _compute(const rTime& t);

	void _arrangeJointDevices();

	void _writeForceArrows();

	void _readMessage_TCPIP();
	void _readMessage_BT();

	void _demo_jointMovement();
	void _jointDirectionTest();

	void _servoOn();
	void _servoOff();


private:
	rTime				_cur_time;

	BHand*				_hand;
	bool				_is_left_hand;

	rID					_motor[16];
	rID					_enc[16];
	rID					_remoteTP_BT;
	rID					_remoteTP_TCPIP;

	dVector				_q;
	dVector				_qdot;
	dVector				_torque;

	dVector				_q_unfiltered;
	dVector				_q_raw_tick;

	int					_jdof;

	double				_x[4];
	double				_y[4];
	double				_z[4];

	double				_fx[4];
	double				_fy[4];
	double				_fz[4];

	double				_magnitude_f[4];

	double				_f_norm[4];

	double				_objectDisp[3];

	// for testing each motor
	int					_jid_test;
	bool				_test_mode;

	// for remote control
	MessageRemoteTP_t	_msgRTP;
	int					_sz2read_msgRTP;

	// for show off
	bool				_demo_mode;
	rTime				_demo_start_time;
	dVector				_demo_q_des;

	bool				_move_object;

	int					_enc_port;

	rHANDLE				_arrow0;
	float				_arrow_vector0[10];
	rHANDLE				_arrow1;
	float				_arrow_vector1[10];
	rHANDLE				_arrow2;
	float				_arrow_vector2[10];
	rHANDLE				_arrow3;
	float				_arrow_vector3[10];
};

#endif
