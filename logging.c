#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>

#include "logging.h"
#include "bipbuffer.h"
#include "string_buf.h"

#define TRUE 1
#define FALSE 0
#define BOOL int

#define LOG_MAX_BUFSIZE	2048
#define LOG_MAX_LINESIZE 256

typedef struct logging_context
{
	FILE	*logging_file;
	int	verbose_level;
	
	pthread_t	thread_id;
	pthread_mutex_t	*mutex;
	pthread_cond_t	*cond;

	BOOL		running;
	bipbuf_t	*buf;

} logging_ctx_t;
static logging_ctx_t *logging_ctx = NULL;


static void * thread_proc(void *arg)
{	
	logging_ctx_t*	ctx;
	int bytes;
	unsigned char *buf, ch;

//	struct timespec abs;

	ctx = (logging_ctx_t *)arg;
	if (ctx != NULL && ctx->buf != NULL)
	{
		pthread_mutex_lock(ctx->mutex);
		fprintf(ctx->logging_file, "\n*** Logging thread start to run. ***\n\n");
		while (ctx->running)
		{
//			clock_gettime(CLOCK_REALTIME, &abs);
//			abs.tv_nsec += 10000000; // 10 ms
//			while (abs.tv_nsec > 1000000000) {
//			abs.tv_sec += 1;
//			abs.tv_nsec -= 1000000000;
//
//			pthread_cond_timedwait(ctx->cond, ctx->mutex, &abs);

			pthread_cond_wait(ctx->cond, ctx->mutex);

			bytes = 0;
			buf = bipbuf_get_contiguous_block(ctx->buf, &bytes);
			if (buf && bytes > 0) 
			{
				ch = *(buf + bytes);	// backup
				*(buf + bytes) = '\0';

				fprintf(ctx->logging_file, "%s", buf);

				*(buf + bytes) = ch; // restore
				bipbuf_decommit_block(ctx->buf, bytes);
			}

		}

		fprintf(ctx->logging_file, "\n\n*** Logging thread stop running. ***\n");

		pthread_mutex_unlock(ctx->mutex);

	}

	return 0;
}


int logging_init(const char *filename)
{
	struct stat fdstat;
	BOOL OK = TRUE;
	
	if (logging_ctx != NULL)
	{
		logging_uninit();
	}

	assert(logging_ctx == NULL);
	logging_ctx = (logging_ctx_t *)calloc(1, sizeof(logging_ctx_t));
	if (logging_ctx != NULL)
	{
		logging_ctx->verbose_level = LOGGING_VERBOS;

		logging_ctx->buf = bipbuf_new(LOG_MAX_BUFSIZE);
		logging_ctx->mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));			
		logging_ctx->cond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));

		if (logging_ctx->buf != NULL && logging_ctx->mutex != NULL && logging_ctx->cond != NULL)
		{
			pthread_mutex_init(logging_ctx->mutex, NULL);
			pthread_cond_init(logging_ctx->cond, NULL);

			if (filename != NULL && strlen(filename) > 0)
			{
				logging_ctx->logging_file = fopen(filename, "w+");
				if (logging_ctx->logging_file == NULL)
				{
					fprintf(stderr, "Failed to open log file:%s, errno = %d\r\n", filename, errno);
					OK = FALSE;
				}
			}
			else
			{
				logging_ctx->logging_file = stdout;
			}
			fstat(fileno(logging_ctx->logging_file), &fdstat);
			fprintf(stdout, "logging file: st_blksize=%d, st_blocks=%d\n", (int)fdstat.st_blksize, (int)fdstat.st_blocks);

			if (OK == TRUE)
			{
				logging_ctx->running = TRUE;
				if (pthread_create(&logging_ctx->thread_id, NULL, thread_proc, logging_ctx) == 0)
					return 0;
			}
		}
	}


	// release memeory
	if (logging_ctx)
	{
		if (logging_ctx->logging_file)
		{
			fclose(logging_ctx->logging_file);
		}
		free(logging_ctx->mutex);
		free(logging_ctx->cond);
		free(logging_ctx);
		logging_ctx = NULL;
	}

	return -1;
}


void logging_uninit()
{
	void *ret = NULL;

	if (logging_ctx)
	{
		pthread_mutex_lock(logging_ctx->mutex);
		if (logging_ctx->running == TRUE)
		{
			logging_ctx->running = FALSE;
		}
		pthread_mutex_unlock(logging_ctx->mutex);
		pthread_cond_signal(logging_ctx->cond);

		pthread_join(logging_ctx->thread_id, &ret);
		free(ret);

		if (logging_ctx->cond != NULL)
		{
			pthread_cond_destroy(logging_ctx->cond);
			free(logging_ctx->cond);
		}

		if (logging_ctx->mutex != NULL)
		{
			pthread_mutex_destroy(logging_ctx->mutex);
			free(logging_ctx->mutex);
		}

		if (logging_ctx->logging_file)
		{
			fclose(logging_ctx->logging_file);
		}

		free(logging_ctx);
		logging_ctx = NULL;		
	}
}


void logging_set_verbose_level(verbose_level_t level)
{
	if (logging_ctx != NULL && logging_ctx->mutex != NULL)
	{
		pthread_mutex_lock(logging_ctx->mutex);
		logging_ctx->verbose_level = level;
		pthread_mutex_unlock(logging_ctx->mutex);
	}
}


static int logging_print(verbose_level_t level, const char *format, va_list args)
{
	int size = -1;
	char *buffer = NULL;

	if (logging_ctx != NULL && logging_ctx->mutex != NULL)
	{
		pthread_mutex_lock(logging_ctx->mutex);
		if (logging_ctx->verbose_level <= level)
		{
			buffer = (char *)bipbuf_reserve(logging_ctx->buf, LOG_MAX_LINESIZE, &size);
			if (buffer != NULL && size > 0)
			{
				size = vsnprintf(buffer, size, format, args);
				if (size > 0)
				{
					bipbuf_commit(logging_ctx->buf, size);
				}
				else
				{
					bipbuf_commit(logging_ctx->buf, 0);
					size = -1;
				}
			}
		}	
		pthread_mutex_unlock(logging_ctx->mutex);

		if (size > 0)
		{
			pthread_cond_signal(logging_ctx->cond);
		}		
	}
	
	return size;
}


static int logging_vsprint(verbose_level_t level, const char *format, ...)
{
	int ret;
	va_list args;

	va_start(args, format);
	ret = logging_print(level, format, args);
	va_end(args);

	return ret;
}


int logging_verbos(const char *format, ...)
{
	int ret;
	va_list args;
 
	va_start(args, format);
	ret = logging_print(LOGGING_VERBOS, format, args);
	va_end(args);

	return ret;
}


int logging_debug(const char *format, ...)
{
	int ret;
	va_list args;
 
	va_start(args, format);
	ret = logging_print(LOGGING_DEBUG, format, args);
	va_end(args);	

	return ret;
}


int logging_trace(const char *format, ...)
{
	int ret;
	va_list args;
 
	va_start(args, format);
	ret = logging_print(LOGGING_TRACE, format, args);
	va_end(args);	

	return ret;
}


int logging_warning(const char *format, ...)
{
	int ret;
	va_list args;
 
	va_start(args, format);
	ret = logging_print(LOGGING_WARNING, format, args);
	va_end(args);	

	return ret;
}


int logging_error(const char *format, ...)
{
	int ret;
	va_list args;
 
	va_start(args, format);
	ret = logging_print(LOGGING_ERROR, format, args);
	va_end(args);	

	return ret;
}


static int logging_dump(verbose_level_t level, unsigned char *buffer, int size)
{
	int           i;      // used to keep track of line lengths
	unsigned char *line;  // used to print char version of data
	unsigned char ch;     // also used to print char version of data

	string_buf_t  *stringbuf;
	int           ret;
	int           length = 0;

	stringbuf = string_buf_new(256); // Be careful for this size; ensure it is bigger enough to contain one line

	if (stringbuf != NULL)
	{
		ret = string_buf_vsnprintf(stringbuf, "DUMP:\r\n");
		if (ret > 0)
		{
			length += ret;

			i = 0; 
			line = buffer; 

			ret = string_buf_vsnprintf(stringbuf, "%08X | ", (unsigned int)buffer);
			length += ret;

			while (size-- > 0)
			{
				ret = string_buf_vsnprintf(stringbuf, "%02X ", *buffer++); // print each char
				assert(ret > 0);
				length += ret;

				if (!(++i % 16) || (size == 0 && i % 16))
				{ 
					// if we come to the end of a line...

					// if this is the last line, print some fillers.
					if (size == 0)
					{
						while (i++ % 16)
						{ 
							ret = string_buf_vsnprintf(stringbuf, "__ ");
							assert(ret > 0);
							length += ret;
						}
					}

					ret = string_buf_vsnprintf(stringbuf, "| ");
					assert(ret > 0);
					length += ret;

					while (line < buffer) // print the character version
					{  
						ch = *line++;
						ret = string_buf_vsnprintf(stringbuf, "%c", (ch < 33 || ch == 255) ? 0x2E : ch);
						assert(ret > 0);
						length += ret;
					}

					// If we are not on the last line, prefix the next line with the address.
					if (size > 0)
					{
						logging_vsprint(level, "%s", stringbuf->buf);
						stringbuf->pos = 0;	// reset

						ret = string_buf_vsnprintf(stringbuf, "\n%08X | ", (int)buffer);
						assert(ret > 0);
						length += ret;
					}
				}
			}

			ret = string_buf_vsnprintf(stringbuf, "\n\n");			
			assert(ret > 0);
			length += ret;
		}

		logging_vsprint(level, "%s", stringbuf->buf);

		string_buf_delete(stringbuf);
	}

	return length;	
}


int logging_verbos_dump(char *buf, int size)
{
	return logging_dump(LOGGING_VERBOS, (unsigned char *)buf, size);
}


int logging_debug_dump(char *buf, int size)
{
	return logging_dump(LOGGING_DEBUG, (unsigned char *)buf, size);
}


int logging_trace_dump(char *buf, int size)
{
	return logging_dump(LOGGING_TRACE, (unsigned char *)buf, size);
}


int logging_warning_dump(char *buf, int size)
{
	return logging_dump(LOGGING_WARNING, (unsigned char *)buf, size);
}


int logging_error_dump(char *buf, int size)
{
	return logging_dump(LOGGING_ERROR, (unsigned char *)buf, size);
}


#ifdef LOGGING_TEST_ENABLE

void help(char *filename)
{
	printf("Usage:\r\n");
	printf("%s [Filename].\r\n", filename);
	printf("\t -- Printf a TEXT file via background logging thread.\r\n");
	printf("\t -- You must specify a text file as parameter.\r\n");
	printf("\r\n");
}

#include <unistd.h>

int main(int argc, char *argv[])
{
	FILE *fp;
	char buf[80];
	int ret, bytes = -1;

	if (argc < 2)
	{
		help(argv[0]);
		return -1;
	}

	fp = fopen(argv[1], "r");
	if (fp == NULL)
	{
		printf("Failed to open file: %s.\r\n", argv[1]);
		return -1;
	}

	if (argc >= 3)
	{
		logging_init(argv[2]);
	}
	else
		logging_init("");

	bytes = fread(buf, 1, sizeof(buf) - 1, fp);
	while (bytes > 0)
	{
		buf[bytes] = '\0';
		ret = logging_trace("%s", buf);
		assert(ret == bytes);

		bytes = fread(buf, 1, sizeof(buf) - 1, fp);

		usleep(1000);
	}
//	logging_trace("\r\nFile end. bytes = %d.\r\n", bytes);

	logging_uninit();
	return 0;
}

#endif