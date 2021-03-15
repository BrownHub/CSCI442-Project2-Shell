#ifndef _PARSER_H
#define _PARSER_H

/**
 * The maximum number of arguments supported by the parser, including
 * room for the NULL terminator.  Note: this is not a limitation of
 * Linux, but this parser.  execve() can support many more arguments
 * than this.
 */
#define ARGS_MAX 256

enum command_output_type {
	COMMAND_OUTPUT_STDOUT,
	COMMAND_OUTPUT_FILE_TRUNCATE,
	COMMAND_OUTPUT_FILE_APPEND,
	COMMAND_OUTPUT_PIPE,
};

/**
 * The result of parsing a command.  This linked-list structure
 * represents a pipeline of commands.  See the comments below for
 * documentantion on each field.
 */
struct command {
	/* The collected arguments. */
	char *argv[ARGS_MAX];

	/*
	 * The input file, or NULL if none.
	 * Note: it is not valid to have an input file on the
	 * receiving end of a pipe.
	 */
	char *input_filename;

	/*
	 * The output type.
	 *
	 * COMMAND_OUTPUT_STDOUT:
	 *     Output to the terminal.
	 * COMMAND_OUTPUT_FILE_TRUNCATE:
	 *     Output to the file specified by output_filename.
	 *     Overwrite any existing contents.
	 * COMMAND_OUTPUT_FILE_APPEND:
	 *     Output to the file specified by output_filename.
	 *     Append to any existing contents.
	 * COMMAND_OUTPUT_PIPE:
	 *     Forward the output to the input of the command
	 *     specified by pipe_to.
	 */
	enum command_output_type output_type;
	union {
		/* When COMMAND_OUTPUT_FILE_*, this is set. */
		char *output_filename;
		/* When COMMAND_OUTPUT_PIPE, this is set. */
		struct command *pipe_to;
	};
};

/**
 * The result of a parsing.
 *
 * PARSE_SUCCESS indicates the command was successfully parsed,
 * whereas PARSE_ERR_* indicate there was a semantical issue with the
 * input.
 */
enum parse_error {
	PARSE_SUCCESS,
	PARSE_ERR_COMMAND_WITHOUT_ARGS,
	PARSE_ERR_MULTIPLE_INPUTS,
	PARSE_ERR_MULTIPLE_OUTPUTS,
	PARSE_ERR_MISSING_ARG_TO_FILE_OP,
	PARSE_ERR_TOO_MANY_ARGS,
};

/**
 * This array can be used to translate an "enum parse_error" to a
 * human-readable string.
 */
extern const char *parse_error_str[];

/**
 * parse_input() - parse a shell command
 *
 * @input:        The shell command, as a string, to parse.
 * @pipeline_out: An output parameter of the resultant pipeline.  This
 *                pointer will be set to NULL on a parse error.  This
 *                value should always be passed to free_parse_result()
 *                after usage is completed.
 *
 * Return: PARSE_SUCCESS upon successful parse, or a relevant error
 * upon failure.
 */
enum parse_error parse_input(const char *input, struct command **pipeline_out);

/**
 * free_parse_result() - clean up memory allocated from parse_input
 *
 * @parse_result: The pointer pipeline_out was set to.
 */
void free_parse_result(struct command *parse_result);

#endif /* _PARSER_H */
