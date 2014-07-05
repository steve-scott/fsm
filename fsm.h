/*
 *
 * File: fsm.h
 *
 * Hierarchical State Machine declarations
 *
 *
 */

#ifndef _FSM_H_
#define _FSM_H_

#ifndef __cplusplus
typedef unsigned char	bool;
#define	true			1
#define false			0
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************/
// C implementation of OOP Hierarchical State Machine
/**************************************************************************************************/

// A decent article on hierarhical state machines:
// http://www.barrgroup.com/Embedded-Systems/How-To/Introduction-Hierarchical-State-Machines .

// See notes in FSM_README.txt in fsm folder for instructions on using the data structures declared
// in this file.

/**************************************************************************************************/
// Base class event list

#define FSM_NON_EVENTS   \
   FSM_EVENT_ID(EVT_FSM_NULL)  FSM_EVENT_ID_VAL(-1)

#define FSM_STD_EVENTS                                 \
	FSM_EVENT_ID(EVT_FSM_ENTRY) FSM_EVENT_ID_VAL(0),   \
	FSM_EVENT_ID(EVT_FSM_EXIT),                        \
	FSM_EVENT_ID(EVT_FSM_SUPERSTATE_ENTRY),            \
	FSM_EVENT_ID(EVT_FSM_SUPERSTATE_EXIT),             \
	FSM_EVENT_ID(EVT_FSM_DEFAULT)

// Normally you'll have additional IDs for your state machines.
// Recommended practice is just to have a single list of event IDs for your whole
// system, so each ID in the system is unique. This really helps when debugging
// (like when you send an event to the wrong thread or state machine). So copy the
// enum below into the fsm_events.h file, add your own event ids, and define
// FSM_EVENT_ID and FSM_EVENT_ID_VAL as shown.
// Or you can put them here and define FSM_ENUM.
#ifdef FSM_ENUM

#undef FSM_EVENT_ID
#undef FSM_EVENT_ID_VAL
#define FSM_EVENT_ID(x)        x
#define FSM_EVENT_ID_VAL(x)  =(x)
typedef enum
{
	FSM_NON_EVENTS,			// These must be first!!!
	FSM_STD_EVENTS,			// These must be second!!!
	//------ put your events here -------
	//------ new entries above here -----
	EVT_FSM_EOL			// keep this last
} eFsmEvent;

// To turn the symbol name into a string (for example to create an array of strings
// of the event ID enum symbols), use these definitions:
//		#define FSM_EVENT_ID(x)        #x
//		#define FSM_EVENT_ID_VAL(x)

#else
#ifndef FSM_EVENT_FILE
#define FSM_EVENT_FILE "fsm_events.h"
#endif
#include FSM_EVENT_FILE

#endif // FSM_ENUM

// Finite State Machine base class
typedef struct Fsm Fsm;
typedef struct FsmEvent FsmEvent;
typedef struct FsmState FsmState, *FsmStatePtr;
typedef FsmStatePtr (*FsmEvtHandler)(FsmState* pState, FsmEvent * pEvent);		// returns state ptr to transition to, NULL if no transition
typedef bool (*FsmStateHandler)(FsmState *pState, int eventId);	// returns true if no further event processing (i.e., event was consumed)
typedef struct FsmQ FsmQ;

struct Fsm
{
	const char *	name;
	FsmStatePtr		pState; /* the current state */
	FsmQ *			deferQ;
	FsmQ *			recallQ;
};

// State base class
struct FsmState
{
	Fsm*			pFsm;			// pointer to fsm this state belongs to
	Fsm**			nestedFsmList;	// Array of pointers to nested FSMs
	FsmEvent**		eventList;		// Array of event pointers handled by this state
	const char* 	name;
	FsmStateHandler	pfnStateHandler;
	FsmStatePtr		pNextState;
};

// Event base Class

struct FsmEvent
{
	int 			id;
	FsmEvtHandler	pfnEvtHandler;
	bool			consumed;
	int 			altId;		// used by FSM to store real id when processing "default" event
								// don't set this field! Read it when handling a EVT_FSM_DEFAULT event
};

// Base class methods
void FsmInit (Fsm *pFsm, FsmState *pState);
void FsmRun(Fsm *pFsm, int eventId);
bool FsmDispatch(Fsm *pFsm, int eventId);
void FsmTransition(Fsm *pFsm, FsmStatePtr pNextState);
bool FsmStateDefaultHandler(FsmState *pState, int eventId);
int  FsmDeferEvent(Fsm* pFsm, int eventId);
int  FsmRecallEvent(Fsm* pFsm, int *pEventId);

// Base class data members
extern FsmEvent fsmNullEvent;

// FSM event queues
struct FsmQ {
	int size;
	int	head;
	int	tail;
	int	count;
	int	eventId[];
};

int  FsmPutEvent (FsmQ *q, int eventId);
int  FsmGetEvent (FsmQ *q);

// test function
void	TestFsm(void);

// Logging macros

#ifndef FSM_LOG
#include <stdio.h>
	#if defined(_WIN32)
		#if (FSM_LOG_FILE)
			extern FILE * csvFile;
			#define FSM_LOG(format, ...)	{									\
				printf( "%s,"format"\n", __FUNCTION__, ##__VA_ARGS__);			\
				fprintf(csvFile, "%s,"format"\n", __FUNCTION__, ##__VA_ARGS__);	\
				}
		#else
		#define FSM_LOG(format, ...)	{										\
			printf( "%s,"format"\n\r", __FUNCTION__, ##__VA_ARGS__);			\
			}
		#endif
	#elif defined(__GNUC__)
		#define FSM_LOG(format, ...)	{printf( "%s,"format"\n\r", __func__, ##__VA_ARGS__);}
	#else
		#define FSM_LOG(format, ...)	{printf( "%s,"format"\n\r","xxx", ##__VA_ARGS__);}
	#endif
#endif

// Use these macros in your .c files to trace on your state machines.
// #define FSM_TRACE 1 in your .c module before including fsm.h to turn on tracing
// This way you can enable tracing for selected state machines, as long as they are
// implemented in separate files.

#if FSM_TRACE
	#define FSM_RUN_LOG(format, ...)	FSM_LOG("run,"format, ##__VA_ARGS__)
	#define FSM_ENTER_LOG(format, ...)	FSM_LOG("enter,"format, ##__VA_ARGS__)
	#define FSM_EXIT_LOG(format, ...)	FSM_LOG("exit,"format, ##__VA_ARGS__)
#else
	#define FSM_RUN_LOG(format, ...)	{;}
	#define FSM_ENTER_LOG(format, ...)	{;}
	#define FSM_EXIT_LOG(format, ...)	{;}
#endif

// Macros for object initialization

// Designated Initializers for structures are supported in C99, but not in C++.
// GNU g++ compiler has an extension that supports them, but the syntax is not C99.
#ifdef __cplusplus
	#ifdef __GNUG__		// g++ compiler
		#define DESIG_INIT(field,value)	field : (value)
	#else
		#error	Designated Initializers not supported
	#endif
#else
	#define DESIG_INIT(field,value)	.field=(value)
#endif

// ... FSM objects
#define FSM(obj,name_str,initial_state,defer_Q,recall_Q)	\
	Fsm obj = {									\
		DESIG_INIT(name,name_str),				\
		DESIG_INIT(pState,initial_state),		\
		DESIG_INIT(deferQ,(FsmQ*)defer_Q),		\
		DESIG_INIT(recallQ,(FsmQ*)recall_Q)		\
		}

// ... Event Handlers
#define FSM_EVENT_HANDLER(handler)	FsmStatePtr handler(FsmState* pState, FsmEvent * pEvent)

// ... Event objects
#define FSM_EVENT(obj,event_id,handler)	\
	FsmEvent obj = { DESIG_INIT(id,(event_id)), DESIG_INIT(pfnEvtHandler,handler) }

// ... State Objects
#define FSM_STATE(obj,fsm,nested_fsm_list,event_list,name_str,handler)	\
	FsmState obj = {								\
		DESIG_INIT(pFsm,fsm),						\
		DESIG_INIT(nestedFsmList,nested_fsm_list),	\
		DESIG_INIT(eventList,event_list),			\
		DESIG_INIT(name,name_str),					\
		DESIG_INIT(pfnStateHandler,handler)			\
		}

// ... Event Queues
#define	FSM_Q(obj,qsize)	\
	struct obj##_tag {		\
		int	size;			\
		int	head;			\
		int	tail;			\
		int	count;			\
		int	eventId[qsize];	\
	} obj = { qsize, 0, 0, 0 };

#define FSM_Q_INIT(obj)	{ obj.head = 0; obj.tail = 0; obj.count = 0;}

#ifdef __cplusplus
}
#endif

#endif // _FSM_H_
