
#ifndef _FSM_EVENTS_H_
#define _FSM_EVENTS_H_

#define FSM_USER_EVENTS		\
	FSM_EVENT_ID(MYEVT_BOL)	\
	FSM_EVENT_ID(EVT_1)		\
	FSM_EVENT_ID(EVT_2)		\
	FSM_EVENT_ID(EVT_3)		\
	FSM_EVENT_ID(EVT_4)		\
	FSM_EVENT_ID(MYEVT_EOL)

#define IS_MY_EVENT(x)	((x<MYEVT_EOL) && (x>MYEVT_BOL))

#endif // _FSM_EVENTS_H_
