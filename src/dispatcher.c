#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "dispatcher.h"
#include "shell_builtins.h"
#include "parser.h"

/**
 * dispatch_external_command() - run a pipeline of commands
 *
 * @pipeline:   A "struct command" pointer representing one or more
 *              commands chained together in a pipeline.  See the
 *              documentation in parser.h for the layout of this data
 *              structure.  It is also recommended that you use the
 *              "parseview" demo program included in this project to
 *              observe the layout of this structure for a variety of
 *              inputs.
 *
 * Note: this function should not return until all commands in the
 * pipeline have completed their execution.
 *
 * Return: The return status of the last command executed in the
 * pipeline.
 */
static int dispatch_external_command(struct command *pipeline)
{
	/*
	 * Note: this is where you'll start implementing the project.
	 *
	 * It's the only function with a "TODO".  However, if you try
	 * and squeeze your entire external command logic into a
	 * single routine with no helper functions, you'll quickly
	 * find your code becomes sloppy and unmaintainable.
	 *
	 * It's up to *you* to structure your software cleanly.  Write
	 * plenty of helper functions, and even start making yourself
	 * new files if you need.
	 *
	 * For D1: you only need to support running a single command
	 * (not a chain of commands in a pipeline), with no input or
	 * output files (output to stdout only).  In other words, you
	 * may live with the assumption that the "input_file" field in
	 * the pipeline struct you are given is NULL, and that
	 * "output_type" will always be COMMAND_OUTPUT_STDOUT.
	 * 
	 * For D2: you'll extend this function to support input and
	 * output files, as well as pipeline functionality.
	 *
	 * Good luck!
	 */

	/*
	If input != null
		pipe input file into first command
	while output is pipe
		execute command
		pipe command to next command
	if output is redirect(truncate)
		if file does not exist
			make it
		redirect to overwrite file
	if output is redirect(append)
		if file does not exist
			make it
		redirect to append to file
	*/

	//Initialize variables
	struct command *the_pipe = pipeline;
	pid_t pid;
	int child_status;

	//Variables for pipeline
	int pipe_fd[2];
	pipe(pipe_fd);
	int fd_read = pipe_fd[0];
	int fd_write = pipe_fd[1];
	int fd_read_next;

	//Code for pipe commands
	if (the_pipe->output_type == COMMAND_OUTPUT_PIPE){
		if((pid = fork()) < 0){
			fprintf(stderr, "fork error\n");
		}
		else if (pid == 0){
			//Redirects input
			if (the_pipe->input_filename != NULL){
				int input_fd = open(the_pipe->input_filename,
					       	O_RDONLY, 0666);
				dup2(input_fd, STDIN_FILENO);
			}

			close(fd_read);
			dup2(fd_write, STDOUT_FILENO);

			if(execvp(the_pipe->argv[0], the_pipe->argv)){
				perror("Failed to execute");
				fprintf(stderr, "exec error\n");
			}
			exit(-1);
		}

		close(fd_write);

		if(waitpid(pid, &child_status, 0) != pid){
			fprintf(stderr, "waitpid error\n");
		}

		if(child_status != 0){
			return -1;
		}

		the_pipe = the_pipe->pipe_to;
	

		while (the_pipe->output_type == COMMAND_OUTPUT_PIPE){
	
			pipe(pipe_fd);
			fd_write = pipe_fd[1];
			fd_read_next = pipe_fd[0];	

			if((pid = fork()) < 0){
				fprintf(stderr, "fork error\n");
			}
			else if (pid == 0){
				dup2(fd_read, STDIN_FILENO);
				dup2(fd_write, STDOUT_FILENO);
	
				if(execvp(the_pipe->argv[0], the_pipe->argv)){
					perror("Failed to execute");
					fprintf(stderr, "exec error\n");
				}
				exit(-1);
			}

			if(waitpid(pid, &child_status, 0) != pid){
				fprintf(stderr, "waitpid error\n");
			}

			close(fd_write);
			close(fd_read);

			if(child_status != 0){
				return -1;
			}

			fd_read = fd_read_next;
		
			the_pipe = the_pipe->pipe_to;	

		}

		if((pid = fork()) < 0){
			fprintf(stderr, "fork error\n");
		}
		else if (pid == 0){
			//Redirects output
			if (the_pipe->output_type == COMMAND_OUTPUT_FILE_APPEND){
			int output_fd = open(the_pipe->output_filename,
				       	O_WRONLY | O_CREAT | O_APPEND , 0666);
			dup2(output_fd, STDOUT_FILENO);
			}
			else if (the_pipe->output_type == COMMAND_OUTPUT_FILE_TRUNCATE){
			int output_fd = open(the_pipe->output_filename,
				       	O_WRONLY | O_CREAT | O_TRUNC , 0666);
			dup2(output_fd, STDOUT_FILENO);
			}

			dup2(fd_read, STDIN_FILENO);

			if(execvp(the_pipe->argv[0], the_pipe->argv)){
				perror("Failed to execute");
				fprintf(stderr, "exec error\n");
			}
			exit(-1);
		}

		close(fd_read);

		if(waitpid(pid, &child_status, 0) != pid){
			fprintf(stderr, "waitpid error\n");
		}

		if(child_status != 0){
			return -1;
		}

		return 0;
	
	}

	//Code for non-piped command execution
	if((pid = fork()) < 0){
		fprintf(stderr, "fork error\n");
	}
	else if (pid == 0){
		//Redirects input
		if (the_pipe->input_filename != NULL){
			int input_fd = open(the_pipe->input_filename,
				       	O_RDONLY, 0666);
			dup2(input_fd, STDIN_FILENO);
		}

		//Redirects output
		if (the_pipe->output_type == COMMAND_OUTPUT_FILE_APPEND){
		int output_fd = open(the_pipe->output_filename,
			       	O_WRONLY | O_CREAT | O_APPEND , 0666);
		dup2(output_fd, STDOUT_FILENO);
		}
		else if (the_pipe->output_type == COMMAND_OUTPUT_FILE_TRUNCATE){
		int output_fd = open(the_pipe->output_filename,
			       	O_WRONLY | O_CREAT | O_TRUNC , 0666);
		dup2(output_fd, STDOUT_FILENO);
		}

		if(execvp(the_pipe->argv[0], the_pipe->argv)){
			perror("Failed to execute");
				
		}
		exit(-1);
	}

	close(fd_read);
	close(fd_write);

	if(waitpid(pid, &child_status, 0) != pid){
		fprintf(stderr, "waitpid error\n");
	}

	if(child_status != 0){
		return -1;
	}
	
	return 0;
}

/**
 * dispatch_parsed_command() - run a command after it has been parsed
 *
 * @cmd:                The parsed command.
 * @last_rv:            The return code of the previously executed
 *                      command.
 * @shell_should_exit:  Output parameter which is set to true when the
 *                      shell is intended to exit.
 *
 * Return: the return status of the command.
 */
static int dispatch_parsed_command(struct command *cmd, int last_rv,
				   bool *shell_should_exit)
{
	/* First, try to see if it's a builtin. */
	for (size_t i = 0; builtin_commands[i].name; i++) {
		if (!strcmp(builtin_commands[i].name, cmd->argv[0])) {
			/* We found a match!  Run it. */
			return builtin_commands[i].handler(
				(const char *const *)cmd->argv, last_rv,
				shell_should_exit);
		}
	}

	/* Otherwise, it's an external command. */
	return dispatch_external_command(cmd);
}

int shell_command_dispatcher(const char *input, int last_rv,
			     bool *shell_should_exit)
{
	int rv;
	struct command *parse_result;
	enum parse_error parse_error = parse_input(input, &parse_result);

	if (parse_error) {
		fprintf(stderr, "Input parse error: %s\n",
			parse_error_str[parse_error]);
		return -1;
	}

	/* Empty line */
	if (!parse_result)
		return last_rv;

	rv = dispatch_parsed_command(parse_result, last_rv, shell_should_exit);
	free_parse_result(parse_result);
	return rv;
}
