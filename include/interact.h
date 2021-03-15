#ifndef _INTERACT_H
#define _INTERACT_H

#include <stdbool.h>

/**
 * default_prompt_generator() - generate a prompt for the shell
 *
 * The prompt generated is of the following format:
 *    ${USER}@${HOSTNAME} ${PWD} ${HAPPY_OR_SAD}
 * Followed by a "$" and space at the end.
 *
 * @last_return_code:    HAPPY_OR_SAD will be ":)" if zero, ":("
 *                       otherwise.
 *
 * Return: A newly allocated buffer with the prompt string.  This
 * should be freed by the caller.
 */
char *default_prompt_generator(int last_return_code);

/**
 * interact() - run a prompt loop with readline and history enabled
 *
 * @prompt_generator:    A callback function to generate prompt strings.
 *                       This function should create newly-allocated
 *                       buffers with a prompt string, and it will be
 *                       freed by this function.
 * @dispatcher:          A callback function to execute inputs.  This
 *                       function should set the "shell_should_exit"
 *                       output parameter to true when the interact
 *                       loop should end.
 *
 * Return: after the interact loop ends, the last integer value
 * returned by the dispatcher.
 */
int interact(char *(*prompt_generator)(int last_return_code),
	     int (*dispatcher)(const char *line, int last_rv,
			       bool *shell_should_exit));

#endif /* _INTERACT_H */
