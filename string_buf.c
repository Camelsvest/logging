#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "string_buf.h"

string_buf_t* string_buf_new(int length)
{
	string_buf_t *buf = NULL;
	
	if (length > 0)
	{
		buf = (string_buf_t *)malloc(sizeof(string_buf_t));
		buf->pos = 0;
		if (buf != NULL)
		{
			buf->buf = (unsigned char *)malloc(length);
			if (buf->buf != NULL)
			{
				buf->buf_size = length;
			}
			else
			{
				free(buf);
				buf = NULL;
			}
		}

	}

	return buf;
}


void string_buf_delete(string_buf_t *string)
{
	if (string != NULL)
	{
		free(string->buf);
		free(string);
	}
}


int string_buf_vsnprintf(string_buf_t *string, const char *format, ...)
{
	va_list args;
	int available_bytes, ret = -1;

	if (string != NULL)
	{
		if (string->pos < string->buf_size )
		{
			available_bytes = string->buf_size - string->pos;
			
			va_start(args, format);
			ret = vsnprintf((char *)string->buf + string->pos, available_bytes, format, args);
			va_end(args);

			if (ret > 0)
			{
				string->pos += ret;
			}
		}
	}

	return -1;
}