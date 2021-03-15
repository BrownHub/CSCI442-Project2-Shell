#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"

#define WHITESPACE_DELIMS " \f\n\r\t\v"
#define ALL_DELIMS WHITESPACE_DELIMS "<>|"

const char *parse_error_str[] = {
	[PARSE_SUCCESS] = "Success",
	[PARSE_ERR_COMMAND_WITHOUT_ARGS] = "Command has no arguments",
	[PARSE_ERR_MULTIPLE_INPUTS] = "Command has multiple inputs",
	[PARSE_ERR_MULTIPLE_OUTPUTS] = "Command has multiple outputs",
	[PARSE_ERR_MISSING_ARG_TO_FILE_OP] = "Missing operand to file operator",
	[PARSE_ERR_TOO_MANY_ARGS] =
	"The number of command line arguments is not supported by this shell",
};

static char *sized_strdup(const char *base, size_t len)
{
	char *buf;

	buf = malloc(len + 1);
	memcpy(buf, base, len);
	buf[len] = '\0';
	return buf;
}

static size_t consume_delims(const char **input, const char *delims)
{
	size_t n_delims = strspn(*input, delims);

	*input += n_delims;
	return n_delims;
}

static bool consume_string(const char **input, const char *search)
{
	size_t search_len;

	consume_delims(input, WHITESPACE_DELIMS);
	search_len = strlen(search);

	if (!strncmp(*input, search, search_len)) {
		*input += search_len;
		return true;
	}
	return false;
}

static char *consume_word(const char **input)
{
	char *result;
	size_t word_len;

	consume_delims(input, WHITESPACE_DELIMS);
	word_len = strcspn(*input, ALL_DELIMS);
	if (!word_len)
		return NULL;

	result = sized_strdup(*input, word_len);
	*input += word_len;
	return result;
}

enum parse_error parse_input(const char *input, struct command **pipeline_out)
{
	struct command cmd;
	enum parse_error rv = PARSE_SUCCESS;
	size_t args = 0;

	*pipeline_out = NULL;
	memset(&cmd, 0, sizeof(cmd));
	for (;;) {
		if (consume_string(&input, ">>")) {
			if (cmd.output_type) {
				rv = PARSE_ERR_MULTIPLE_OUTPUTS;
				goto fail;
			}
			cmd.output_filename = consume_word(&input);
			if (!cmd.output_filename) {
				rv = PARSE_ERR_MISSING_ARG_TO_FILE_OP;
				goto fail;
			}
			cmd.output_type = COMMAND_OUTPUT_FILE_APPEND;
			continue;
		} else if (consume_string(&input, ">")) {
			if (cmd.output_type) {
				rv = PARSE_ERR_MULTIPLE_OUTPUTS;
				goto fail;
			}
			cmd.output_filename = consume_word(&input);
			if (!cmd.output_filename) {
				rv = PARSE_ERR_MISSING_ARG_TO_FILE_OP;
				goto fail;
			}
			cmd.output_type = COMMAND_OUTPUT_FILE_TRUNCATE;
			continue;
		} else if (consume_string(&input, "<")) {
			if (cmd.input_filename) {
				rv = PARSE_ERR_MULTIPLE_INPUTS;
				goto fail;
			}

			cmd.input_filename = consume_word(&input);
			if (!cmd.input_filename) {
				rv = PARSE_ERR_MISSING_ARG_TO_FILE_OP;
				goto fail;
			}
			continue;
		} else if (consume_string(&input, "|")) {
			if (cmd.output_type) {
				rv = PARSE_ERR_MULTIPLE_OUTPUTS;
				goto fail;
			}

			rv = parse_input(input, &cmd.pipe_to);
			if (rv)
				goto fail;
			if (!cmd.pipe_to) {
				rv = PARSE_ERR_COMMAND_WITHOUT_ARGS;
				goto fail;
			}
			if (cmd.pipe_to->input_filename) {
				free_parse_result(cmd.pipe_to);
				cmd.pipe_to = NULL;
				rv = PARSE_ERR_MULTIPLE_INPUTS;
				goto fail;
			}
			cmd.output_type = COMMAND_OUTPUT_PIPE;
			break;
		}

		if (args >= ARGS_MAX - 1) {
			rv = PARSE_ERR_TOO_MANY_ARGS;
			goto fail;
		}

		cmd.argv[args] = consume_word(&input);
		if (!cmd.argv[args])
			break;
		args++;
	}

	if (!args) {
		if (cmd.input_filename || cmd.output_type) {
			rv = PARSE_ERR_COMMAND_WITHOUT_ARGS;
			goto fail;
		}
		return PARSE_SUCCESS;
	}

	*pipeline_out = malloc(sizeof(struct command));
	memcpy(*pipeline_out, &cmd, sizeof(cmd));
	return PARSE_SUCCESS;

fail:
	free(cmd.input_filename);
	free(cmd.output_filename);
	for (size_t i = 0; i < args; i++)
		free(cmd.argv[i]);
	return rv;
}

void free_parse_result(struct command *parse_result)
{
	if (parse_result) {
		for (char **p = parse_result->argv; *p; p++)
			free(*p);
		free(parse_result->input_filename);
		if (parse_result->output_type == COMMAND_OUTPUT_PIPE)
			free_parse_result(parse_result->pipe_to);
		else
			free(parse_result->output_filename);
		free(parse_result);
	}
}
