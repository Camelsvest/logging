#ifndef _LOGGING_H_
#define _LOGGING_H_

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

#define LOGV(fmt, ...) logging(LOGGING_VERBOS, fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...) logging(LOGGING_DEBUG, fmt, ##__VA_ARGS__)
#define LOGT(fmt, ...) logging(LOGGING_TRACE, fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) logging(LOGGING_WARNING, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) logging(LOGGING_ERROR, fmt, ##__VA_ARGS__)	

int logging(verbose_level_t level, const char *format, ...);

#define logging_verbos(fmt, ...) 	LOGV(fmt, ##__VA_ARGS__)
#define logging_debug(fmt, ...)		LOGD(fmt, ##__VA_ARGS__)
#define logging_trace(fmt, ...) 	LOGT(fmt, ##__VA_ARGS__)
#define logging_warning(fmt, ...) 	LOGW(fmt, ##__VA_ARGS__)
#define logging_error(fmt, ...) 	LOGE(fmt, ##__VA_ARGS__)

#define DUMPV(buf, size)	dump(LOGGING_VERBOS, buf, size)
#define DUMPD(buf, size)	dump(LOGGING_DEBUG, buf, size)
#define DUMPT(buf, size)	dump(LOGGING_TRACE, buf, size)
#define DUMPW(buf, size)	dump(LOGGING_WARNING, buf, size)
#define DUMPE(buf, size)	dump(LOGGING_ERROR, buf, size)

int dump(verbose_level_t level, unsigned char *buf, int size);

#define logging_verbos_dump(buf, size)		dump(LOGGING_VERBOS, buf, size)
#define logging_debug_dump(buf, size)		dump(LOGGING_DEBUG, buf, size)
#define logging_trace_dump(buf, size)		dump(LOGGING_TRACE, buf, size)
#define logging_warning_dump(buf, size)		dump(LOGGING_WARNING, buf, size)
#define logging_error_dump(buf, size)		dump(LOGGING_ERROR, buf, size)

#endif
