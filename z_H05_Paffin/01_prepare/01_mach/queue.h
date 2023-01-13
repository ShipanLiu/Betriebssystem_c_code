/**
 * @file  queue.h
 * @brief Synchronized blocking queue implementation for the mach project.
 *
 * This module implements a synchronized blocking FIFO (first-in-first-out)
 * queue for the mach project to store commands to run, their output and
 * optional flags.
 */

#ifndef QUEUE_H
#define QUEUE_H

/**
 * @brief Initialize the queue.
 *
 * This function initializes the queue. It's undefined to call other functions
 * of this module before initializing the queue.
 *
 * If an error occurs during initialization, all already allocated resources
 * must be freed before returning an error.
 *
 * @return @c 0 on success, any other value on error (sets @c errno on error)
 */
int queue_init(void);
/**
 * @brief Destroy the queue.
 *
 * This functions deallocates all resources of the queue. queue_deinit()
 * assumes the queue is empty and will not remove existing elements. It's
 * undefined to call other functions of this module after calling this
 * function.
 */
void queue_deinit(void);

/**
 * @brief Add entry to the queue.
 *
 * Add a new entry with command @c cmd, output @c out and optional flags @c
 * flags to the queue. All values can be @c NULL respectively @c 0. @c flags
 * is optional and can be used to store additional attributes.
 *
 * The caller controls the lifespan of all pointers. The queue will not
 * duplicate or copy any arguments. It only stores the provided pointers.
 *
 * @param cmd command to run
 * @param out output of the command
 * @param flags additional flags (optional)
 * @return @c 0 on success, any other value on error (sets @c errno on error)
 */
int queue_put(char *cmd, char *out, int flags);
/**
 * @brief Remove entry from the queue.
 *
 * Removes an entry from the queue and writes its values to the given
 * pointers. The function waits until an element is available.
 *
 * Example:
 *
 * \code
 * char *cmd, *out;
 * int flags;
 * if (queue_get(&cmd, &out, &flags)) {
 *     // error handling ...
 * }
 * \endcode
 *
 * @param cmd command to run
 * @param out output of the command
 * @param flags additional flags (optional)
 * @return @c 0 on success, any other value on error (sets @c errno on error)
 */
int queue_get(char **cmd, char **out, int *flags);

#endif
