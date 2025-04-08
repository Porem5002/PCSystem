#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif

#include <windows.h>
#include <tchar.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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

template<class TItem, class TUserData>
struct PCShared
{
	TUserData user_data;
	int in, out;
	TItem items[MAX_ITEM_COUNT];
};

template<class TItem, class TUserData>
class PCSystem
{
	bool initialized;

	HANDLE hInitEvent;
	HANDLE hFinishEvent;

	HANDLE hInMutex;
	HANDLE hOutMutex;

	HANDLE hFreeSem;
	HANDLE hOccupiedSem;

	HANDLE hMap;
	PCShared<TItem, TUserData>* shared;

	static void printError(DWORD error)
	{
		TCHAR msg[100];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, msg, _countof(msg), NULL);
		_tprintf(_T("  %s\n"), msg);
	}
public:
	PCSystem() { memset(this, 0, sizeof(PCSystem)); }
	void stop() { SetEvent(hFinishEvent); }

	TUserData* getUserData() { return &shared->user_data; }
	void releaseProductionSlot() { ReleaseSemaphore(hOccupiedSem, 1, NULL); }
	void releaseConsumptionSlot() { ReleaseSemaphore(hFreeSem, 1, NULL); }

	bool init(const TUserData* init_value = NULL)
	{
		assert(!initialized);

		hInitEvent = CreateEvent(NULL, TRUE, FALSE, INIT_EVENT_NAME);
		if(hInitEvent == NULL)
		{
			_tprintf(_T("ERRO na criação do evento de inicialização\n"));
			goto onError;
		}

		hFinishEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if(hFinishEvent == NULL)
		{
			_tprintf(_T("ERRO na criação do event de finalização\n"));
			goto onError;
		}

		hInMutex = CreateMutex(NULL, FALSE, IN_MUTEX_NAME);
		if(hInMutex == NULL)
		{
			_tprintf(_T("ERRO na criação do mutex de input\n"));
			goto onError;
		}

		hOutMutex = CreateMutex(NULL, FALSE, OUT_MUTEX_NAME);
		if(hOutMutex == NULL)
		{
			_tprintf(_T("ERRO na criação do mutex de output\n"));
			goto onError;
		}

		hFreeSem = CreateSemaphore(NULL, MAX_ITEM_COUNT, MAX_ITEM_COUNT, FREE_SEM_NAME);
		if(hFreeSem == NULL)
		{
			_tprintf(_T("ERRO na criação do semaforo de slots livres\n"));
			goto onError;
		}

		hOccupiedSem = CreateSemaphore(NULL, 0, MAX_ITEM_COUNT, OCCUPIED_SEM_NAME);
		if(hOccupiedSem == NULL)
		{
			_tprintf(_T("ERRO na criação do semaforo de slots ocupados\n"));
			goto onError;
		}

		hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(PCSystem), SHARED_MEM_NAME);
		if(hMap == NULL)
		{
			_tprintf(_T("ERRO na criação da memoria partilhada\n"));
			goto onError;
		}

		shared = (decltype(shared))MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
		if(shared == NULL)
		{
			_tprintf(_T("ERRO no mapeamento da memoria partilhada\n"));
			goto onError;
		}

		if(GetLastError() != ERROR_ALREADY_EXISTS)
		{
			shared->in = 0;
			shared->out = 0;
			
			if(init_value != NULL)
				shared->user_data = *init_value;
			else
				memset(&shared->user_data, 0, sizeof(shared->user_data));

			SetEvent(hInitEvent);
		}

		WaitForSingleObject(hInitEvent, INFINITE);
		initialized = true;
		return true;
	onError:
		printError(GetLastError());
		cleanup();
		return false;
	}

	PCRequestStatus requestProductionSlot(TItem** out_item_ptr)
	{
		assert(out_item_ptr != NULL);

		*out_item_ptr = NULL;

		HANDLE objsToCheck[2] = { hFinishEvent, hFreeSem };
		DWORD waitResult = WaitForMultipleObjects(2, objsToCheck, 0, INFINITE);
		if (waitResult == WAIT_OBJECT_0) return PCRequestStatus::NO_MORE;
		
		WaitForSingleObject(hInMutex, INFINITE);
		int in = shared->in;
		shared->in = (shared->in + 1) % MAX_ITEM_COUNT;
		ReleaseMutex(hInMutex);
		
		*out_item_ptr = &shared->items[in];
		return PCRequestStatus::ACQUIRED;
	}

	PCRequestStatus requestConsumptionSlot(const TItem** out_item_ptr)
	{
		assert(out_item_ptr != NULL);

		*out_item_ptr = NULL;

		HANDLE objsToCheck[2] = { hFinishEvent, hOccupiedSem };
		DWORD waitResult = WaitForMultipleObjects(2, objsToCheck, 0, INFINITE);
		if(waitResult == WAIT_OBJECT_0) return PCRequestStatus::NO_MORE;

		WaitForSingleObject(hOutMutex, INFINITE);
		int out = shared->out;
		shared->out = (shared->out + 1) % MAX_ITEM_COUNT;
		ReleaseMutex(hOutMutex);

		*out_item_ptr = &shared->items[out];
		return PCRequestStatus::ACQUIRED;
	}

	void cleanup()
	{
		assert(initialized);
		initialized = false;

		if(hInitEvent != NULL) CloseHandle(hInitEvent);
		if(hFinishEvent != NULL) CloseHandle(hFinishEvent);
		if(hInMutex != NULL) CloseHandle(hInMutex);
		if(hOutMutex != NULL) CloseHandle(hOutMutex);
		if(hFreeSem != NULL) CloseHandle(hFreeSem);
		if(hOccupiedSem != NULL) CloseHandle(hOccupiedSem);

		if(shared != NULL) UnmapViewOfFile(shared);
		if(hMap) CloseHandle(hMap);

		memset(this, 0, sizeof(PCSystem));
	}
};
