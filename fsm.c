/*
 *
 * File: fsm.c
 *
 * Hierarchical State Machine Framework 
 *
 *
 */

#include "fsm.h"

FsmEvent fsmNullEvent = {id : EVT_FSM_NULL, pfnEvtHandler : NULL};

/**************************************************************************************************/
// C implementation of OOP Hierarchical State Machine class
/**************************************************************************************************/
FsmEvent * FsmFindEvent( FsmEvent** pEventList, int eventId )
{
	int			i=0;

	while ((int)pEventList[i]->id != (int)EVT_FSM_NULL)
	{
		if (pEventList[i]->id == eventId)
			break;
		i++;
	}

	return pEventList[i];
}

/**************************************************************************************************/
bool FsmDispatch(Fsm *pFsm, int eventId)
{
	bool 		consumed;
	FsmStatePtr pState = pFsm->pState;

	if (NULL == pFsm)
	{
		FSM_LOG("!!!! FSM ERROR !!!! fsm undefined");
		return false;
	}

	if (NULL == pState)
	{
		FSM_LOG("!!!! FSM ERROR !!!! FSM %s: state undefined", pFsm->name);
		return false;
	}

	if (NULL == pState->pfnStateHandler)
	{
		FSM_LOG("!!!! FSM ERROR !!!! FSM %s: state %s handler undefined", pFsm->name, pState->name);
		return false;
	}

	consumed = (*pFsm->pState->pfnStateHandler) (pState, eventId);

	// Transition to next state if necessary
	if (pState->pNextState)
		FsmTransition(pState->pFsm, pState->pNextState);

	return consumed;
}


/**************************************************************************************************/
// FSM Base Class State Handler function
// returns true if no further processing for event (i.e., event consumed)
// sets pState->pNextState = NULL if no transition, otherwise points to next state
//
// NB: State transitions can't occur in Exit actions. That's gotta be
//     illegal, right? This implementation igores transitions in exit actions.

bool FsmStateDefaultHandler(FsmState *pState, int eventId)
{
	FsmState *		pNextState = NULL;
	FsmEvent *		pEvent;
	FsmEvtHandler	pfnEventHandler;
	bool			consumed = false;
	bool			isEntry = (EVT_FSM_ENTRY == eventId) || (EVT_FSM_SUPERSTATE_ENTRY == eventId);

	pEvent = FsmFindEvent( pState->eventList, eventId );	// returns pEvent->id == EVT_FSM_NULL if no handler found		
	pfnEventHandler = pEvent->pfnEvtHandler;
	pEvent->consumed = false;

	// Handle ENTRY events before passing to the substate; i.e., 
	// ENTRY events are handled in top-down order, always consume
	if ((pEvent->id == eventId) && isEntry )
	{
		pNextState = (pfnEventHandler == NULL ? NULL : (*pfnEventHandler)(pState, pEvent) );
		consumed = true;
	}

	// Pass the event to the substate.
	if (pState->nestedFsmList)
	{
		bool	subStateConsumed = false;
		int		subStateEventId = eventId;
		int		i=0;

		if (EVT_FSM_ENTRY == eventId)
			subStateEventId = EVT_FSM_SUPERSTATE_ENTRY;
		else if (EVT_FSM_EXIT ==  eventId)
			subStateEventId = EVT_FSM_SUPERSTATE_EXIT;

		while (pState->nestedFsmList[i] != NULL)
		{
			// ignore transitions in the nested FSM
			subStateConsumed = FsmDispatch(pState->nestedFsmList[i++], subStateEventId );
			consumed = consumed || subStateConsumed;
		}
	}

	// Handle non-ENTRY events not consumed by the substate
	// NB: EXIT events are handled in bottom-up order
	if ( (!consumed) && (pEvent->id == eventId) && (!isEntry) )
	{
		pNextState = (pfnEventHandler == NULL ? NULL : (*pfnEventHandler)(pState, pEvent) );
		// ignore transitions in Exit actions (or else we'll wind up in an infinite recursive loop...)
		pNextState = ( (EVT_FSM_EXIT == eventId) ? NULL : pNextState);

		consumed =  consumed || pEvent->consumed;
	}

	pState->pNextState = pNextState;

	return consumed;

} // FsmDispatch

/**************************************************************************************************/
void FsmTransition(Fsm *pFsm, FsmStatePtr pNextState)
{
	FsmDispatch(pFsm, EVT_FSM_EXIT);		// exit the source
	pFsm->pState = pNextState;				// change current state
	FsmDispatch(pFsm, EVT_FSM_ENTRY);		// enter the target
}

/**************************************************************************************************/
void FsmInit (Fsm *pFsm, FsmState *pState)
{
	pFsm->pState = pState;				// set initial state
	FsmDispatch(pFsm, EVT_FSM_ENTRY);	// enter the initial state

} // FsmInit



