#define _CRT_SECURE_NO_WARNINGS

#include "pcsystem/pcsystem.hpp"

#include <time.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

DWORD WINAPI produce(void* data)
{
	PCSystem* pc = (PCSystem*)data;

	srand(time(NULL));

	int prod_count = 0;
	int prod_id = pc->getNewProducerId();

	while(true)
	{
		PCItem* pItem;
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
#ifdef _UNICODE
	(void)_setmode(_fileno(stdin), _O_WTEXT);
	(void)_setmode(_fileno(stdout), _O_WTEXT);
	(void)_setmode(_fileno(stderr), _O_WTEXT);
#endif
	PCSystem pc;
	if (!pc.init())
	{
		_tprintf(_T("Não foi possível inicializar o sistema produtor-consumidor!\n"));
		ExitProcess(1);
	}

	HANDLE hThread = CreateThread(NULL, 0, produce, &pc, 0, NULL);
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