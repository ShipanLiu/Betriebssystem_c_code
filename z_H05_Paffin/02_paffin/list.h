/**
 * @file  list.h
 * @brief List implementation to store and retrieve lines of files.
 *
 * This module implements a list to store lines of files. The list is **not**
 * multithreading safe and it is the responsibility of the user to synchronise
 * calls list functions.
 */

#ifndef LIST_H
#define LIST_H

/**
 * @brief Enqueues a line of a file into the list.
 *
 * The list stores the path of the file, the line and the corresponding line
 * number. This function internally allocates memory and therefore may fail.
 *
 * @param path The path of the file (must not be NULL).
 * @param line The line contents (must not be NULL).
 * @line_number The corresponding line_number to line.
 * @return Returns 0 on success and -1 on error.
 */
int enqueue(const char *path, const char *line, unsigned long line_number);






/**
 * @brief Dequeues and formats an element of the list.
 *
 * Dequeues an element of the list and returns an already formatted string
 * containing the path, line number, and line.
 *
 * It is the responsibility of the user to free() the returned string.
 *
 * @return Returns a string of the dequeued list item or NULL if the list is empty
 */
char *dequeue(void);

/**
 * @brief Dequeues and fancy formats an element of the list.
 *
 * Dequeues an element of the list and returns an already fancy formatted string
 * containing the path, line number, and line.
 *
 * As the fancy formatting includes memory allocation this function may fail.
 *
 * It is the responsibility of the user to free() the returned string.
 *
 * @string Pointer to a string. The functions allocates sufficient memory for the string
 * behind the pointer copies the formatted output there. If the return value is 0 and
 * string points to NULL, the list was empty. string must not be NULL (however *string
 * can be NULL).
 * @pattern String with a search pattern to highlight within the line or NULL
 * @return Returns 0 on success and -1 on error. An empty list is indicated
 * by an return value of 0 and *string == NULL.
 */
int dequeue_fancy(char **string, const char *pattern);

#endif /* SEM_H */
