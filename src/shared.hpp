#pragma once

#include "pcsystem/pcsystem.hpp"

#define USER_DATA_MUTEX_NAME _T("UserDataMutex")

struct Item
{
	int id;
	int valor;
};

struct UserData
{
	int nconsumers;
	int nproducers;
};

using ProducerConsumer = PCSystem<Item, UserData>;

struct ThreadData
{
	ProducerConsumer* pc;
	HANDLE hUserDataMutex;
};

void set_correct_char_io_mode();