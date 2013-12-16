/* RoboticsLab, Copyright 2008-2010 SimLab Co., Ltd. All rights reserved.
 *
 * This library is commercial and cannot be redistributed, and/or modified
 * WITHOUT ANY ALLOWANCE OR PERMISSION OF SimLab Co., LTD.
 */
#include "control_AHAS.h"
#include "control_AHASCmd.h"

//#define DEBUG_COMM

control_AHAS::control_AHAS(rDC rdc) 
:rControlAlgorithmEx(rdc)
, _cur_time(0)
, _jdof(0)
, _jid_test(0)
, _test_mode(false)
, _demo_mode(false)
, _hand(NULL)
, _is_left_hand(false)
, _demo_start_time(0)
{
}

control_AHAS::~control_AHAS()
{
	if (_hand)
		delete _hand;
}

void control_AHAS::_servoOn()
{
}

void control_AHAS::_servoOff()
{
}

void control_AHAS::_arrangeJointDevices()
{
	for (int i=0; i<_jdof; i++)
	{
		TCHAR devname[32];

		// Initialize all motors and encoders (joint devices)
		_stprintf(devname, _T("motor%d"), i + 1);
		_motor[i] = findDevice(devname);

		_stprintf(devname, _T("enc%d"), i + 1);
		_enc[i] = findDevice(devname);
	}
}

void control_AHAS::init(int mode)
{
	// Left or Right Hand Controller? Get property from control_AHAS_X.dll
	const TCHAR* prop = NULL;
	prop = getProperty(_T("whichHand"));
	if (prop && _tcsicmp(prop, _T("right")) == 0)
	{
		_is_left_hand = false;
		_hand = bhCreateRightHand();
	}
	else
	{
		_is_left_hand = true;
		_hand = bhCreateLeftHand();
	}
	assert(_hand);

	// Control Loop Period
	_hand->SetTimeInterval(0.003);
	
	// Allegro Hand DOF = 16 (4x4)
	_jdof = JDOF;

	// Call Fn to find all motors and encoders
	_arrangeJointDevices();

	// Setup Bluetooth and TCP/IP Comm
	_remoteTP_BT	= findDevice(_T("RemoteTP_BT"));
	_remoteTP_TCPIP	= findDevice(_T("RemoteTP_TCPIP"));

	// Setup up Arror graphic devices to represent fingertip control forces
	_arrow0 = findDevice(_T("force_tip_0"));
	_arrow1 = findDevice(_T("force_tip_1"));
	_arrow2 = findDevice(_T("force_tip_2"));
	_arrow3 = findDevice(_T("force_tip_3"));

	
	// Initialize joint position, vel, and torque arrays 
	_q.resize(_jdof);
	_qdot.resize(_jdof);
	_torque.resize(_jdof);
	_demo_q_des.resize(_jdof);

	_q.zero();
	_qdot.zero();
	_torque.zero();
	_demo_q_des.zero();

	// Initialize (as 0) the x, y and z positions of the finger tips
	memset(_x, 0, sizeof(_x[0])*4);
	memset(_y, 0, sizeof(_y[0])*4);
	memset(_z, 0, sizeof(_z[0])*4);

	_sz2read_msgRTP = sizeof(MessageRemoteTP_t);

	printf("\n\nController: control_AHAS.cpp\nAlex Alspach <alexalspach@simlab.co.kr>\nMarch 13, 2013\n\n");

	printf("\n\nATTENTION:\n\nMake sure the sim/app are referencing an updated controller.\nYou can check by altering the following message.\nThe proper output path in Linker/General is\n..\\..\\..\\..\\..\\..\\bin\\controls\\$(ProjectName).dll\n\n");
	printf("Alter this message.\n\n\n\n\n\n\n\n\n");
}


void control_AHAS::update(const rTime& t)
{
	// Update Control with the current time
	_cur_time = t;
	rControlAlgorithm::update(t);
}

void control_AHAS::setNominalSystem(const TCHAR* path, const TCHAR* aml, const HTransform& T0, const dVector& q0)
{
}

void control_AHAS::setPeriod(const rTime& dT)
{
}

void control_AHAS::_readDevices()
{
	float val;
	for (int i=0; i<JDOF; i++)
	{
		if (_enc[i] != INVALID_RID)
		{
			// Read Hand encoders
			readDeviceValue(_enc[i], &val, 4, 0); // 0 = filtered (rad) 
			_q[i] = (float)val;


		}
	}

	// read message from bluetooth
	_readMessage_BT();

	//  message from TCP/IP
	_readMessage_TCPIP();

}

void control_AHAS::_writeDevices()
{
	float val;
	for (int i=0; i<JDOF; i++)
	{
		val = (float)_torque[i];
		if (_motor[i] != INVALID_RID)
		{
			// Write torque to Hand motors
			writeDeviceValue(_motor[i], &val, 4);
		}
	}

	_writeForceArrows();

}

void control_AHAS::_reflect()
{
}

void control_AHAS::_compute(const double& t)
{
	if (_hand)
	{
		if (_test_mode)
		{	
			_jointDirectionTest();
		}
		else
		{
			// ShowOff Finger Motions
			// Makes sinusoidal motions at each joint
			
			
			_hand->SetJointPosition(_q.array); // tell BHand library the current joint positions

			// This DEMO Mode moves the finger joints in a sinusoidal pattern to show motion
			if (_demo_mode)		_demo_jointMovement();

			_hand->UpdateControl(t);
			_hand->GetFKResult(_x, _y, _z);
			_hand->GetJointTorque(_torque.array);

			
		}
	}
}

void control_AHAS::_estimate()
{
}

int control_AHAS::command(const short& cmd, const int& arg)
{

	// Commands via TCP/IP, Bluetooth, Keyboard Command or AHAS Button Press

	// NOTE: IS IT POSSIBLE TO CHANGE THIS TO A CASE STATEMENT WHERE THE FIRST CASE SETS EVERYTHING FALSE THEN THE FOLLOWING SET THE PROPER FLAG TO TRUE??
	if (cmd == BH_TEST)
	{
		// If Test Mode is commanded
		_test_mode =	true;
		_demo_mode =	false;

		_jid_test++;
		if (_jid_test >= JDOF)
			_jid_test = 0;
	}
	else if (cmd == BH_SHOWOFF)
	{
		// If Demo Mode (Showoff Mode) is commanded
		_test_mode =	false;
		_demo_mode =	true;
		_demo_start_time = _cur_time;

		if (_hand)
			_hand->SetMotionType(eMotionType_JOINT_PD);
	}
	else
	{
		// for any other command 
		_test_mode =	false;
		_demo_mode =	false;

		switch (cmd)
		{
		case BH_NONE:
			{
				// Turns all servos off / no control mode specified
				if (_hand)
					_hand->SetMotionType(eMotionType_NONE);
			}
			break;

		case BH_HOME:
			{
				// Home positions (PD/Joint Space Control)
				if (_hand)
					_hand->SetMotionType(eMotionType_HOME);
			}
			break;

		case BH_READY:
			{
				// Ready position before grasping (task space control)
				if (_hand)
					_hand->SetMotionType(eMotionType_READY);
			}
			break;

		case BH_GRASP_3:
			{
				// Three finger task space grasp
				if (_hand)
					_hand->SetMotionType(eMotionType_GRASP_3);
			}
			break;

		case BH_GRASP_4:
			{
				// Four finger task space grasp
				if (_hand)
					_hand->SetMotionType(eMotionType_GRASP_4);
			}
			break;

		case BH_PINCH_IT:
			{
				// Two finger task space pinch (Index/Thumb)
				if (_hand)
					_hand->SetMotionType(eMotionType_PINCH_IT);
			}
			break;

		case BH_PINCH_MT:
			{
				// Two finger task space pinch (Middle/Thumb)
				if (_hand)
					_hand->SetMotionType(eMotionType_PINCH_MT);
			}
			break;

		case BH_ENVELOP:
			{
				// Filly enveloping grasp (Command constant joint torque)
				if (_hand)
					_hand->SetMotionType(eMotionType_ENVELOP);
			}
			break;

		case BH_GRAVITY_COMP:
			{
				if (_hand)
					_hand->SetMotionType(eMotionType_GRAVITY_COMP);
			}
			break;

		case RESERVED_CMD_SERVO_ON:
			// Turn all servos off
			_servoOn();
			break;

		case RESERVED_CMD_SERVO_OFF:
			// Turn all servos on
			_servoOff();
			break;

		default:
			break;
		}
	}

	return 0;
}

void control_AHAS::datanames(vector<string_type>& names, int channel)
{
	switch (channel)
	{
		// (For RoboticsLab) Initialize plots for joint torques and positions of each finger
		case ePlotManipulator::PLOT_TORQUE:
			{
				TCHAR dname[64];
				for (int i=0; i<NOF; i++) {
					for (int j=0; j<NOJ; j++) {
						_stprintf(dname, _T("tau_des[%d][%d]"), i, j);
						names.push_back(dname);
					}
				}
			}
			break;

		case 252:
			{
				TCHAR dname[64];
				for (int i=0; i<NOF; i++) {
					_stprintf(dname, _T("Finger %d"), i);
					names.push_back(dname);
				}
			}
			break; 
	}
}

void control_AHAS::collect(vector<double>& data, int channel)
{
	switch (channel)
	{
	case ePlotManipulator::PLOT_TORQUE:
		{
			// collect torque data for plotting
			for (int i=0; i<NOF*NOJ; i++)
				data.push_back(_torque[i]);
		}
		break;

	case 249:
		{
			// collect Finger 0 (index) positions data for plotting
			data.push_back(_q[0]);
			data.push_back(_q[1]);
			data.push_back(_q[2]);
			data.push_back(_q[3]);
		}
		break;

	case 248:
		{
			// collect Finger 1 (middle) positions data for plotting
			data.push_back(_q[4]);
			data.push_back(_q[5]);
			data.push_back(_q[6]);
			data.push_back(_q[7]);
		}
		break;

	case 247:
		{
			// collect Finger 2 (pinky) positions data for plotting
			data.push_back(_q[8]);
			data.push_back(_q[9]);
			data.push_back(_q[10]);
			data.push_back(_q[11]);
		}
		break;

	case 246:
		{
			// collect Finger 3 (thumb) positions data for plotting
			data.push_back(_q[12]);
			data.push_back(_q[13]);
			data.push_back(_q[14]);
			data.push_back(_q[15]);
		}
		break;

	case 255:
		{
			// collect X position of all four fingertips for plotting
			data.push_back(_x[0]);
			data.push_back(_x[1]);
			data.push_back(_x[2]);
			data.push_back(_x[3]);
		}
		break;

	case 254:
		{
			// collect Y position of all four fingertips for plotting
			data.push_back(_y[0]);
			data.push_back(_y[1]);
			data.push_back(_y[2]);
			data.push_back(_y[3]);
		}
		break;

	case 253:
		{
			// collect Z position of all four fingertips for plotting
			data.push_back(_z[0]);
			data.push_back(_z[1]);
			data.push_back(_z[2]);
			data.push_back(_z[3]);
		}
		break;

	case 252:
		{
			// collect grasping force magnitude for all four fingertips for plotting
			data.push_back(_magnitude_f[0]);
			data.push_back(_magnitude_f[1]);
			data.push_back(_magnitude_f[2]);
			data.push_back(_magnitude_f[3]);

		}
		break;
	}
}

void control_AHAS::onSetInterestFrame(const TCHAR* name, const HTransform& T)
{
}

rControlAlgorithm* CreateControlAlgorithm(rDC& rdc)
{
	return new control_AHAS(rdc);
}


void control_AHAS::_writeForceArrows()
{
	_hand->GetGraspingForce(_fx, _fy, _fz);
	for(int i=0; i<NOF; i++) _magnitude_f[i] = sqrt(_fx[i]*_fx[i] + _fy[i]*_fy[i] + _fz[i]*_fz[i]);

	float force_arrow_disp_scale = 0.008;
	//START
	_arrow_vector0[0] = _x[0];
	_arrow_vector0[1] = _y[0];
	_arrow_vector0[2] = _z[0];

	_arrow_vector1[0] = _x[1];
	_arrow_vector1[1] = _y[1];
	_arrow_vector1[2] = _z[1];

	_arrow_vector2[0] = _x[2];
	_arrow_vector2[1] = _y[2];
	_arrow_vector2[2] = _z[2];

	_arrow_vector3[0] = _x[3];
	_arrow_vector3[1] = _y[3];
	_arrow_vector3[2] = _z[3];

	//END
	_arrow_vector0[3] = _x[0]+_fx[0]*force_arrow_disp_scale;
	_arrow_vector0[4] = _y[0]+_fy[0]*force_arrow_disp_scale;
	_arrow_vector0[5] = _z[0]+_fz[0]*force_arrow_disp_scale;

	_arrow_vector1[3] = _x[1]+_fx[1]*force_arrow_disp_scale;
	_arrow_vector1[4] = _y[1]+_fy[1]*force_arrow_disp_scale;
	_arrow_vector1[5] = _z[1]+_fz[1]*force_arrow_disp_scale;

	_arrow_vector2[3] = _x[2]+_fx[2]*force_arrow_disp_scale;
	_arrow_vector2[4] = _y[2]+_fy[2]*force_arrow_disp_scale;
	_arrow_vector2[5] = _z[2]+_fz[2]*force_arrow_disp_scale;

	_arrow_vector3[3] = _x[3]+_fx[3]*force_arrow_disp_scale;
	_arrow_vector3[4] = _y[3]+_fy[3]*force_arrow_disp_scale;
	_arrow_vector3[5] = _z[3]+_fz[3]*force_arrow_disp_scale;

	_arrow_vector0[6] = _arrow_vector1[6] = _arrow_vector2[6] = _arrow_vector3[6] = 1; //R
	_arrow_vector0[7] = _arrow_vector1[7] = _arrow_vector2[7] = _arrow_vector3[7] = 0; //G
	_arrow_vector0[8] = _arrow_vector1[8] = _arrow_vector2[8] = _arrow_vector3[8] = 0; //B
	_arrow_vector0[9] = _arrow_vector1[9] = _arrow_vector2[9] = _arrow_vector3[9] = 1; //A

	if (_arrow0 != INVALID_RHANDLE)
		writeDeviceValue(_arrow0, _arrow_vector0, sizeof(float)*10);

	if (_arrow1 != INVALID_RHANDLE)
		writeDeviceValue(_arrow1, _arrow_vector1, sizeof(float)*10);

	if (_arrow2 != INVALID_RHANDLE)
		writeDeviceValue(_arrow2, _arrow_vector2, sizeof(float)*10);

	if (_arrow3 != INVALID_RHANDLE)
		writeDeviceValue(_arrow3, _arrow_vector3, sizeof(float)*10);
}

void control_AHAS::_readMessage_TCPIP()
{
	if(_remoteTP_TCPIP != INVALID_RHANDLE)
	{
		int szRead = readDeviceValue(_remoteTP_TCPIP, &_msgRTP, _sz2read_msgRTP, DEVPORT_FETCH_MESSAGE);
		if(szRead == _sz2read_msgRTP && szRead > 0)
		{
#ifdef DEBUG_COMM
			printf("type : %d, size : %d, cmd : %d, data : %d\n", _msgRTP.type, _msgRTP.size, _msgRTP.cmd, _msgRTP.data);
#endif
			if(_msgRTP.type == RTP_TYPE_COMMAND_EVENT)
			{
				switch(_msgRTP.cmd)
				{
				case RTP_CMD_CMD1:
					{
						command(BH_HOME);
					}
					break;

				case RTP_CMD_CMD2:
					{
						command(BH_READY);
					}
					break;

				case RTP_CMD_CMD3:
					{
						command(BH_GRASP_3);
					}
					break;

				case RTP_CMD_CMD4:
					{
						command(BH_GRASP_4);
					}
					break;

				case RTP_CMD_CMD5:
					{
						command(BH_PINCH_IT);
					}
					break;

				case RTP_CMD_CMD6:
					{
						command(BH_PINCH_MT);
					}
					break;

				case RTP_CMD_CMD7:
					{
						command(BH_MOVE_OBJ);
					}
					break;

				case RTP_CMD_CMD8:
					{
						command(BH_ENVELOP);
					}
					break;

				case RTP_CMD_CMD9:
					{
						printf("reserved command[9]\n");
					}
					break;

				case RTP_CMD_CMD10:
					{
						printf("reserved command[10]\n");
					}
					break;

				case RTP_CMD_CMD11:
					{
						command(BH_SHOWOFF);
					}
					break;

				case RTP_CMD_CMD12:
					{
						printf("reserved command[12]\n");
					}
					break;
				}
			}
		}
	} // TCP/IP

}
void control_AHAS::_readMessage_BT()
{
	if(_remoteTP_BT != INVALID_RHANDLE)
	{
		int szRead = readDeviceValue(_remoteTP_BT, &_msgRTP, _sz2read_msgRTP, DEVPORT_FETCH_MESSAGE);
		if(szRead == _sz2read_msgRTP && szRead > 0)
		{
#ifdef DEBUG_COMM
			printf("type : %d, size : %d, cmd : %d, data : %d\n", _msgRTP.type, _msgRTP.size, _msgRTP.cmd, _msgRTP.data);
#endif
			if(_msgRTP.type == RTP_TYPE_COMMAND_EVENT)
			{
				switch(_msgRTP.cmd)
				{
				case RTP_CMD_CMD1:
					{
						command(BH_HOME);
					}
					break;

				case RTP_CMD_CMD2:
					{
						command(BH_READY);
					}
					break;

				case RTP_CMD_CMD3:
					{
						command(BH_GRASP_3);
					}
					break;

				case RTP_CMD_CMD4:
					{
						command(BH_GRASP_4);
					}
					break;

				case RTP_CMD_CMD5:
					{
						command(BH_PINCH_IT);
					}
					break;

				case RTP_CMD_CMD6:
					{
						command(BH_PINCH_MT);
					}
					break;

				case RTP_CMD_CMD7:
					{
						command(BH_MOVE_OBJ);
					}
					break;

				case RTP_CMD_CMD8:
					{
						command(BH_ENVELOP);
					}
					break;

				case RTP_CMD_CMD9:
					{
						printf("reserved command[9]\n");
					}
					break;

				case RTP_CMD_CMD10:
					{
						printf("reserved command[10]\n");
					}
					break;

				case RTP_CMD_CMD11:
					{
						command(BH_SHOWOFF);
					}
					break;

				case RTP_CMD_CMD12:
					{
						printf("reserved command[12]\n");
					}
					break;
				}
			}
		}
	} // bluetooth
}
void control_AHAS::_demo_jointMovement()
{
	static double q_home_left[NOF][NOJ] = {
		{  0*DEG2RAD,	-10*DEG2RAD,	45*DEG2RAD,		45*DEG2RAD},
		{  0*DEG2RAD,	-10*DEG2RAD,	45*DEG2RAD,		45*DEG2RAD},
		{ -5*DEG2RAD,	 -5*DEG2RAD,	50*DEG2RAD,		45*DEG2RAD},
		{ 50*DEG2RAD,	 25*DEG2RAD,	15*DEG2RAD,		45*DEG2RAD}
	};

	static double q_home_right[NOF][NOJ] = {
		{  0*DEG2RAD,	-10*DEG2RAD,	45*DEG2RAD,		45*DEG2RAD},
		{  0*DEG2RAD,	-10*DEG2RAD,	45*DEG2RAD,		45*DEG2RAD},
		{  5*DEG2RAD,	 -5*DEG2RAD,	50*DEG2RAD,		45*DEG2RAD},
		{ 50*DEG2RAD,	 25*DEG2RAD,	15*DEG2RAD,		45*DEG2RAD}
	};
	_demo_q_des.zero();
	if (_is_left_hand)
	{
		double delQ = 20*DEGREE*sin(2.0*M_PI*0.5*_cur_time-_demo_start_time)*sin(2.0*M_PI*0.5*_cur_time-_demo_start_time);
		for (int i=0; i<NOF; i++)
			for (int j=0; j<NOJ; j++)
				_demo_q_des[i*NOJ+j] = q_home_left[i][j] + delQ;
	}
	else
	{
		double delQ = 20*DEGREE*sin(2.0*M_PI*0.5*_cur_time-_demo_start_time)*sin(2.0*M_PI*0.5*_cur_time-_demo_start_time);
		for (int i=0; i<NOF; i++)
			for (int j=0; j<NOJ; j++)
				_demo_q_des[i*NOJ+j] = q_home_left[i][j] + delQ;
	}
	_hand->SetJointDesiredPosition(_demo_q_des.array);
}
void control_AHAS::_jointDirectionTest()
{
	// TEST1: constant torque
	_torque.zero();
	_torque[_jid_test] = 200.0f / 800.0f;

	// TEST2: PD control
	/*double tau_tmp = _torque[_jid_test];
	if (tau_tmp < -0.5) tau_tmp = -0.5;
	else if (tau_tmp > 0.5) tau_tmp = 0.5;

	_torque.zero();
	_torque[_jid_test] = tau_tmp;*/
}