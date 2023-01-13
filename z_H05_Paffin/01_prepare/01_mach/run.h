/**
 * @file  run.h
 * @brief Run commands and return their output.
 */

#ifndef RUN_H
#define RUN_H

/**
 * @brief Run a new command and collect its output.
 *
 * This function forks and executes @cmd using the shell and waits for its
 * completion. No input is provided on stdin. The output (stdout and stderr)
 * is written to a newly allocated buffer.
 *
 * @param cmd The command to run using the shell.
 * @param out Output of the program, automatically allocated. The caller must
 *            manually free this buffer. On error NULL is written to @out or
 *            parts of the allocated output.
 * @return Exit code of the program if the program starts (errno not defined),
 *         negative value on any other error (errno set to the cause).
 */
int run_cmd(const char *cmd, char **out);

#endif
