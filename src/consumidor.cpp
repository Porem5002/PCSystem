#define _CRT_SECURE_NO_WARNINGS

#include "shared.hpp"

DWORD WINAPI consume(void* data)
{
	ThreadData* threadData = (decltype(threadData))data;

	ProducerConsumer* pc = threadData->pc;
	HANDLE hMutexUserData = threadData->hUserDataMutex;

	int con_count = 0;
	int con_sum = 0;

	UserData* user_data = pc->getUserData();
	WaitForSingleObject(hMutexUserData, INFINITE);
	int con_id = user_data->nconsumers++;
	ReleaseMutex(hMutexUserData);

	_tprintf(_T("Consumer C%d started...\n"), con_id);

	while(true)
	{
		const Item* pItem;
		PCRequestStatus status = pc->requestConsumptionSlot(&pItem);
		if(status == PCRequestStatus::NO_MORE) break;
			
		int value = pItem->valor;
		_tprintf(_T("Do P%d "), pItem->id);
		pc->releaseConsumptionSlot();

		con_count++;
		con_sum += value;
		_tprintf(_T("C%d consumiu %d\n"), con_id, value);
	}

	_tprintf(_T("C%d consumiu %d itens, somando um valor acumulado de %d\n"), con_id, con_count, con_sum);
	return 0;
}

int _tmain(int argc, const TCHAR** argv)
{
	set_correct_char_io_mode();

	ProducerConsumer pc;
	if(!pc.init())
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

	HANDLE hThread = CreateThread(NULL, 0, consume, &threadData, 0, NULL);
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

	CloseHandle(hThread);
	pc.cleanup();
	return 0;
}