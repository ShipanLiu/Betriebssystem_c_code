/**
 * @file  sem.h
 * @brief Semaphore implementation for the synchronization of POSIX threads.
 *
 * This module implements counting P/V semaphores suitable for the
 * synchronization of POSIX threads. POSIX mutexes and condition variables are
 * utilized to implement the semaphor operations.
 */

#ifndef SEM_H
#define SEM_H


/** Opaque type of a semaphore. */
typedef struct SEM SEM;

/**
 * @brief Creates a new semaphore.
 *
 * This function creates a new semaphore. If an error occurs during the
 * initialization, the implementation frees all resources already allocated by
 * then and sets @c errno to an appropriate value.
 *
 * It is legal to initialize the semaphore with a negative value. If this is the
 * case, in order to reset the semaphore counter to zero, the V-operation must be
 * performed @c (-initVal) times.
 *
 * @param initVal The initial value of the semaphore.
 * @return Handle for the created semaphore, or @c NULL if an error occurred.
 */
SEM *semCreate(int initVal);

/**
 * @brief Destroys a semaphore and frees all associated resources.
 * @param sem Handle of the semaphore to destroy. If a @c NULL pointer is
 *            passed, the implementation does nothing.
 */
void semDestroy(SEM *sem);

/**
 * @brief P-operation.
 *
 * Attempts to decrement the semaphore value by 1. If the semaphore value is not a
 * positive number, the operation blocks until a V-operation increments the value
 * and the P-operation succeeds.
 *
 * @param sem Handle of the semaphore to decrement.
 */
void P(SEM *sem);

/**
 * @brief V-operation.
 *
 * Increments the semaphore value by 1 and notifies P-operations that are
 * blocked on the semaphore of the change.
 *
 * @param sem Handle of the semaphore to increment.
 */
void V(SEM *sem); 


#endif /* SEM_H */
