
#define	FSM_TRACE		1	// undefine or set to 0 to disable fsm tracing
#define _FSM_EXAMPLE_C_

#include "fsm.h"

void MyFsmInit (void);

// State Handler override function
bool MyFsmStateHandler(FsmState *pState, int eventId);


//==================
//Define Fsm Objects
//==================

FSM(fsm_Top    , "Top"    , NULL, NULL, NULL );	// instantiate the top level (superstate) FSM
FSM(fsm_Nested1, "Nested1", NULL, NULL, NULL );	// instantiate a nested FSM
FSM(fsm_Nested2, "Nested2", NULL, NULL, NULL );	// instantiate a nested FSM

//==============================
//Define Event objects and lists
//==============================

//++++ Nested FSM 1 State 1 events ++++

// Method prototypes
FSM_EVENT_HANDLER( Nested1_State1_Entry );
FSM_EVENT_HANDLER( Nested1_State1_EVT3 );

// Objects
FSM_EVENT( evt_Nested1_State1_Entry, EVT_FSM_ENTRY, Nested1_State1_Entry );
FSM_EVENT( evt_Nested1_State1_EVT3,  EVT_3,         Nested1_State1_EVT3  );

// Event list array
FsmEvent* eventList_Nested1_State1[] = {
		&evt_Nested1_State1_Entry,		// note superstate entry events are ignored, resume where we left off
		&evt_Nested1_State1_EVT3,
		// keep this last
		&fsmNullEvent
};

//++++ Nested FSM 1 State 2 events ++++

// Method prototypes
FSM_EVENT_HANDLER( Nested1_State2_EVT4 );

// Objects
FSM_EVENT( evt_Nested1_State2_EVT4, EVT_4,  Nested1_State2_EVT4 );

// Event list array
FsmEvent* eventList_Nested1_State2[] = {
		&evt_Nested1_State2_EVT4,			// note superstate entry events are ignored, resume where we left off
		// keep this last
		&fsmNullEvent
};

//++++ Nested FSM 2 State 1 events ++++

// Method prototypes
FSM_EVENT_HANDLER( Nested2_State1_Entry );
FSM_EVENT_HANDLER( Nested2_State1_EVT3  );

// Objects
// note superstate entry events are treated just like normal entry events
FSM_EVENT( evt_Nested2_State1_Entry,           EVT_FSM_ENTRY,             Nested2_State1_Entry );
FSM_EVENT( evt_Nested2_State1_SuperstateEntry, EVT_FSM_SUPERSTATE_ENTRY,  Nested2_State1_Entry );
FSM_EVENT( evt_Nested2_State1_EVT3,            EVT_3,                     Nested2_State1_EVT3  );

// Event list array
FsmEvent* eventList_Nested2_State1[] = {
		&evt_Nested2_State1_Entry,
		&evt_Nested2_State1_SuperstateEntry,
		&evt_Nested2_State1_EVT3,
		// keep this last
		&fsmNullEvent
};

//++++ Nested FSM 2 State 2 events ++++

// Method prototypes
FSM_EVENT_HANDLER( Nested2_State2_EVT4 );

// Objects
FSM_EVENT( evt_Nested2_State2_EVT4, EVT_4,  Nested2_State2_EVT4 );

// Event list array
FsmEvent* eventList_Nested2_State2[] = {
		&evt_Nested2_State2_EVT4,		// note superstate entry events are ignored - we always enter through state_Nested1_State1
		// keep this last
		&fsmNullEvent
};

//++++ Top State 1 events ++++

// Method prototypes
FSM_EVENT_HANDLER( Top_State1_Entry );
FSM_EVENT_HANDLER( Top_State1_EVT1 );

// Objects
FSM_EVENT( evt_Top_State1_Entry, EVT_FSM_ENTRY,  Top_State1_Entry );
FSM_EVENT( evt_Top_State1_EVT1,  EVT_1,          Top_State1_EVT1  );

// Event list array
FsmEvent* eventList_Top_State1[] = {
		&evt_Top_State1_Entry,
		&evt_Top_State1_EVT1,
		// keep this last
		&fsmNullEvent
};

//++++ Top State 2 events ++++

// Method prototypes
FSM_EVENT_HANDLER( Top_State2_Entry );
FSM_EVENT_HANDLER( Top_State2_EVT2 );

// Objects
FSM_EVENT( evt_Top_State2_Entry, EVT_FSM_ENTRY,  Top_State2_Entry );
FSM_EVENT( evt_Top_State2_EVT2,  EVT_2,          Top_State2_EVT2  );

// Event list array
FsmEvent* eventList_Top_State2[] = {
		&evt_Top_State2_Entry,
		&evt_Top_State2_EVT2,
		// keep this last
		&fsmNullEvent
};

//==================
//Define State Objects
//==================

//++++ sub states ++++
FSM_STATE( state_Nested1_State1, &fsm_Nested1, NULL, eventList_Nested1_State1, "State1", MyFsmStateHandler );
FSM_STATE( state_Nested1_State2, &fsm_Nested1, NULL, eventList_Nested1_State2, "State2", MyFsmStateHandler );
FSM_STATE( state_Nested2_State1, &fsm_Nested2, NULL, eventList_Nested2_State1, "State1", MyFsmStateHandler );
FSM_STATE( state_Nested2_State2, &fsm_Nested2, NULL, eventList_Nested2_State2, "State2", MyFsmStateHandler );

//++++ top states ++++

// Top state 1 Nested FSM list array
Fsm* nestedFsmList_Top_State1[] = {
		&fsm_Nested1,
		NULL
};
FSM_STATE( state_Top_State1, &fsm_Top, nestedFsmList_Top_State1, eventList_Top_State1, "State1", MyFsmStateHandler );


// Top state 2 Nested FSM list array
Fsm* nestedFsmList_Top_State2[] = {
		&fsm_Nested2,
		NULL
};
FSM_STATE( state_Top_State2, &fsm_Top, nestedFsmList_Top_State2, eventList_Top_State2, "State2", MyFsmStateHandler );

//===============
// Event Handlers
//===============

FSM_EVENT_HANDLER( Top_State1_Entry )
{	// Note we init the FSM the first time only. FsmInit sends EVT_FSM_ENTRY, and the default state handler
	// sends EVT_FSM_SUPERSTATE_ENTRY. state_Nested2_State1 ignores EVT_FSM_SUPERSTATE_ENTRY.
	if (NULL == fsm_Nested1.pState)
		FsmInit (&fsm_Nested1, &state_Nested1_State1);	// first time enter to nested state 1

	return NULL;
}

FSM_EVENT_HANDLER( Top_State2_Entry )
{	// Note we set the state, but dont call FsmInit. This avoids sending EVT_FSM_ENTRY and EVT_FSM_SUPERSTATE_ENTRY
	// each time we enter the top state. state_Nested2_State1 treats both events the same.
	fsm_Nested2.pState = &state_Nested2_State1;			// always enter to nested state 1
	return NULL;
}

FSM_EVENT_HANDLER( Top_State1_EVT1 )     { pEvent->consumed = true; return &state_Top_State2; }
FSM_EVENT_HANDLER( Top_State2_EVT2 )     { pEvent->consumed = true; return &state_Top_State1; }

FSM_EVENT_HANDLER( Nested1_State1_Entry )
{
	FSM_RUN_LOG("%s,%s,entry_actions", pState->pFsm->name, pState->name);
	pEvent->consumed = true;
	return NULL;
}

FSM_EVENT_HANDLER( Nested2_State1_Entry )
{
	FSM_RUN_LOG("%s,%s,entry_actions", pState->pFsm->name, pState->name);
	pEvent->consumed = true;
	return NULL;
}

FSM_EVENT_HANDLER( Nested1_State1_EVT3 ) { pEvent->consumed = true; return &state_Nested1_State2; }
FSM_EVENT_HANDLER( Nested1_State2_EVT4 ) { pEvent->consumed = true; return &state_Nested1_State1; }
FSM_EVENT_HANDLER( Nested2_State1_EVT3 ) { pEvent->consumed = true; return &state_Nested2_State2; }
FSM_EVENT_HANDLER( Nested2_State2_EVT4 ) { pEvent->consumed = true; return &state_Nested2_State1; }


/**************************************************************************************************/
// override of base class State Handler. Calls base class.
/**************************************************************************************************/
bool MyFsmStateHandler(FsmState *pState, int eventId)
{
	bool consumed;

	FSM_ENTER_LOG("%s,%s,%s", pState->pFsm->name, pState->name, FSM_EVT_NAME(eventId));

	consumed = FsmStateDefaultHandler(pState, eventId);

	FSM_EXIT_LOG("%s,%s,%s,%sconsumed", pState->pFsm->name, pState->name, FSM_EVT_NAME(eventId), (consumed==0? "not_" : ""));

	return consumed;

} // MyFsmStateHandleEvent

/**************************************************************************************************/
// FSM Initialization
/**************************************************************************************************/
void MyFsmInit (void)
{
	Fsm			*pFsm = &fsm_Top;
	FsmState	*pState = &state_Top_State1;

	FSM_RUN_LOG("%s,%s,init", pState->pFsm->name, pState->name);

	FsmInit(pFsm, pState);

} // MyFsmInit

/**************************************************************************************************/
// FSM Entry Point
/**************************************************************************************************/
void MyFsmRun(eFsmEvent eventId)
{

	FsmRun(&fsm_Top, eventId);

} // MyFsmRun

/**************************************************************************************************/
void TestFsm(void)
{
	MyFsmInit ();
	MyFsmRun(EVT_3);
	MyFsmRun(EVT_1);
	MyFsmRun(EVT_3);
	MyFsmRun(EVT_2);
	MyFsmRun(EVT_4);
	MyFsmRun(EVT_1);
	MyFsmRun(EVT_4);
}

/**************************************************************************************************/
void PrintMyEventMenu(void)
{
	int i;

	for (i=0; i<EVT_FSM_EOL; i++)
	{
		char menu_char;
		if (i<10)	   menu_char = '0'+i;
		else if (i<36) menu_char = 'a'+i-10;
		else if (i<62) menu_char = 'A'+i-36;
		else           menu_char = '*';		// too many, need to add more commands
		printf("[%c] Send %s\n", menu_char, FSM_EVT_NAME(i));
	}
}

#include "ctype.h"
/**************************************************************************************************/
// choice is menu character selected from list generated by PrintMyEventMenu
bool SendEvent (int choice)
{
	eFsmEvent	eventId;

	if (isdigit(choice))
		eventId = (eFsmEvent)(choice - '0');
	else if (isalpha(choice))
		eventId = (eFsmEvent)(choice < 'a' ? (choice - 'A' + 36) : (choice - 'a' + 10));
	else
		eventId = EVT_FSM_EOL;

	if ( IS_MY_EVENT(eventId) )
	{
		MyFsmRun(eventId);
		return true;
	}

	return false;
}
