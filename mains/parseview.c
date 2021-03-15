#define _POSIX_C_SOURCE 200809L

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interact.h"
#include "parser.h"

static char *get_prompt(int last_rv)
{
	return strdup("parseview> ");
}

static void ntabs(int num_tabs)
{
	for (int i = 0; i < num_tabs; i++)
		printf("\t");
}

static void dump_str(const char *str)
{
	if (!str) {
		printf("NULL");
		return;
	}

	printf("\"");
	for (int i = 0; str[i]; i++) {
		switch (str[i]) {
		case '\"':
			printf("\\\"");
		case '\n':
			printf("\\n");
		case '\t':
			printf("\\t");
		case '\r':
			printf("\\r");
		case '\v':
			printf("\\v");
		default:
			printf("%c", str[i]);
		}
	}
	printf("\"");
}

static void dump_cmd(struct command *cmd, int level)
{
	if (!cmd) {
		printf("NULL\n");
		return;
	}
	printf("{\n");

	/* argv */
	ntabs(level + 1);
	printf(".argv = {\n");
	ntabs(level + 2);
	for (int i = 0; cmd->argv[i]; i++) {
		dump_str(cmd->argv[i]);
		printf(", ");
	}
	printf("NULL,\n");
	ntabs(level + 1);
	printf("},\n");

	/* input_file */
	ntabs(level + 1);
	printf(".input_filename = ");
	dump_str(cmd->input_filename);
	printf(",\n");

	/* output_type */
	ntabs(level + 1);
	printf(".output_type = ");
	switch (cmd->output_type) {
	case COMMAND_OUTPUT_STDOUT:
		printf("COMMAND_OUTPUT_STDOUT,\n");
		break;
	case COMMAND_OUTPUT_FILE_TRUNCATE:
		printf("COMMAND_OUTPUT_TRUNCATE,\n");
		break;
	case COMMAND_OUTPUT_FILE_APPEND:
		printf("COMMAND_OUTPUT_APPEND,\n");
		break;
	case COMMAND_OUTPUT_PIPE:
		printf("COMMAND_OUTPUT_PIPE,\n");
		break;
	}

	if (cmd->output_type == COMMAND_OUTPUT_PIPE) {
		ntabs(level + 1);
		printf(".pipe_to = &");
		dump_cmd(cmd->pipe_to, level + 1);
	} else if (cmd->output_type != COMMAND_OUTPUT_STDOUT) {
		ntabs(level + 1);
		printf(".output_filename = ");
		dump_str(cmd->output_filename);
		printf(",\n");
	}
	ntabs(level);
	printf("}%s\n", level ? "," : "");
}

static int show_parse(const char *input, int last_rv, bool *should_exit)
{
	struct command *cmd;
	enum parse_error err;

	if (!strcmp(input, "exit"))
		*should_exit = true;

	err = parse_input(input, &cmd);
	if (err) {
		printf("Parse error: %s\n", parse_error_str[err]);
	} else {
		printf("cmd = ");
		dump_cmd(cmd, 0);
		free_parse_result(cmd);
	}
	return err;
}

int main(int argc, char *argv[])
{
	interact(get_prompt, show_parse);
}
