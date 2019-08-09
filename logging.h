#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <string.h>

#ifdef __WIN32_WINNT
    #define FILENAME(x) (strrchr(x,'\\')?strrchr(x,'\\')+1:x)
#else
    #define FILENAME(x) (strrchr(x,'/')?strrchr(x,'/')+1:x)
#endif

#define __FILENAME__     (FILENAME(__FILE__))

#define ENTER_CLASS_FUNCTION(class_name) logging_verbos("%s:%u\t Enter %s->%s\r\n", __FILENAME__, __LINE__, class_name, __FUNCTION__)
#define EXIT_CLASS_FUNCTION(class_name)  logging_verbos("%s:%u\t Exit %s->%s\r\n", __FILENAME__, __LINE__, class_name, __FUNCTION__)

#define ENTER_FUNCTION  logging_verbos("%s:%u\t Enter %s\r\n", __FILENAME__, __LINE__, __FUNCTION__)
#define EXIT_FUNCTION   logging_verbos("%s:%u\t Exit %s\r\n", __FILENAME__, __LINE__, __FUNCTION__);

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