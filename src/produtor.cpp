#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <time.h>

#include "shared.hpp"

DWORD WINAPI produce(void* data)
{
	ThreadData* threadData = (decltype(threadData))data;

	ProducerConsumer* pc = threadData->pc;
	HANDLE hUserDataMutex = threadData->hUserDataMutex;

	srand((unsigned)time(NULL));

	int prod_count = 0;
	
	UserData* user_data = pc->getUserData();
	WaitForSingleObject(hUserDataMutex, INFINITE);
	int prod_id = user_data->nproducers++;
	ReleaseMutex(hUserDataMutex);

	_tprintf(_T("Producer P%d started...\n"), prod_id);

	while(true)
	{
		Item* pItem;
		PCRequestStatus status = pc->requestProductionSlot(&pItem);
		if(status == PCRequestStatus::NO_MORE) break;

		int value = (rand() % 90) + 10;
		pItem->id = prod_id;
		pItem->valor = value;

		pc->releaseProductionSlot();

		prod_count++;
		_tprintf(_T("P%d produziu %d\n"), prod_id, value);
		
		int secondsToPause = (rand() % 3) + 2;
		Sleep(secondsToPause * 1000);
	}

	_tprintf(_T("P%d produziu %d itens\n"), prod_id, prod_count);
	return 0;
}

int main(int argc, const TCHAR** argv)
{
	set_correct_char_io_mode();

	ProducerConsumer pc;
	if (!pc.init())
	{
		_tprintf(_T("Não foi possível inicializar o sistema produtor-consumidor!\n"));
		ExitProcess(1);
	}

	HANDLE hUserDataMutex = CreateMutex(NULL, FALSE, USER_DATA_MUTEX_NAME);
	if(hUserDataMutex == NULL)
	{
		_tprintf(_T("Não foi possível criar o mutex para user data!\n"));
		ExitProcess(1);
	}

	ThreadData threadData = {};
	threadData.pc = &pc;
	threadData.hUserDataMutex = hUserDataMutex;

	HANDLE hThread = CreateThread(NULL, 0, produce, &threadData, 0, NULL);
	if(hThread == NULL)
	{
		pc.cleanup();
		ExitProcess(1);
	}

	_tprintf(_T("Pressione qualquer caracter para sair\n"));
	(void)_gettch();
	pc.stop();
	_tprintf(_T("A sair...\n"));

	WaitForSingleObject(hThread, INFINITE);

	pc.cleanup();
	return 0;
}