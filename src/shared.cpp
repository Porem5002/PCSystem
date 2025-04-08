#include "shared.hpp"

#include <stdio.h>
#include <io.h>
#include <fcntl.h>

void set_correct_char_io_mode()
{
#ifdef _UNICODE
	(void)_setmode(_fileno(stdin), _O_WTEXT);
	(void)_setmode(_fileno(stdout), _O_WTEXT);
	(void)_setmode(_fileno(stderr), _O_WTEXT);
#endif
}