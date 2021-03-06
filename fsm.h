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

#define	FSM_TEST	1	// set to 0 to disable test functions and objects

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

// The framework defines a base set of event identifiers. Normally you'll have
// additional event IDs for your state machines. Recommended practice is just
// to have a single list of event IDs for your whole system. This has two advantages:
//     1) each ID in the system is unique,
// and 2) you get the whole list in a single enum for stronger type checking

// This really helps when debugging (like when you send an event to the wrong
// thread or state machine).

// You have several options for defining your event IDs:
//   1) Create a macro symbol named FSM_USER_EVENTS in the FSM user include file fsm_events.h
#if 0
//      The macro has the form:
			#define FSM_USER_EVENTS		\
				FSM_EVENT_ID(EVT_1)		\
				FSM_EVENT_ID(EVT_2)		\
				FSM_EVENT_ID(EVT_3)		\
				FSM_EVENT_ID(EVT_4)
//
#endif
//      Define as many events as you want. Substitute your event id symbols for EVT_1, etc.
//      The framework creates the eFsmEvent enum with your identifiers as the enum symbols,
//      along with the framework's base set of event identifiers.
//
//      The framework also creates an identifier name array with strings that have the same name
//      as the enum event id symbols. Your code can get the string identifier for each event id
//      by using
//              FSM_EVT_NAME(x)
//      where x is the identifier symbol (e.g., EVT_1, etc.) The list includes the framework's
//      base set of identifiers.
//
//      Your identifiers will lie in the range 0 < your_identifiers < EVT_FSM_EOL .
//      Identifier values are enums, auto incrementing in the order you define them. The
//      minimum value is not specified, but will be greater than 0.
//
//      This also gives the framework access to your identifiers for debug messages.
//
//   2) Don't define FSM_USER_EVENTS in fsm_events.h . Include fsm.h in your soucce file, then
//     define your identifiers so that they are all >= EVT_FSM_EOL. You can have different lists
//     for each thread or state machine, you just can't use any values below EVT_FSM_EOL.
//     For example
//            typedef enum {
//                MY_EVT_1 = EVT_FSM_EOL,
//                MY_EVT_2,					// etc...
//            } MyEventsA;
//
//            typedef enum {
//                MY_EVT_3 = EVT_FSM_EOL,
//                MY_EVT_4,					// etc...
//            } MyEventsB;
//
//      Your identifiers will all be positive numbers. You may not have system-wide unique
//      event ids depending on how you define them (e.g. in the example above IDs are not unique).
//
//      Your identifiers will NOT be in the framework's identifier name string array. Do NOT use
//      FSM_EVT_NAME(x) with your identifiers - at best you will get garbage, at worst
//      your system will crash inexplicably at random times.
//
//      Using this method, the framework will not have access to any event ID names for
//      your events.

// Define the event identifier lists
#include "fsm_events.h"		// user should define FSM_USER_EVENTS in here
#ifndef FSM_USER_EVENTS
	#define FSM_USER_EVENTS FSM_EVENT_ID(EVT_USER)
#endif

#define FSM_STD_EVENTS                       \
	FSM_EVENT_ID(EVT_FSM_ENTRY)              \
	FSM_EVENT_ID(EVT_FSM_EXIT)               \
	FSM_EVENT_ID(EVT_FSM_SUPERSTATE_ENTRY)   \
	FSM_EVENT_ID(EVT_FSM_SUPERSTATE_EXIT)    \
	FSM_EVENT_ID(EVT_FSM_DEFAULT)

#undef  FSM_EVENT_ID
#define FSM_EVENT_ID(x)        x,
typedef enum
{
	EVT_FSM_NULL = -1,		// This must be first!!!
	FSM_STD_EVENTS			// These must be second!!!
	//------ put user events here -------
	FSM_USER_EVENTS
	//------ user entries above here -----
	EVT_FSM_EOL			// keep this last
} eFsmEvent;

// Create the event name list
#undef  FSM_EVENT_ID
#define FSM_EVENT_ID(x)        #x ,

extern const char * gFsmEventNames[];
#ifdef _FSM_C_
const char * gFsmEventNames[] = {
		FSM_STD_EVENTS			// These must be first!!!
		FSM_USER_EVENTS
		"EVT_FSM_EOL"
};
#endif //_FSM_C_

#define FSM_EVT_NAME(x) gFsmEventNames[x]


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
	int				notifyEventId;
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
		extern FILE * csvFile;
		#define FSM_LOG(format, ...)											\
		{																		\
			printf( "%s,"format"\n", __FUNCTION__, ##__VA_ARGS__);				\
			if (csvFile != NULL)												\
				fprintf(csvFile, "%s,"format"\n", __FUNCTION__, ##__VA_ARGS__);	\
		}
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
	#define FSM_ENTER_LOG(format, ...)	FSM_LOG("begin,"format, ##__VA_ARGS__)
	#define FSM_EXIT_LOG(format, ...)	FSM_LOG("end,"format, ##__VA_ARGS__)
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
		DESIG_INIT(pfnStateHandler,handler),		\
		DESIG_INIT(notifyEventId,EVT_FSM_NULL)		\
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

// Create the event insertion queues for testing
#if !FSM_TEST
	#define FSM_INSERT_BEFORE(pFsm,x)
	#define FSM_INSERT_AFTER(pFsm,x)
	#define FSM_NOTIFY(pState,x)
	#define FSM_SET_NOTIFY_FCN(x)
	#define FSM_CLR_NOTIFY_FCN()
	#define FSM_SET_NOTIFY_EVENT(pState,x)
	#define FSM_CLR_NOTIFY_EVENT(pState)
#else
	#define FSM_MAX_INSERT_EVENTS	8

	int FsmInsertBefore(eFsmEvent eventId, eFsmEvent insertId);
	int FsmInsertAfter(eFsmEvent eventId, eFsmEvent insertId);
	void FsmNotify(void);

	#define FSM_INSERT_BEFORE(pFsm,x)	FsmDoInsertedBefore(pFsm,x)
	#define FSM_INSERT_AFTER(pFsm,x)	FsmDoInsertedAfter(pFsm,x)
	#define FSM_NOTIFY(pState,x)		if (pState->notifyEventId == x) (*gpNotifyFcn)()
	#define FSM_SET_NOTIFY_FCN(x)		gpNotifyFcn = x
	#define FSM_CLR_NOTIFY_FCN()		gpNotifyFcn = FsmNotify
	#define FSM_SET_NOTIFY_EVENT(pState,x)	pState->notifyEventId = x
	#define FSM_CLR_NOTIFY_EVENT(pState)	pState->notifyEventId = EVT_FSM_NULL

	extern FsmQ * gFsmQInsertBefore[];
	extern FsmQ * gFsmQInsertAfter[];

	typedef void (*FsmNotifyFcn)(void);		// this is called when state sees a specified event
	extern FsmNotifyFcn	gpNotifyFcn;

	#ifdef _FSM_C_
		FsmNotifyFcn	gpNotifyFcn = FsmNotify;
		// instantiate the queues
		#undef  FSM_EVENT_ID
		#define FSM_EVENT_ID(x)        FSM_Q(fsmQ_insertBefore_##x,FSM_MAX_INSERT_EVENTS)
		FSM_STD_EVENTS
		FSM_USER_EVENTS

		#undef  FSM_EVENT_ID
		#define FSM_EVENT_ID(x)        FSM_Q(fsmQ_insertAfter_##x,FSM_MAX_INSERT_EVENTS)
		FSM_STD_EVENTS
		FSM_USER_EVENTS

		// instantiate the arrays
		#undef  FSM_EVENT_ID
		#define FSM_EVENT_ID(x)        (FsmQ *)&fsmQ_insertBefore_##x,

		FsmQ * gFsmQInsertBefore[] = {
				FSM_STD_EVENTS			// These must be first!!!
				FSM_USER_EVENTS
		};

		#undef  FSM_EVENT_ID
		#define FSM_EVENT_ID(x)        (FsmQ *)&fsmQ_insertAfter_##x,

		FsmQ * gFsmQInsertAfter[] = {
				FSM_STD_EVENTS			// These must be first!!!
				FSM_USER_EVENTS
		};

	#endif //_FSM_C_
#endif // FSM_TEST

#ifdef __cplusplus
}
#endif

#endif // _FSM_H_
