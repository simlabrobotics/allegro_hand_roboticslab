/* RoboticsLab, Copyright 2008-2011 SimLab Co., Ltd. All rights reserved.
 *
 * This library is commercial and cannot be redistributed, and/or modified
 * WITHOUT ANY ALLOWANCE OR PERMISSION OF SimLab Co., LTD.
 */
#include "stdafx.h"

#include "rMath/rMath.h"
using namespace rMath;

#include "../control_AHAS/control_AHAS.h"
#include "../control_AHAS/control_AHASCmd.h"

#include "rDeviceERHandCANCmd.h"
#include "rxSDK/rxSDK.h"
#include "rCommon/rCodeUtil.h"

bool bContact = false;		// Enables/disables contact dynamics.
bool bQuit = false;			// Set this flag true to quit this program.
bool bRun = false;			// Set this flag true to activate the program.
										// See OnKeyRun() function for the details.
const rTime delT = 0.003;


///////////
// RIGHT //
///////////

//	Make sure to copy the correct dml files into the devices folder for the hand you are using
//	Make sure you are using the correct (left.right) simulation model in "simERHand.cpp"

//	Softing CAN
//  string_type aml_path = _T("models/Etc/ERHand/ERHand_RT_SOFTING.aml");

//	NI CAN
//	string_type aml_path = _T("models/Etc/ERHand/ERHand_RT_NICAN.aml");

//	ESD CAN
//	string_type aml_path = _T("models/Etc/ERHand/ERHand_RT_ESDCAN.aml");

//	Kvaser CAN
//	string_type aml_path = _T("models/Etc/ERHand/ERHand_RT_KvaserCAN.aml");

//	Peak CAN
	string_type aml_path = _T("models/Etc/ERHand/ERHand_RT_PeakCAN.aml");

//	Peak CAN
//	string_type aml_path = _T("models/Etc/ERHand/ERHand_RT_PeakPCICAN.aml");


//////////
// LEFT //
//////////

//	Make sure to copy the correct dml files into the devices folder for the hand you are using
//	Make sure you are using the correct (left.right) simulation model in "simERHand.cpp"

//	Softing CAN
//	string_type aml_path = _T("models/Etc/ERHand/ERHandL_RT_SOFTING.aml");

//	NI CAN
//	string_type aml_path = _T("models/Etc/ERHand/ERHandL_RT_NICAN.aml");

//	ESD CAN
//	string_type aml_path = _T("models/Etc/ERHand/ERHandL_RT_ESDCAN.aml");

//	Kvaser CAN
//	string_type aml_path = _T("models/Etc/ERHand/ERHandL_RT_KvaserCAN.aml");

//	Peak CAN
//	string_type aml_path = _T("models/Etc/ERHand/ERHandL_RT_PeakCAN.aml");

//	Peak CAN
//	string_type aml_path = _T("models/Etc/ERHand/ERHandL_RT_PeakPCICAN.aml");



string_type aml_name = _T("hand1");  // is named in control_ERHand2.XDL
HTransform aml_T0;
dVector aml_q0;
rxSystem* sys_hand1 = NULL;
rxDevice* devCAN = NULL;
rxDevice* devEnc[16] = {NULL, };
string_type devCAN_name = _T("COMM");
rxControlInterface* control = NULL;
string_type control_path = _T("controls/control_AHAS_R.xdl");
//string_type control_path = _T("controls/control_AHAS_R.xdl");
string_type control_name = _T("MyController");


rID rtTimer = INVALID_RID;
string_type timer_name = _T("RTTimer");
//rTimerMode timer_mode = rTimer_MODE_REAL_RTAPI;
rTimerMode timer_mode = rTimer_MODE_REAL_WIN32;

void MyKeyboardHandler(int key, void* data);
void MyControlCallback(rTime time, void* data);
void SetupDAQ();
void PrintInstrunction();

int _tmain(int argc, _TCHAR* argv[])
{
	rCreateWorld(bContact, delT);
	rSetGravity(0.0f, 0.0f, (float)-GRAV_ACC);

	rtTimer = rCreateTimer(timer_name, timer_mode, delT);
	sys_hand1 = rCreateRTSystem(aml_path, aml_name, aml_T0, aml_q0, rtTimer);

	if (sys_hand1)
	{
		devCAN = sys_hand1->findDevice(devCAN_name);
		for (int i=0; i<16; i++)
		{
			TCHAR dname[256];
			_stprintf_s(dname, 256, _T("enc%d"), i+1);
			devEnc[i] = sys_hand1->findDevice(dname);
		}
	}



	rInitializeEx(true, false);
	//rInitializeEx(false, false);

	if (sys_hand1)
	{
		int step = 1;
		control = rCreateController(control_name, step*delT, sys_hand1, rtTimer);
		control->setAlgorithmDll(control_path);
		control->setPeriod(step*delT);
		control->setNominalSystem(aml_path, aml_name, aml_T0, aml_q0);
		control->initAlgorithm();
	}


	SetupDAQ();

	rAddKeyboardHandler(MyKeyboardHandler, NULL);
	rAddControlHandler(MyControlCallback, NULL, delT, rtTimer);

	PrintInstrunction();
	Sleep(1000);
	MyKeyboardHandler(VK_TAB, NULL); // activate

	rRun(-1, 50);

	return 0;
}

void MyKeyboardHandler(int key, void* data)
{
	switch (key)
	{
	case VK_TAB:
		{
			bRun = !bRun;
			if(bRun)
			{
				if (devCAN)
					devCAN->on();
				if (control) 
					control->command(RESERVED_CMD_SERVO_ON);

				rActivateWorld();
			}
			else
			{
				if (devCAN)
					devCAN->off();
				if (control)
					control->command(RESERVED_CMD_SERVO_OFF);

				rDeactivateWorld();
			}
		}
		break;
	
	case VK_Q:
		{
			if (control) 
				control->command(RESERVED_CMD_SERVO_ON);

			rDeactivateWorld();
			//bQuit = true;
			rQuit();
		}
		break;

	case VK_H:
	//case VK_Z:
		{
			if (control)
				control->command(RCMD_GO_HOME, key);
		}
		break;

	case VK_R:
		{
			if (control)
				control->command(BH_READY, key);
		}
		break;

	case VK_G:
		{
			if (control)
				control->command(BH_GRASP_3, key);
		}
		break;

	case VK_K:
		{
			if (control)
				control->command(BH_GRASP_4, key);
		}
		break;

	case VK_P:
		{
			if (control)
				control->command(BH_PINCH_IT, key);
		}
		break;

	case VK_M:
		{
			if (control)
				control->command(BH_PINCH_MT, key);
		}
		break;

	case VK_E:
		{
			if (control)
				control->command(BH_ENVELOP, key);
		}
		break;

	case VK_9:
		{
			if (control)
				control->command(BH_TEST);
		}
		break;

	case VK_O:
		{
			if (control)
				control->command(BH_NONE, key);
		}
		break;

	case VK_A:
		{
			if (control)
				control->command(BH_GRAVITY_COMP,key);
		}
		break;

	case VK_Z:
		{
			if (control)
				control->command(BH_MOVE_OBJ,key);
		}
		break;

/*	case VK_W:
		{
			if (control)
				control->command(BH_ENCPORT);
		}
		break;
*/
	case VK_F:
		{
			for (int i=0; i<16; i++)
			{
				if (devEnc[i])
					devEnc[i]->command(CAN_CMD_RESET_ENC);
			}
		}
		break;

	case VK_S:
		{
			if (control)
				control->command(BH_SHOWOFF, key);
		}
		break;
	}
}

void MyControlCallback(rTime time, void* data)
{
	rSetTime(rGetTime(rtTimer));
	//printf("MyControlCallback: time=%.3f\n", time);
}

void SetupDAQ()
{
	rID pid_f1 = rdaqCreatePlot(_T("finger_1"), eDataPlotType_TimeLine, rtTimer);
	rdaqAddData(pid_f1, control, 249);

	rID pid_f2 = rdaqCreatePlot(_T("finger_2"), eDataPlotType_TimeLine, rtTimer);
	rdaqAddData(pid_f2, control, 248);

	rID pid_f3 = rdaqCreatePlot(_T("finger_3"), eDataPlotType_TimeLine, rtTimer);
	rdaqAddData(pid_f3, control, 247);

	rID pid_f4 = rdaqCreatePlot(_T("finger_4"), eDataPlotType_TimeLine, rtTimer);
	rdaqAddData(pid_f4, control, 246);

	rID pid_lp = rdaqCreatePlot(_T("low_pass"), eDataPlotType_TimeLine, rtTimer);
	rdaqAddData(pid_lp, control, 245);

	rID pid_raw = rdaqCreatePlot(_T("raw_enc"), eDataPlotType_TimeLine, rtTimer);
	rdaqAddData(pid_raw, control, 244);


	if (control)
	{
		rID pid_tau = rdaqCreatePlot(_T("torque"), eDataPlotType_TimeLine, rtTimer);
		rdaqAddData(pid_tau, control, PLOT_TORQUE);

		rID pid_x = rdaqCreatePlot(_T("x"), eDataPlotType_TimeLine, rtTimer);
		rdaqAddData(pid_x, control, 255);

		rID pid_y = rdaqCreatePlot(_T("y"), eDataPlotType_TimeLine, rtTimer);
		rdaqAddData(pid_y, control, 254);

		rID pid_z = rdaqCreatePlot(_T("z"), eDataPlotType_TimeLine, rtTimer);
		rdaqAddData(pid_z, control, 253);

		rID force_mag = rdaqCreatePlot(_T("tip_force"), eDataPlotType_TimeLine, rtTimer);
		rdaqAddData(force_mag, control, 252);
	}
}

void PrintInstrunction()
{
	printf("---------------------------------------------------\n");
	printf("Allegro Hand v1.0 operating instruction.\n");
	printf("\n");
	printf("(H)ome position\n");
	printf("(R)eady\n");
	printf("(G)rasp\n");
	printf("(E)nvelop grasp\n");
	printf("(P)inching using index finger\n");
	printf("Pinching using (M)iddle finger\n");
	printf("\n");
	printf("Servo (O)ff\n");
	printf("TAB: pause/restart\n");
	printf("F: print encoder offsets\n");
	printf("(Q)uit this program\n");
	printf("---------------------------------------------------\n");
}
