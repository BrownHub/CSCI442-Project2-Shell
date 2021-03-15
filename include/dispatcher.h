#ifndef _DISPATCHER_H
#define _DISPATCHER_H

#include <stdbool.h>

/**
 * shell_command_dispatcher() - run a shell command
 *
 * @input               The input, as a string.
 * @last_rv             The return value from the previous command.
 * @shell_should_exit   This output parameter will be set to true when
 *                      the command should result in the shell exiting.
 *
 * Return: the return status of the shell command, where zero
 * indicates success, and non-zero indicates failure.
 */
int shell_command_dispatcher(const char *input, int last_rv,
			     bool *shell_should_exit);

#endif /* _DISPATCHER_H */
