#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "logging.h"

void help(char *filename)
{
	printf("Usage:\r\n");
	printf("%s [Filename].\r\n", filename);
	printf("\t -- Printf a TEXT file via background logging thread.\r\n");
	printf("\t -- You must specify a text file as parameter.\r\n");
	printf("\r\n");
}


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
		printf("Failed to open file: %s, error: %s\r\n", argv[1], strerror(errno));
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

