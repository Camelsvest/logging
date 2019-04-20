#ifndef _LOGGING_H_
#define _LOGGING_H_

#ifdef __cplusplus
extern "C" {
#endif

int     logging_init(const char *filename);
void    logging_uninit();


typedef enum {
	LOGGING_VERBOS = 0,
	LOGGING_DEBUG,
	LOGGING_TRACE,
	LOGGING_WARNING,
	LOGGING_ERROR
} verbose_level_t;
void logging_set_verbose_level(verbose_level_t level);

int logging_verbos(const char *format, ...);
int logging_debug(const char *format, ...);
int logging_trace(const char *format, ...);
int logging_warning(const char *format, ...);
int logging_error(const char *format, ...);

int logging_verbos_dump(char *buf, int size);
int logging_debug_dump(char *buf, int size);
int logging_trace_dump(char *buf, int size);
int logging_warning_dump(char *buf, int size);
int logging_error_dump(char *buf, int size);

#ifdef __cplusplus
}
#endif

#endif