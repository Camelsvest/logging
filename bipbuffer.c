#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* for memcpy */
#include <string.h>

#include "bipbuffer.h"

#define BOOL	int
#define TRUE	1
#define FALSE	0

#define IS_TRUE(a)	(a == TRUE)
#define IS_FALSE(a)	(a == FALSE)
#define IS_NULL(a)	(a == NULL)


static BOOL bipbuf_init(bipbuf_t *me, const unsigned int size)
{
	me->buffer = (unsigned char *)calloc(size, sizeof(unsigned char));
	if (me->buffer == NULL)
	{
		return FALSE;
	}

	me->ixa = 0;
	me->sza = 0;
	me->ixb = 0;
	me->szb = 0;
	me->buflen = size;
	me->ixResrv = 0;
	me->szResrv = 0;

	return TRUE;
}


/*
 *  
 * +---------------------------------------------------------------------------+
 * |                            |xxxxxxxxx|                                    |
 * +---------------------------------------------------------------------------+
 *                              ixa      sza               
 */
static int bipbuf_get_space_aftera() const
{
	return buflen - ixa - sza;
}


/*
 * Only for below situation: block B is in front of block A 
 * +---------------------------------------------------------------------------+
 * |                            |xxxxxxxxx|                 |xxxxxxx|          |
 * +---------------------------------------------------------------------------+
 *                              ixb      szb               ixa
 */
static int bipbuf_get_space_afterb() const
{
	return ixa - ixb - szb;
}


bipbuf_t *bipbuf_new(const unsigned int size)
{
	bipbuf_t *me = NULL;

	if (size > 0)
	{
		me = (bipbuf_t *)malloc(1, sizeof(bipbuf_t));
		if (me)
		{
			if (IS_FALSE(bipbuf_init(me, size)))
			{
				free(me);
				me == NULL;
			}
		}
	}

	return me;
}


void bipbuf_free(bipbuf_t *me)
{
	if (me)
	{
		if (me->buffer)
		{
			free(me->buffer);
		}
		free(me);
	}
}


unsigned char* bipbuf_reserve(bipbuf_t *me, int size, int* reserved)
{
	int freespace;

        // We always allocate on B if B exists; this means we have two blocks and our buffer is filling.
        if (me->szb)
        {
            freespace = bipbuf_get_freespace_afterb();

            if (size < freespace)
		freespace = size;

            if (freespace == 0)
		return NULL;

            me->szResrv = freespace;
            *reserved = freespace;
            me->ixResrv = me->ixb + me->szb;
            return me->buffer + me->ixResrv;
        }
        else
        {
            // Block b does not exist, so we can check if the space AFTER a is bigger than the space
            // before A, and allocate the bigger one.

            freespace = bipbuf_get_space_aftera();
            if (freespace >= me->ixa)
            {
                if (freespace == 0)
			return NULL;

                if (size < freespace)
			freespace = size;

                me->szResrv = freespace;
                *reserved = freespace;
                me->ixResrv = ixa + sza;

                return me->buffer + me->ixResrv;
            }
            else
            {
                if (me->ixa == 0)
			return NULL;

                if (me->ixa < size)
			size = me->ixa;

                me->szResrv = size;
                *reserved = size;
                me->ixResrv = 0;

		// actually me->ixb shall be alwayse 0, so we don't set
                return me->buffer;
            }
        }
	
}


void bipbuf_commit(bipbuf_t *me, int size)
{
	if (size == 0)
	{
		// decommit any reservation
		me->szResrv = me->ixResrv = 0;
		return;
	}

	// If we try to commit more space than we asked for, clip to the size we asked for.

	if (size > szResrv)
	{
		size = me->szResrv;
	}

	// If we have no blocks being used currently, we create one in A.

	if (me->sza == 0 && me->szb == 0)
	{
	    me->ixa = me->ixResrv;
	    me->sza = size;

	    me->ixResrv = 0;
	    me->szResrv = 0;
	    return;
	}

	if (me->ixResrv == me->sza + me->ixa)
	{
	    me->sza += size;
	}
	else
	{
	    me->szb += size;
	}

	me->ixResrv = 0;
	me->szResrv = 0;
}


unsigned char* bipbuf_get_contiguous_block(bipbuf_t *me, int* size)
{
	if (me->sza == 0)
	{
	    *size = 0;
	    return NULL;
	}

	*size = me->sza;
	return buffer + ixa;
}



void bipbuf_decommit_block(bipbuf_t *me, int size)
{
	if (size >= me->sza)
	{
		me->ixa = me->ixb;

		// fix issue of original CBipBuffer classs here
		size -= me->sza;

		me->sza = me->szb;

		me->ixb = 0;
		me->szb = 0;
	}

	if (me->sza >= size)
	{
		me->sza -= size;
		me->ixa += size;
	}
	else
		assert(0);	// bug
}

int bipbuf_get_committed_size(bipbuf_t *me) const
{
	return me->sza + me->szb;
}