
#ifndef _FSM_EVENTS_H_
#define _FSM_EVENTS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MY_EVENTS				\
	FSM_EVENT_ID(MYEVT_BOL),	\
	FSM_EVENT_ID(EVT_1),		\
	FSM_EVENT_ID(EVT_2),		\
	FSM_EVENT_ID(EVT_3),		\
	FSM_EVENT_ID(EVT_4),		\
	FSM_EVENT_ID(MYEVT_EOL)

// create the event ID list
#undef FSM_EVENT_ID
#undef FSM_EVENT_ID_VAL

#define FSM_EVENT_ID(x)        x
#define FSM_EVENT_ID_VAL(x)  =(x)
typedef enum
{
	FSM_NON_EVENTS,			// These must be first!!!
	FSM_STD_EVENTS,			// These must be second!!!
	//------ put your events here -------
	MY_EVENTS,
	//------ new entries above here -----
	EVT_FSM_EOL			// keep this last
} eFsmEvent;

// create an array of event names, indexed by event ID
#undef FSM_EVENT_ID
#undef FSM_EVENT_ID_VAL

#define FSM_EVENT_ID(x)        #x
#define FSM_EVENT_ID_VAL(x)

extern const char* myEvents[];

#ifdef _FSM_EXAMPLE_C_
const char * myEvents[] = { FSM_STD_EVENTS, MY_EVENTS };	// don't include the FSM_NON_EVENTS in this array!
#endif

#undef FSM_EVT_NAME
#define FSM_EVT_NAME(x) myEvents[x]

#define IS_MY_EVENT(x)	((x<MYEVT_EOL) && (x>MYEVT_BOL))
#ifdef __cplusplus
}
#endif

#endif // _FSM_EVENTS_H_