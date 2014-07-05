// fsm_test.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#include <conio.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void TestFsm(void);
void MyFsmInit (void);
void MyFsmRun(int eventId);
void PrintMyEventMenu(void);
bool SendEvent (int choice);

FILE * csvFile;

#ifdef __cplusplus
}
#endif

void Menu(void);

using namespace std;

DWORD WINAPI TopFsmThread( LPVOID lpParam );
HANDLE	hTopFsmThreadReady;
HANDLE  hTopFsmThread;
DWORD	dwTopFsmThreadId;

DWORD WINAPI Thread2( LPVOID lpParam );
HANDLE	hThread2Ready;
HANDLE  hThread2;
DWORD	dwThread2Id;

DWORD WINAPI Thread3( LPVOID lpParam );
HANDLE	hThread3Ready;
HANDLE  hThread3;
DWORD	dwThread3Id;

/**************************************************************************************************/
int main(int argc, char* argv[])
{
	csvFile = fopen("fsm_log.csv","w");
	if (NULL == csvFile)
	{
		printf("%s\n", strerror(errno));
		return -1;
	}

	//TestFsm();
	const HANDLE threadsReady[3] = {hThread2Ready, hTopFsmThreadReady, hThread3Ready};

	hTopFsmThreadReady  = CreateEvent(NULL,FALSE,FALSE, NULL);
	hThread2Ready       = CreateEvent(NULL,FALSE,FALSE, NULL);
	hThread3Ready       = CreateEvent(NULL,FALSE,FALSE, NULL);

	hTopFsmThread = CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
            TopFsmThread,			// thread function name
            0,						// argument to thread function 
            0,                      // use default creation flags 
            &dwTopFsmThreadId);		// returns the thread identifier 

	hThread2 = CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
            Thread2,				// thread function name
            0,						// argument to thread function 
            0,                      // use default creation flags 
            &dwThread2Id);		// returns the thread identifier 

	hThread3 = CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
            Thread3,		// thread function name
            0,						// argument to thread function 
            0,                      // use default creation flags 
            &dwThread3Id);	// returns the thread identifier 

	printf("waiting for threads\n");
	WaitForMultipleObjects(3, threadsReady, TRUE, INFINITE);

	Menu();

	fclose(csvFile);

	return 0;
}

/**************************************************************************************************/
DWORD WINAPI TopFsmThread( LPVOID lpParam )
{
	MSG msg;
	int	eventId;

	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	SetEvent(hTopFsmThreadReady);

	MyFsmInit();

	while (1)
	{
		GetMessage(&msg, NULL, 0, 0);
		eventId = msg.message-WM_USER;
		//printf ("%s got message id %d\n", __FUNCTION__ eventId);
		MyFsmRun(eventId);
	}

}

/**************************************************************************************************/
DWORD WINAPI Thread2( LPVOID lpParam )
{
	MSG msg;
	int	eventId;

	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	SetEvent(hThread2Ready);


	while (1)
	{
		GetMessage(&msg, NULL, 0, 0);
		eventId = msg.message-WM_USER;
		//printf ("%s got message id %d\n", __FUNCTION__ eventId);
	}

}

/**************************************************************************************************/
DWORD WINAPI Thread3( LPVOID lpParam )
{
	MSG msg;
	int	eventId;

	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	SetEvent(hThread3Ready);

	while (1)
	{
		GetMessage(&msg, NULL, 0, 0);
		eventId = msg.message-WM_USER;
		//printf ("%s got message id %d\n", __FUNCTION__ eventId);
	}

}

/**************************************************************************************************/
int SendMsgTopFsmThreadEvent(int eventId)
{
	BOOL result = PostThreadMessage(dwTopFsmThreadId,WM_USER+eventId, NULL, NULL);
	if (result == 0)
	{
		printf ("%s post message failed\n", __FUNCTION__);
		return -1;
	}

	return 0;
}

/**************************************************************************************************/
int SendMsgThread2Event(int eventId)
{
	BOOL result = PostThreadMessage(dwThread2Id,WM_USER+eventId, NULL, NULL);
	if (result == 0)
	{
		printf ("%s post message failed\n", __FUNCTION__);
		return -1;
	}

	return 0;
}

/**************************************************************************************************/
int SendMsgThread3Event(int eventId)
{
	BOOL result = PostThreadMessage(dwThread3Id,WM_USER+eventId, NULL, NULL);
	if (result == 0)
	{
		printf ("%s post message failed\n", __FUNCTION__);
		return -1;
	}

	return 0;
}

/**************************************************************************************************/
void EventMenu(void)
{
	bool 		done = false;
	int			choice;

	while (!done)
	{
		printf("\n");
		printf("\n");
		printf("\n");
		printf("==================\n");
		printf("**  Event Menu  **\n");
		printf("==================\n");

		printf("\nChoose option:\n");

		printf("[.] Exit this menu\n");

		PrintMyEventMenu();

		choice = _getch();

		printf(" %c\n", choice);

		if (!SendEvent(choice))
		switch(choice)
		{
			case '.':
				done = true;
				break;
		}

	}

} // EventMenu

/**************************************************************************************************/
void Menu (void)
{
	bool 		done = false;
	int			choice;

	while (!done)
	{
		printf("\nChoose option:\n");

		printf("[.] Exit this menu\n");
		printf("[0] Simple FSM test\n");
		printf("[1] Send FSM event\n");

		choice = _getch();
		if (isalpha(choice))
			choice = toupper(choice);

		switch(choice)
		{
			case '0':
				TestFsm();
				break;

			case '1':
				EventMenu();
				break;

			case '.':
				done = true;
				break;
		}
	}
}
