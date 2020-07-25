#define __NO_STDIO_INLINES
#include "stdioint.h"

void clearerr(FILE *__f)
{
	__f->_IO_error = 0;
	__f->_IO_eof = 0;
}
