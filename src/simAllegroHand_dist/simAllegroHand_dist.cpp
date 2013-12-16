/* RoboticsLab, Copyright 2008-2011 SimLab Co., Ltd. All rights reserved.
 *
 * This library is commercial and cannot be redistributed, and/or modified
 * WITHOUT ANY ALLOWANCE OR PERMISSION OF SimLab Co., LTD.
 */
#include "stdafx.h"

#include "rMath/rMath.h"
using namespace rMath;

#include "../control_AHAS/control_AHASCmd.h"

#include "rxSDK/rxSDK.h"
#include "rCommon/rCodeUtil.h"

bool bContact = true;		// Enables/disables contact dynamics.
bool bQuit = false;			// Set this flag true to quit this program.
bool bRun = false;			// Set this flag true to activate the program.
										// See OnKeyRun() function for the details.
const rTime delT = 0.005;

// Right Hand
string_type aml_path = _T("models/Etc/ERHand/ERHand.aml");

// Left Hand
//string_type aml_path = _T("models/Etc/ERHand/ERHandL.aml");

string_type aml_name = _T("hand1");
HTransform aml_T0;
dVector aml_q0;
rxSystem* sys_hand1 = NULL;
string_type eml_path = _T("models/default.eml");
string_type eml_name = _T("Environment");
HTransform eml_T0;
rxEnvironment* env = NULL;
rxControlInterface* control = NULL;

string_type control_path = _T("controls/control_AHAS_R.xdl");
//string_type control_path = _T("controls/control_AHAS_R.xdl");

string_type control_name = _T("MyController");
rxSystem* apple = NULL;
string_type apple_path = _T("models/Etc/Objects/apple.aml");
string_type apple_name = _T("Apple");
HTransform apple_T0;
dVector apple_q0;

void MyKeyboardHandler(int key, void* data);
void MyControlCallback(rTime time, void* data);
void SetupDAQ();
void PrintInstrunction();

int _tmain(int argc, _TCHAR* argv[])
{
	rCreateWorld(bContact, delT);
	rSetGravity(0, 0, -GRAV_ACC);

	aml_T0.r[0] = 0.0; aml_T0.r[1] = 0.0; aml_T0.r[2] = 0.0;
	sys_hand1 = rCreateSystem(aml_path, aml_name, aml_T0, aml_q0);

	// for Right Hand, Neg y for Left Hand
	apple_T0.r[0] = 0.03; apple_T0.r[1] = 0.03; apple_T0.r[2] = 0.02;
	apple = rCreateStaticSystem(apple_path, apple_name, apple_T0, apple_q0);

	env = rCreateEnvironment(eml_path, eml_name, eml_T0);

	rInitializeEx(true, true);

	if (sys_hand1)
	{
		int step = 1;
		control = rCreateController(control_name, sys_hand1, step);
		control->setAlgorithmDll(control_path);
		control->setPeriod(step*delT);
		control->setNominalSystem(aml_path, aml_name, aml_T0, aml_q0);
		control->initAlgorithm();
	}

	SetupDAQ();
	PrintInstrunction();

	rAddKeyboardHandler(MyKeyboardHandler, NULL);
	rAddControlHandler(MyControlCallback, NULL);

	rRun(0);

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
				if (control) 
					control->command(RESERVED_CMD_SERVO_ON);
				rActivateWorld();
			}
			else
			{
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
			printf("Home\n");
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

	/*case VK_1:
		{
			if (control)
				control->command(BH_TEST);
		}
		break;*/

	case VK_O:
		{
			if (control)
				control->command(BH_NONE, key);
		}

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
	//printf("MyControlCallback: time=%.3f\n", time);
}

void SetupDAQ()
{
	rID pid_f1 = rdaqCreatePlot(_T("finger_1"), eDataPlotType_TimeLine);
	rdaqAddData(pid_f1, sys_hand1->findDevice(_T("enc1")), eDeviceDataType_ReadFloat);
	rdaqAddData(pid_f1, sys_hand1->findDevice(_T("enc2")), eDeviceDataType_ReadFloat);
	rdaqAddData(pid_f1, sys_hand1->findDevice(_T("enc3")), eDeviceDataType_ReadFloat);
	rdaqAddData(pid_f1, sys_hand1->findDevice(_T("enc4")), eDeviceDataType_ReadFloat);

	rID pid_f2 = rdaqCreatePlot(_T("finger_2"), eDataPlotType_TimeLine);
	rdaqAddData(pid_f2, sys_hand1->findDevice(_T("enc5")), eDeviceDataType_ReadFloat);
	rdaqAddData(pid_f2, sys_hand1->findDevice(_T("enc6")), eDeviceDataType_ReadFloat);
	rdaqAddData(pid_f2, sys_hand1->findDevice(_T("enc7")), eDeviceDataType_ReadFloat);
	rdaqAddData(pid_f2, sys_hand1->findDevice(_T("enc8")), eDeviceDataType_ReadFloat);

	rID pid_f3 = rdaqCreatePlot(_T("finger_3"), eDataPlotType_TimeLine);
	rdaqAddData(pid_f3, sys_hand1->findDevice(_T("enc9")), eDeviceDataType_ReadFloat);
	rdaqAddData(pid_f3, sys_hand1->findDevice(_T("enc10")), eDeviceDataType_ReadFloat);
	rdaqAddData(pid_f3, sys_hand1->findDevice(_T("enc11")), eDeviceDataType_ReadFloat);
	rdaqAddData(pid_f3, sys_hand1->findDevice(_T("enc12")), eDeviceDataType_ReadFloat);

	rID pid_f4 = rdaqCreatePlot(_T("finger_4"), eDataPlotType_TimeLine);
	rdaqAddData(pid_f4, sys_hand1->findDevice(_T("enc13")), eDeviceDataType_ReadFloat);
	rdaqAddData(pid_f4, sys_hand1->findDevice(_T("enc14")), eDeviceDataType_ReadFloat);
	rdaqAddData(pid_f4, sys_hand1->findDevice(_T("enc15")), eDeviceDataType_ReadFloat);
	rdaqAddData(pid_f4, sys_hand1->findDevice(_T("enc16")), eDeviceDataType_ReadFloat);

	if (control)
	{
		rID pid_tau = rdaqCreatePlot(_T("torque"), eDataPlotType_TimeLine);
		rdaqAddData(pid_tau, control, ePlotManipulator::PLOT_TORQUE);

		rID pid_x = rdaqCreatePlot(_T("x"), eDataPlotType_TimeLine);
		rdaqAddData(pid_x, control, 255);

		rID pid_y = rdaqCreatePlot(_T("y"), eDataPlotType_TimeLine);
		rdaqAddData(pid_y, control, 254);

		rID pid_z = rdaqCreatePlot(_T("z"), eDataPlotType_TimeLine);
		rdaqAddData(pid_z, control, 253);
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
	printf("(Q)uit this program\n");
	printf("---------------------------------------------------\n");
}
