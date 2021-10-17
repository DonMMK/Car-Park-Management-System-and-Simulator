#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "sharedMemory.c"

/**
 * Controller: initialise a shared_object_t, creating a block of shared memory
 * with the designated name, and setting its storage capacity to the size of a
 * shared data block.
 *
 * PRE: n/a
 *
 * POST: shm_unlink has been invoked to delete any previous instance of the
 *       shared memory object, if it exists.
 * AND   The share name has been saved in shm->name.
 * AND   shm_open has been invoked to obtain a file descriptor connected to a
 *       newly created shared memory object with size equal to the size of a
 *       shared_data_t struct, with support for read and write operations. The
 *       file descriptor should be saved in shm->fd, regardless of the final outcome.
 * AND   ftruncate has been invoked to set the size of the shared memory object
 *       equal to the size of a shared_data_t struct.
 * AND   mmap has been invoked to obtain a pointer to the shared memory, and the
 *       result has been stored in shm->data, regardless of the final outcome.
 * AND   (this code is provided for you, don't interfere with it) The shared
 *       semaphore has been initialised to manage access to the shared buffer.
 * AND   Semaphores have been initialised in a waiting state.
 *
 * \param shm The address of a shared memory control structure which will be
 *            populated by this function.
 * \param share_name The unique string used to identify the shared memory object.
 * \returns Returns true if and only if shm->fd >= 0 and shm->data != MAP_FAILED.
 *          Even if false is returned, shm->fd should contain the value returned
 *          by shm_open, and shm->data should contain the value returned by mmap.
 */
bool create_shared_object_RW( shared_memory_t* shm, const char* share_name ) {
    // Remove any previous instance of the shared memory object, if it exists.
    shm_unlink(share_name);

    // Assign share name to shm->name.
    shm->name = share_name;

    // Create the shared memory object, allowing read-write access
    if ((shm->fd = shm_open(share_name, O_CREAT | O_RDWR, 0666)) < 0){
        shm->data = NULL;
        return false;
    }

    // Set the capacity of the shared memory object via ftruncate.
    if (ftruncate(shm->fd,sizeof(shared_data_t)) == -1){
        shm->data = NULL;
        return false;
    }

    // Map memory segment
    if ((shm->data = mmap(0, sizeof(shared_data_t), PROT_WRITE, MAP_SHARED, shm->fd, 0)) == (void *)-1)
    {
        return false;
    }

    // If we reach this point we should return true.
    return true;
}

/**
 * Controller: initialise a shared_object_t, creating a block of shared memory
 * with the designated name, and setting its storage capacity to the size of a
 * shared data block.
 *
 * PRE: n/a
 *
 * POST: shm_unlink has been invoked to delete any previous instance of the
 *       shared memory object, if it exists.
 * AND   The share name has been saved in shm->name.
 * AND   shm_open has been invoked to obtain a file descriptor connected to a
 *       newly created shared memory object with size equal to the size of a
 *       shared_data_t struct, with support for read and write operations. The
 *       file descriptor should be saved in shm->fd, regardless of the final outcome.
 * AND   ftruncate has been invoked to set the size of the shared memory object
 *       equal to the size of a shared_data_t struct.
 * AND   mmap has been invoked to obtain a pointer to the shared memory, and the
 *       result has been stored in shm->data, regardless of the final outcome.
 * AND   (this code is provided for you, don't interfere with it) The shared
 *       semaphore has been initialised to manage access to the shared buffer.
 * AND   Semaphores have been initialised in a waiting state.
 *
 * \param shm The address of a shared memory control structure which will be
 *            populated by this function.
 * \param share_name The unique string used to identify the shared memory object.
 * \returns Returns true if and only if shm->fd >= 0 and shm->data != MAP_FAILED.
 *          Even if false is returned, shm->fd should contain the value returned
 *          by shm_open, and shm->data should contain the value returned by mmap.
 */
bool create_shared_object_R( shared_memory_t* shm, const char* share_name ) {
    // Remove any previous instance of the shared memory object, if it exists.
    shm_unlink(share_name);

    // Assign share name to shm->name.
    shm->name = share_name;

    // Create the shared memory object, allowing read-write access
    if ((shm->fd = shm_open(share_name, O_RDONLY, 0666)) < 0){
        shm->data = NULL;
        return false;
    }

    // Set the capacity of the shared memory object via ftruncate.
    if (ftruncate(shm->fd,sizeof(shared_data_t)) == -1){
        shm->data = NULL;
        return false;
    }

    // Map memory segment
    if ((shm->data = mmap(0, sizeof(shared_data_t), PROT_WRITE, MAP_SHARED, shm->fd, 0)) == (void *)-1)
    {
        return false;
    }

    // If we reach this point we should return true.
    return true;
}


/**
 * Controller: destroys the shared memory object managed by a shared memory
 * control block.
 *
 * PRE: create_shared_object( shm, shm->name ) has previously been
 *      successfully invoked.
 *
 * POST: munmap has been invoked to remove the mapped memory from the address
 *       space
 * AND   shm_unlink has been invoked to remove the object
 * AND   shm->fd == -1
 * AND   shm->data == NULL.
 *
 * \param shm The address of a shared memory control block.
 */
void destroy_shared_object( shared_memory_t* shm ) {
    // Remove the shared memory object.
    munmap(shm, 48);
    shm->data = NULL;
    shm->fd = -1;
    shm_unlink(shm->name);
}

/**
 * Worker: Get a file descriptor which may be used to interact with a shared memory
 * object, and map the shared object to get its address.
 *
 * PRE: The Controller has previously invoked create_shared_fd to instantiate the
 *      shared memory object.
 *
 * POST: shm_open has been invoked to obtain a file descriptor connected to a
 *       shared_data_t struct, with support for read and write operations. The
 *       file descriptor should be saved in shm->fd, regardless of the final outcome.
 * AND   mmap has been invoked to obtain a pointer to the shared memory, and the
 *       result has been stored in shm->data, regardless of the final outcome.
 *
 * \param share_name The unique identification string of the shared memory object.
 * \return Returns true if and only if shm->fd >= 0 and
 *         shm->data != NULL.
 */
bool get_shared_object( shared_memory_t* shm, const char* share_name ) {
    // Get a file descriptor connected to shared memory object and save in 
    // shm->fd. If the operation fails, ensure that shm->data is 
    // NULL and return false.
    if ((shm->fd = shm_open(share_name, O_RDWR, O_RDWR)) < 0){
        shm->data = NULL;
        return false;
    }

    // Otherwise, attempt to map the shared memory via mmap, and save the address
    // in shm->data. If mapping fails, return false.
    if ((shm->data = mmap(0, sizeof(shared_data_t), 3, MAP_SHARED, shm->fd, 0)) == (void *)-1){
        return false;
    }
    // Modify the remaining stub only if necessary.
    return true;
}