/* RoboticsLab, Copyright 2008-2010 SimLab Co., Ltd. All rights reserved.
 *
 * This library is commercial and cannot be redistributed, and/or modified
 * WITHOUT ANY ALLOWANCE OR PERMISSION OF SimLab Co., LTD.
 */
#ifndef __CONTROL_AHAS_CMD_H__
#define __CONTROL_AHAS_CMD_H__

#include "rCommand/rCmdManipulator.h"

#define BH_NONE		(RCMD_USER + 0)
#define BH_HOME		(RCMD_GO_HOME)
#define BH_READY	(RCMD_USER + 2)
#define BH_GRASP_3	(RCMD_USER + 3)
#define BH_GRASP_4	(RCMD_USER + 4)
#define BH_PINCH_IT	(RCMD_USER + 5)
#define BH_PINCH_MT	(RCMD_USER + 6)
#define BH_MOVE_OBJ	(RCMD_USER + 7)
#define BH_ENVELOP	(RCMD_USER + 8)

#define BH_SHOWOFF	(RCMD_USER + 50)

#define BH_GRAVITY_COMP	(RCMD_USER + 51)

#define BH_TEST		(RCMD_USER + 99)

#define BH_ENCPORT	(RCMD_USER + 100)


#endif
