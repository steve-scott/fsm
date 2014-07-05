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
typedef enum
{
	EVT_FSM_NULL = -1,
	EVT_FSM_ENTRY = 0,
	EVT_FSM_EXIT,
	EVT_FSM_SUPERSTATE_ENTRY,
	EVT_FSM_SUPERSTATE_EXIT,
	//------ new entries above here -----
	EVT_FSM_EOL			// keep this last
} eFsmEvent;

// Finite State Machine base class
typedef struct Fsm Fsm;
typedef struct FsmEvent FsmEvent;
typedef struct FsmState FsmState, *FsmStatePtr;
typedef FsmStatePtr (*FsmEvtHandler)(FsmState* pState, FsmEvent * pEvent);		// returns state ptr to transition to, NULL if no transition
typedef bool (*FsmStateHandler)(FsmState *pState, int eventId);	// returns true if no further event processing (i.e., event was consumed)

struct Fsm
{
	const char *	name;
	FsmStatePtr		pState; /* the current state */
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
};

// Base class methods
void FsmInit (Fsm *pFsm, FsmState *pState);
bool FsmDispatch(Fsm *pFsm, int eventId);
void FsmTransition(Fsm *pFsm, FsmStatePtr pNextState);
bool FsmStateDefaultHandler(FsmState *pState, int eventId);

// Base class data members
extern FsmEvent fsmNullEvent;

// test function
void	TestFsm(void);

// Logging macros

#ifndef FSM_LOG
#include <stdio.h>
#define FSM_LOG(format, ...)	printf( "%s,"format"\n\r", __func__, ##__VA_ARGS__)
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
#define FSM(obj,name_str,initial_state)	\
	Fsm obj = {	DESIG_INIT(name,name_str), DESIG_INIT(pState,initial_state) }

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


#ifdef __cplusplus
}
#endif

#endif // _FSM_H_
