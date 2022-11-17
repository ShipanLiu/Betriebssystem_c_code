#ifndef PLIST_H
#define PLIST_H

#include <sys/types.h>

/** \file plist.h
 *
 *  \brief Linked List for maintaining process id - command line pairs.
 *
 *  This implementation is not thread safe.
 */

/**
 *  \brief Inserts a new pid-command line pair into the linked list.
 *
 * During the insert operation, the passed commandLine is copied to
 * an internally allocated buffer. The caller may free or otherwise
 * reuse the memory occupied by commandLine after return from
 * insertElement.
 *
 *  \param pid The process id of the pair that is to be inserted.
 *  \param commandLine The commandLine corresponding to the process with id pid.
 *
 *  \return pid on success, negative value on error
 *    \retval pid  success
 *    \retval  -1  a pair with the given pid already exists
 *    \retval  -2  insufficient memory to complete the operation
 */
int insertElement(pid_t pid, const char *commandLine);

/**
 *  \brief Remove a specific pid-command line pair from the linked list.
 *
 * The linked list is searched for a pair with the given pid. If such a pair is
 * found, the '\\0'-terminated command line is copied to the buffer provided by
 * the caller. If the length of the command line exceeds the size of the
 * buffer, only the first (buffersize-1) characters of the command line are
 * copied to the buffer and terminated with the '\\0' character. Upon
 * completion, removeElement deallocates all resources that were occupied by
 * the removed pair.
 *
 *  \param pid The process id of the pair that is to be removed.
 *  \param commandLineBuffer A buffer provided by the caller that the '\\0'-terminated commandline is written to.
 *  \param bufferSize The size of commandLineBuffer.
 *
 *  \return actual length of the command line on success, negative value on error.
 *    \retval >0  success, actual length of the command line
 *    \retval -1  a pair with the given pid does not exist
 */
int removeElement(pid_t pid, char *commandLineBuffer, size_t bufferSize);

/**
 *  \brief Invokes a callback function on each element in the list.
 *
 * 	The callback function is passed the pid-command line pair for the current
 * 	list element. The callback function shall return 0 to request further
 * 	processing of the list. Any other return value will cause the early
 * 	termination of the list walk. The callback function must not modify the
 * 	process list.
 *
 *  \param callback Pointer to the function to be invoked for each list element.
 */
void walkList(int (*callback) (pid_t, const char *));

#endif
