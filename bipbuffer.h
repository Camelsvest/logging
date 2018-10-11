#ifndef _BIPBUFFER_H_
#define _BIPBUFFER_H_

// principle:
// http://www.codeproject.com/Articles/3479/The-Bip-Buffer-The-Circular-Buffer-with-a-Twist

typedef struct
{
    unsigned char* buffer;
    int            ixa;		// index position of block A in buffer
    int            sza;		// block A size
    int            ixb;		// index position of block B in buffer
    int            szb;		// block B size
    int            buflen;	// buffer length

    // used by reserve and commit
    int            ixResrv;	// index position for reserved buffer, which size is szResrv
    int            szResrv;	// how much space has been reserved in the buffer.
} bipbuf_t;

/**
 * Create a new bip buffer.
 *
 * malloc()s space
 *
 * @param[in] size The size of the buffer */
bipbuf_t *bipbuf_new(const unsigned int size);


/**
 * Free the bip buffer */
void bipbuf_free(bipbuf_t *me);


/**
 * Reserves space in the buffer for a memory write operation
 * Parameters:
 *   int size             amount of space to reserve
 *   int& reserved        size of space actually reserved
 *
 * Returns:
 * unsigned char*         pointer to the reserved block
 */
unsigned char* bipbuf_reserve(bipbuf_t *me, int size, int* reserved);

/*
 * bipbuf_commit
 *
 * Commits space that has been written to in the buffer
 *
 * Parameters:
 *   int size                number of bytes to commit
 *
 * Notes:
 *   Committing a size > than the reserved size will cause an assert in a debug build;
 *   in a release build, the actual reserved size will be used.
 *   Committing a size < than the reserved size will commit that amount of data, and release
 *   the rest of the space.
 *   Committing a size of 0 will release the reservation.
 */
void bipbuf_commit(bipbuf_t *me, int size);


/* bipbuf_get_contiguous_block
 *
 * Gets a pointer to the first contiguous block in the buffer, and returns the size of that block.
 *
 * Parameters:
 *   OUT int & size            returns the size of the first contiguous block
 *
 * Returns:
 *   unsigned char*                    pointer to the first contiguous block, or NULL if empty.
 */
unsigned char* bipbuf_get_contiguous_block(bipbuf_t *me, int* size)


/*
 * bipbuf_decommit_block
 *
 * Decommits space from the first contiguous block
 *
 * Parameters:
 *   int size                amount of memory to decommit
 *
 * Returns:
 *   nothing
 */
void bipbuf_decommit_block(bipbuf_t *me, int size);


/*
 * bipbuf_get_committed_size
 *
 * Queries how much data (in total) has been committed in the buffer
 *
 * Parameters:
 *   none
 *
 * Returns:
 *   int                    total amount of committed data in the buffer
 */
int bipbuf_get_committed_size(bipbuf_t *me);

#endif /* _BIPBUFFER_H_ */
