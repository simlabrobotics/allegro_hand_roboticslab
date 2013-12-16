#ifndef __RDEVICEERHANDREMOTECMD_H__
#define __RDEVICEERHANDREMOTECMD_H__

#include "rCommand/rCmdDefine.h"

// COMMON
#define DEVPORT_FETCH_MESSAGE	1

/*** communication protocol ***/
/******************************/

// message type
#define RTP_TYPE_COMMAND_EVENT	1
#define RTP_TYPE_DATA_SENSOR	2
#define RTP_TYPE_DATA_JOG		3

// command
#define RTP_CMD_CMD1			1  // BH_HOME
#define RTP_CMD_CMD2			2  // BH_READY
#define RTP_CMD_CMD3			3  // BH_GRASP_3
#define RTP_CMD_CMD4			4  // BH_GRASP_4
#define RTP_CMD_CMD5			5  // BH_PINCH_IT
#define RTP_CMD_CMD6			6  // BH_PINCH_MT
#define RTP_CMD_CMD7			7  // BH_MOVE_OBJ
#define RTP_CMD_CMD8			8  // BH_ENVELOP
#define RTP_CMD_CMD9			9
#define RTP_CMD_CMD10			10
#define RTP_CMD_CMD11			11 // BH_SHOWOFF
#define RTP_CMD_CMD12			12


// RS232

/*** device command ***/
/**********************/

// command
#define CMD_CONNECT		(RCMD_USER + 100)
#define CMD_DISCONNECT	(RCMD_USER + 101)

/*** message ***/
/***************/

typedef struct tagMessageRemoteTP{
	unsigned char	type;
	unsigned char	size;
	unsigned char	cmd;
	unsigned char	data;
}MessageRemoteTP_t;


//#include <queue>

// TCP/IP

#define DEFAULT_PORT_NUM				6150

/*** device command ***/
/**********************/

// command
#define CMD_SERVER_RUN		4000
#define CMD_SERVER_STOP		4001
#define CMD_CLEAR_QUEUE		4002

// result
#define CMD_RESULT_SUCCESS	1
#define CMD_RESULT_FAIL		-1

#endif // __RDEVICEERHANDREMOTECMD_H__
