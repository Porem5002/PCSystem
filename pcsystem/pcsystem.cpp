#include "pcsystem.hpp"

bool PCSystem::init()
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

    shared = (PCShared*)MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
    if(shared == NULL)
    {
        _tprintf(_T("ERRO no mapeamento da memoria partilhada\n"));
        goto onError;
    }

    if(GetLastError() != ERROR_ALREADY_EXISTS)
    {
        memset(shared, 0, sizeof(PCShared));
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

int PCSystem::getNewProducerId()
{
    WaitForSingleObject(hInMutex, INFINITE);
    int id = shared->nprod++;
    ReleaseMutex(hInMutex);
    return id;
}

PCRequestStatus PCSystem::requestProductionSlot(PCItem** out_item_ptr)
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

int PCSystem::getNewConsumerId()
{
    WaitForSingleObject(hOutMutex, INFINITE);
    int id = shared->ncons++;
    ReleaseMutex(hOutMutex);
    return id;
}

PCRequestStatus PCSystem::requestConsumptionSlot(const PCItem** out_item_ptr)
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

void PCSystem::cleanup()
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