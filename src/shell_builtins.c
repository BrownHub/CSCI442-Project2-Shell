#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <readline/history.h>

#include "shell_builtins.h"

static int exit_builtin(const char *const argv[], int last_rv,
			bool *shell_should_exit)
{
	int status = last_rv;

	if (argv[1]) {
		if (argv[2]) {
			fprintf(stderr, "%s: too many arguments\n", argv[0]);
			return 1;
		}
		if (sscanf(argv[1], "%d", &status) != 1) {
			fprintf(stderr,
				"%s: numeric argument required, got \"%s\"\n",
				argv[0], argv[1]);
			return 1;
		}
	}

	*shell_should_exit = true;
	return status;
}

static int cd_builtin(const char *const argv[], int last_rv, bool *unused)
{
	const char *dir;

	dir = getenv("HOME");
	if (argv[1]) {
		dir = argv[1];
		if (argv[2]) {
			fprintf(stderr, "usage: %s [dir]\n", argv[0]);
			return 1;
		}
	}

	if (chdir(dir) < 0) {
		fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
		return 1;
	}

	return 0;
}

static int help_builtin(const char *const argv[], int last_rv, bool *unused)
{
	printf("This is CSCI-442 shell!\n\n");
	printf("Builtin commmands are:\n ");
	for (size_t i = 0; builtin_commands[i].name; i++)
		printf(" %s", builtin_commands[i].name);
	printf("\n");
	return 0;
}

static int history_builtin(const char *const argv[], int last_rv, bool *unused)
{
	if (!argv[1]) {
		HIST_ENTRY **histlst = history_list();

		for (int i = 1; *histlst; i++, histlst++)
			printf("%4d %s\n", i, (*histlst)->line);
	} else if (!argv[2] && !strcmp(argv[1], "-c")) {
		clear_history();
	} else {
		fprintf(stderr, "usage: %s [-c]\n", argv[0]);
		return -1;
	}

	return 0;
}

struct builtin_command builtin_commands[] = {
	{ "cd", cd_builtin },
	{ "exit", exit_builtin },
	{ "help", help_builtin },
	{ "history", history_builtin },
	{ NULL },
};
