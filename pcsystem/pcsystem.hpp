#pragma once

#include <windows.h>
#include <tchar.h>
#include <assert.h>

#include <stdio.h>

#define INIT_EVENT_NAME _T("PCSystem_InitEvent")

#define IN_MUTEX_NAME _T("PCSystem_InMutex")
#define OUT_MUTEX_NAME _T("PCSystem_OutMutex")

#define FREE_SEM_NAME _T("PCSystem_FreeSem")
#define OCCUPIED_SEM_NAME _T("PCSystem_OccupiedSem")

#define SHARED_MEM_NAME _T("PCSystem_SharedMem")
#define MAX_ITEM_COUNT 10

enum class PCRequestStatus
{
	NO_MORE,
	ACQUIRED,
};

typedef struct
{
	int id;
	int valor;
} PCItem;

typedef struct
{
	int nprod;
	int ncons;

	int in;
	int out;

	PCItem items[MAX_ITEM_COUNT];
} PCShared;

struct PCSystem
{
	bool initialized;

	HANDLE hInitEvent;
	HANDLE hFinishEvent;

	HANDLE hInMutex;
	HANDLE hOutMutex;

	HANDLE hFreeSem;
	HANDLE hOccupiedSem;

	HANDLE hMap;
	PCShared* shared;

	void printError(DWORD error)
	{
		TCHAR msg[100];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, msg, _countof(msg), NULL);
		_tprintf(_T("  %s\n"), msg);
	}
public:
	PCSystem()
	{
		memset(this, 0, sizeof(PCSystem));
	}

	bool init();
	void stop() { SetEvent(hFinishEvent); }

	int getNewProducerId();
	PCRequestStatus requestProductionSlot(PCItem** out_item_ptr);
	void releaseProductionSlot() { ReleaseSemaphore(hOccupiedSem, 1, NULL); }

	int getNewConsumerId();
	PCRequestStatus requestConsumptionSlot(const PCItem** out_item_ptr);
	void releaseConsumptionSlot() { ReleaseSemaphore(hFreeSem, 1, NULL); }

	void cleanup();
};
