#ifndef _STRING_BUF_H_
#define _STRING_BUF_H_

typedef struct _string_buf
{
	unsigned char *buf;
	int buf_size;
	int pos;
} string_buf_t;

string_buf_t* string_buf_new(int length);
void string_buf_delete(string_buf_t *string);

int string_buf_vsnprintf(string_buf_t *string, const char *format, ...);

#endif