#ifndef _SHELL_BUILTINS_H
#define _SHELL_BUILTINS_H

#include <stdbool.h>

/**
 * A builtin command.
 */
struct builtin_command {
	/* The name of the command to match argv[0]. */
	const char *name;

	/* The handler function. */
	int (*handler)(const char *const argv[], int last_rv,
		       bool *shell_should_exit);
};

/**
 * The global list of builtin commands.
 */
extern struct builtin_command builtin_commands[];

#endif /* _SHELL_BUILTINS_H */
