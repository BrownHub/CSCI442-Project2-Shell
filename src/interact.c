#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "parser.h"
#include "interact.h"

/**
 * maybe_add_history - Add to history only if string has length and
 * string does not start with whitespace
 */
static void maybe_add_history(const char *string)
{
	if (string[0] == '\0' || isspace(string[0]))
		return;
	add_history(string);
}

#define PROMPT_FMT "%s@%s %s %s $ "

char *default_prompt_generator(int last_return_code)
{
	char user_buf[256];
	char hostname_buf[256] = { 0 };
	char cwd_buf[PATH_MAX];
	const char *user = user_buf;
	const char *hostname = hostname_buf;
	const char *cwd = cwd_buf;
	char *prompt;
	size_t prompt_sz;

	if (getlogin_r(user_buf, sizeof(user_buf)) < 0) {
		fprintf(stderr, "Unable to get current username: %s\n",
			strerror(errno));
		user = "???";
	}

	if (gethostname(hostname_buf, sizeof(hostname_buf) - 1) < 0) {
		fprintf(stderr, "Unable to get current hostname: %s\n",
			strerror(errno));
		hostname = "???";
	}

	if (!getcwd(cwd_buf, sizeof(cwd_buf))) {
		fprintf(stderr, "Unable to get the current directory: %s\n",
			strerror(errno));
		cwd = "???";
	}

	prompt_sz = strnlen(user, sizeof(user_buf) - 1) +
		    strnlen(hostname, sizeof(hostname_buf) - 1) +
		    strnlen(cwd, sizeof(cwd_buf) - 1) + sizeof(PROMPT_FMT);
	prompt = malloc(prompt_sz);
	snprintf(prompt, prompt_sz, PROMPT_FMT, user, hostname, cwd,
		 last_return_code == 0 ? ":)" : ":(");
	return prompt;
}

int interact(char *(*prompt_generator)(int last_return_code),
	     int (*dispatcher)(const char *line, int last_rv,
			       bool *shell_should_exit))
{
	char *line = NULL;
	char *expanded_line = NULL;
	char *prompt;
	int history_rv;
	int last_return = 0;
	bool shell_should_exit;

	rl_catch_signals = 1;
	rl_set_signals();

	using_history();
	read_history(NULL);

	for (;;) {
		prompt = prompt_generator(last_return);
		line = readline(prompt);
		free(prompt);
		if (!line)
			line = strdup("exit");
		history_rv = history_expand(line, &expanded_line);

		if (history_rv != 0)
			fprintf(stderr, "%s\n", expanded_line);

		if (history_rv < 0) {
			maybe_add_history(line);
			continue;
		} else {
			maybe_add_history(expanded_line);
		}

		if (history_rv == 2)
			continue;

		shell_should_exit = false;
		last_return = dispatcher(expanded_line, last_return,
					 &shell_should_exit);
		free(line);
		free(expanded_line);

		if (shell_should_exit)
			return last_return;
	}
}
