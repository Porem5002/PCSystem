#define _CRT_SECURE_NO_WARNINGS

#include "pcsystem/pcsystem.hpp"

#include <stdio.h>
#include <io.h>
#include <fcntl.h>

DWORD WINAPI consume(void* data)
{
	PCSystem* pc = (PCSystem*)data;

	int con_count = 0;
	int con_sum = 0;
	int con_id = pc->getNewConsumerId();

	while(true)
	{
		const PCItem* pItem;
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
#ifdef _UNICODE
	(void)_setmode(_fileno(stdin), _O_WTEXT);
	(void)_setmode(_fileno(stdout), _O_WTEXT);
	(void)_setmode(_fileno(stderr), _O_WTEXT);
#endif
	PCSystem pc;
	if(!pc.init())
	{
		_tprintf(_T("Não foi possível inicializar o sistema produtor-consumidor!\n"));
		ExitProcess(1);
	}
	
	HANDLE hThread = CreateThread(NULL, 0, consume, &pc, 0, NULL);
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