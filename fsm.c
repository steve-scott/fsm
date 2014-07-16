/*
 *
 * File: fsm.c
 *
 * Hierarchical State Machine Framework 
 *
 *
 */
#define _FSM_C_
#include "fsm.h"

FsmEvent fsmNullEvent = {DESIG_INIT(id,EVT_FSM_NULL), DESIG_INIT(pfnEvtHandler,NULL)};

/**************************************************************************************************/
// C implementation of OOP Hierarchical State Machine class
/**************************************************************************************************/

/**************************************************************************************************/
// begin test functions
/**************************************************************************************************/
// Functions used only for testing

#if FSM_TEST
/**************************************************************************************************/
int FsmInsertBefore(eFsmEvent eventId, eFsmEvent insertId)
{
	FsmQ	*pQ = gFsmQInsertBefore[eventId];
	int		result = FsmPutEvent(pQ, insertId);
	return	result;
}

/**************************************************************************************************/
int FsmInsertAfter(eFsmEvent eventId, eFsmEvent insertId)
{
	FsmQ	*pQ = gFsmQInsertAfter[eventId];
	int		result = FsmPutEvent(pQ, insertId);
	return	result;
}

/**************************************************************************************************/
void FsmDoInsertedEvents(FsmQ *pQ, Fsm *pFsm, int eventId)
{
	int	 insertEvent;

	while ((insertEvent = FsmGetEvent(pQ)) != EVT_FSM_NULL)
	{
		if (!FsmDispatch(pFsm, insertEvent))
			FSM_LOG("%s,%s,%s,ignored", pFsm->name, pFsm->pState->name, FSM_EVT_NAME(insertEvent))
	}
}

/**************************************************************************************************/
void FsmDoInsertedBefore(Fsm *pFsm, int eventId)
{
	FsmQ *pQ;

	if (eventId >= EVT_FSM_EOL)
		return;

	pQ = gFsmQInsertBefore[eventId];

	FsmDoInsertedEvents(pQ, pFsm, eventId);

}

/**************************************************************************************************/
void FsmDoInsertedAfter(Fsm *pFsm, int eventId)
{
	FsmQ *pQ;

	if (eventId >= EVT_FSM_EOL)
		return;

	pQ = gFsmQInsertAfter[eventId];

	FsmDoInsertedEvents(pQ, pFsm, eventId);
}

/**************************************************************************************************/
// default notify function does nothing
void FsmNotify(void)
{
}

#endif // FSM_TEST

/**************************************************************************************************/
// end test functions
/**************************************************************************************************/


/**************************************************************************************************/
int FsmDeferEvent(Fsm* pFsm, int eventId)
{
	int result = FsmPutEvent(pFsm->deferQ, eventId);

	return result;
}

/**************************************************************************************************/
int FsmRecallEvent(Fsm* pFsm, int *pEventId)
{
	int result;

	*pEventId = FsmGetEvent(pFsm->deferQ);
	result = FsmPutEvent(pFsm->recallQ, *pEventId);

	return result;
}

/**************************************************************************************************/
// Move an event to a queue
// Returns -1 if queue is full, else 0
int FsmPutEvent (FsmQ *q, int eventId)
{
	if (NULL == q)
		return 0;

	if (EVT_FSM_NULL == eventId)	// no event
		return 0;

	if (q->count >= q->size)	// queue full
	{
		FSM_LOG("Recall queue full - put event %d in queue failed", eventId);
		return -1;
	}

	q->eventId[q->tail++] = eventId;
	q->count++;

	if (q->tail >= q->size)
		q->tail = 0;

	return 0;

} // FsmPutEvent

/**************************************************************************************************/
// Retrieves an event from a queue.
// returns EVT_FSM_NULL if no events in the queue, else the next eventId in the queue
int FsmGetEvent (FsmQ *q)
{
	int	eventId;

	if (NULL == q)
		return EVT_FSM_NULL;

	if (q->count <= 0)
		return EVT_FSM_NULL;

	eventId = q->eventId[q->head++];
	q->count--;

	if (q->head >= q->size)
		q->head = 0;

	return eventId;

} // FsmGetEvent

/**************************************************************************************************/
FsmEvent * FsmFindEvent( FsmEvent** pEventList, int eventId )
{
	int		i=0;
	int		defaultEvt = -1;

	while (pEventList[i]->id != EVT_FSM_NULL)
	{
		if (pEventList[i]->id == eventId)
			break;
		else if (pEventList[i]->id == EVT_FSM_DEFAULT)
			defaultEvt = i;
		i++;
	}

	if ( (pEventList[i]->id == EVT_FSM_NULL) && (defaultEvt >= 0) )
	{
		i = defaultEvt;
		pEventList[i]->altId = eventId;
	}

	return pEventList[i];
}

/**************************************************************************************************/
bool FsmDispatch(Fsm *pFsm, int eventId)
{
	bool 		consumed;
	FsmStatePtr pState = pFsm->pState;

	if (EVT_FSM_NULL == eventId)	// no event
		return true;

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

	FSM_NOTIFY(pState, eventId);

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
	if ( (!consumed) && (pEvent->id != EVT_FSM_NULL) && (!isEntry) )
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

/**************************************************************************************************/
void FsmRun(Fsm *pFsm, int eventId)
{
	bool	consumed;
	int		nextEvent = eventId;

	FSM_INSERT_BEFORE(pFsm, eventId);

	do {
		consumed = FsmDispatch(pFsm, nextEvent);
		if (!consumed)
		{
			if (nextEvent >= EVT_FSM_EOL)
				FSM_LOG(",%s,%s,%d,ignored", pFsm->name, pFsm->pState->name, nextEvent)
			else
				FSM_LOG(",%s,%s,%s,ignored", pFsm->name, pFsm->pState->name, FSM_EVT_NAME(nextEvent))
		}

		// look for any any deferred events that have been recalled
		nextEvent = FsmGetEvent(pFsm->recallQ);

	} while (nextEvent != EVT_FSM_NULL);
	
	FSM_INSERT_AFTER(pFsm, eventId);

} // CoordFsmRun
