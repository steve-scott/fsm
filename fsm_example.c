
#define	FSM_TRACE		1	// undefine or set to 0 to disable fsm tracing

#include "fsm.h"

#undef EVT_NAME
#define EVT_NAME(x) myEvents[x]
const char * myEvents[] = {
	"EVT_FSM_ENTRY",
	"EVT_FSM_EXIT",
	"EVT_FSM_SUPERSTATE_ENTRY",
	"EVT_FSM_SUPERSTATE_EXIT",
	"EVT_1",
	"EVT_2",
	"EVT_3",
	"EVT_4",
};

void MyFsmInit (Fsm *pFsm, FsmState *pState);
// State Handler override function
bool MyFsmStateHandler(FsmState *pState, int eventId);

//================
//Define Event IDs
//================
typedef enum {
	EVT_1 = EVT_FSM_EOL,
	EVT_2,
	EVT_3,
	EVT_4,
} eMyEvent;

//==================
//Define Fsm Objects
//==================

FSM(myTopFsm,     "top",      NULL );	// instantiate the top level (superstate) FSM
FSM(myNested1Fsm, "nested_1", NULL );	// instantiate a nested FSM
FSM(myNested2Fsm, "nested_2", NULL );	// instantiate a nested FSM

//==============================
//Define Event objects and lists
//==============================

//++++ Nested FSM 1 State 1 events ++++

// Method prototypes
FSM_EVENT_HANDLER( MyNestedFsm1State1Evt_Entry );
FSM_EVENT_HANDLER( MyNestedFsm1State1Evt_EVT3 );

// Objects
FSM_EVENT( nested1State1Evt_Entry, EVT_FSM_ENTRY, MyNestedFsm1State1Evt_Entry );
FSM_EVENT( nested1State1Evt_EVT3,  EVT_3,         MyNestedFsm1State1Evt_EVT3  );

// Event list array
FsmEvent* nested1State1_EventList[] = {
		&nested1State1Evt_Entry,		// note superstate entry events are ignored, resume where we left off
		&nested1State1Evt_EVT3,
		// keep this last
		&fsmNullEvent
};

//++++ Nested FSM 1 State 2 events ++++

// Method prototypes
FSM_EVENT_HANDLER( MyNestedFsm1State2Evt_EVT4 );

// Objects
FSM_EVENT( nested1State2Evt_EVT4, EVT_4,  MyNestedFsm1State2Evt_EVT4 );

// Event list array
FsmEvent* nested1State2_EventList[] = {
		&nested1State2Evt_EVT4,			// note superstate entry events are ignored, resume where we left off
		// keep this last
		&fsmNullEvent
};

//++++ Nested FSM 2 State 1 events ++++

// Method prototypes
FSM_EVENT_HANDLER( MyNestedFsm2State1Evt_Entry );
FSM_EVENT_HANDLER( MyNestedFsm2State1Evt_EVT3  );

// Objects
// note superstate entry events are treated just like normal entry events
FSM_EVENT( nested2State1Evt_Entry,           EVT_FSM_ENTRY,             MyNestedFsm2State1Evt_Entry );
FSM_EVENT( nested2State1Evt_SuperstateEntry, EVT_FSM_SUPERSTATE_ENTRY,  MyNestedFsm2State1Evt_Entry );
FSM_EVENT( nested2State1Evt_EVT3,            EVT_3,                     MyNestedFsm2State1Evt_EVT3  );

// Event list array
FsmEvent* nested2State1_EventList[] = {
		&nested2State1Evt_Entry,
		&nested2State1Evt_SuperstateEntry,
		&nested2State1Evt_EVT3,
		// keep this last
		&fsmNullEvent
};

//++++ Nested FSM 2 State 2 events ++++

// Method prototypes
FSM_EVENT_HANDLER( MyNestedFsm2State2Evt_EVT4 );

// Objects
FSM_EVENT( nested2State2Evt_EVT4, EVT_4,  MyNestedFsm2State2Evt_EVT4 );

// Event list array
FsmEvent* nested2State2_EventList[] = {
		&nested2State2Evt_EVT4,		// note superstate entry events are ignored - we always enter through nested1State_1
		// keep this last
		&fsmNullEvent
};

//++++ Top State 1 events ++++

// Method prototypes
FSM_EVENT_HANDLER( MyTopFsmState1Evt_Entry );
FSM_EVENT_HANDLER( MyTopFsmState1Evt_EVT1 );

// Objects
FSM_EVENT( topState1Evt_Entry, EVT_FSM_ENTRY,  MyTopFsmState1Evt_Entry );
FSM_EVENT( topState1Evt_EVT1,  EVT_1,          MyTopFsmState1Evt_EVT1  );

// Event list array
FsmEvent* topState1_EventList[] = {
		&topState1Evt_Entry,
		&topState1Evt_EVT1,
		// keep this last
		&fsmNullEvent
};

//++++ Top State 2 events ++++

// Method prototypes
FSM_EVENT_HANDLER( MyTopFsmState2Evt_Entry );
FSM_EVENT_HANDLER( MyTopFsmState2Evt_EVT2 );

// Objects
FSM_EVENT( topState2Evt_Entry, EVT_FSM_ENTRY,  MyTopFsmState2Evt_Entry );
FSM_EVENT( topState2Evt_EVT2,  EVT_2,          MyTopFsmState2Evt_EVT2  );

// Event list array
FsmEvent* topState2_EventList[] = {
		&topState2Evt_Entry,
		&topState2Evt_EVT2,
		// keep this last
		&fsmNullEvent
};

//==================
//Define State Objects
//==================

//++++ sub states ++++
FSM_STATE( nested1State_1, &myNested1Fsm, NULL, nested1State1_EventList, "Nested1State1", MyFsmStateHandler );
FSM_STATE( nested1State_2, &myNested1Fsm, NULL, nested1State2_EventList, "Nested1State2", MyFsmStateHandler );
FSM_STATE( nested2State_1, &myNested2Fsm, NULL, nested2State1_EventList, "Nested2State1", MyFsmStateHandler );
FSM_STATE( nested2State_2, &myNested2Fsm, NULL, nested2State2_EventList, "Nested2State2", MyFsmStateHandler );

//++++ top states ++++

// Top state 1 Nested FSM list array
Fsm* topState1_NestedFsmList[] = {
		&myNested1Fsm,
		NULL
};
FSM_STATE( topState_1, &myTopFsm, topState1_NestedFsmList, topState1_EventList, "TopState1", MyFsmStateHandler );


// Top state 2 Nested FSM list array
Fsm* topState2_NestedFsmList[] = {
		&myNested2Fsm,
		NULL
};
FSM_STATE( topState_2, &myTopFsm, topState2_NestedFsmList, topState2_EventList, "TopState2", MyFsmStateHandler );

//===============
// Event Handlers
//===============

FSM_EVENT_HANDLER( MyTopFsmState1Evt_Entry )
{	// Note we init the FSM the first time only. FsmInit sends EVT_FSM_ENTRY, and the default state handler
	// sends EVT_FSM_SUPERSTATE_ENTRY. nested2State_1 ignores EVT_FSM_SUPERSTATE_ENTRY.
	if (NULL == myNested1Fsm.pState)
		MyFsmInit (&myNested1Fsm, &nested1State_1);	// first time enter to nested state 1

	return NULL;
}

FSM_EVENT_HANDLER( MyTopFsmState2Evt_Entry )
{	// Note we set the state, but dont call FsmInit. This avoids sending EVT_FSM_ENTRY and EVT_FSM_SUPERSTATE_ENTRY
	// each time we enter the top state. nested2State_1 treats both events the same.
	myNested2Fsm.pState = &nested2State_1;			// always enter to nested state 1
	return NULL;
}

FSM_EVENT_HANDLER( MyTopFsmState1Evt_EVT1 )     { pEvent->consumed = true; return &topState_2; }
FSM_EVENT_HANDLER( MyTopFsmState2Evt_EVT2 )     { pEvent->consumed = true; return &topState_1; }

FSM_EVENT_HANDLER( MyNestedFsm1State1Evt_Entry )
{
	FSM_RUN_LOG("%s,%s,entry_actions", pState->pFsm->name, pState->name);
	pEvent->consumed = true;
	return NULL;
}

FSM_EVENT_HANDLER( MyNestedFsm2State1Evt_Entry )
{
	FSM_RUN_LOG("%s,%s,entry_actions", pState->pFsm->name, pState->name);
	pEvent->consumed = true;
	return NULL;
}

FSM_EVENT_HANDLER( MyNestedFsm1State1Evt_EVT3 ) { pEvent->consumed = true; return &nested1State_2; }
FSM_EVENT_HANDLER( MyNestedFsm1State2Evt_EVT4 ) { pEvent->consumed = true; return &nested1State_1; }
FSM_EVENT_HANDLER( MyNestedFsm2State1Evt_EVT3 ) { pEvent->consumed = true; return &nested2State_2; }
FSM_EVENT_HANDLER( MyNestedFsm2State2Evt_EVT4 ) { pEvent->consumed = true; return &nested2State_1; }


/**************************************************************************************************/
// override of base class State Handler. Calls base class.
/**************************************************************************************************/
bool MyFsmStateHandler(FsmState *pState, int eventId)
{
	bool consumed;

	FSM_ENTER_LOG("%s,%s,%s", pState->pFsm->name, pState->name, EVT_NAME(eventId));

	consumed = FsmStateDefaultHandler(pState, eventId);

	FSM_EXIT_LOG("%s,%s,%s,%sconsumed", pState->pFsm->name, pState->name, EVT_NAME(eventId), (consumed==0? "not_" : ""));

	return consumed;

} // MyFsmStateHandleEvent

/**************************************************************************************************/
// FSM Initialization
/**************************************************************************************************/
void MyFsmInit (Fsm *pFsm, FsmState *pState)
{
	FSM_RUN_LOG("%s,%s,init", pState->pFsm->name, pState->name);

	FsmInit(pFsm, pState);

} // MyFsmInit

/**************************************************************************************************/
// FSM Entry Point
/**************************************************************************************************/
void MyFsmRun(int eventId)
{
	bool	consumed;

	consumed = FsmDispatch(&myTopFsm, eventId);

	if (!consumed)
	{
		FSM_EXIT_LOG("%s,%s,%s,ignored", myTopFsm.name, myTopFsm.pState->name, EVT_NAME(eventId));
	}

} // MyFsmRun

/**************************************************************************************************/
void TestFsm(void)
{
	MyFsmInit (&myTopFsm,  &topState_1);
	MyFsmRun(EVT_3);
	MyFsmRun(EVT_1);
	MyFsmRun(EVT_3);
	MyFsmRun(EVT_2);
	MyFsmRun(EVT_4);
	MyFsmRun(EVT_1);
	MyFsmRun(EVT_4);
}
