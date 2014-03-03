/*
 * File:		    buf.C	
 * Semester:		CS564 Spring 2014
 *
 * Author:		Michael Berberet
 * CS login:		berberet
 *
 * Partner:	    Casey Lanham	
 * CS login:        lanham
 *
 * Purpose: The purpose of the BufMgr is to handle creating pages,
 *      reading and writing pages to and from disk, and handle which pages
 *      should be written based upon the clock algorithm that will free pages
 *      that aren't being used or weren't referenced recently.
*/


#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include "page.h"
#include "buf.h"

#define ASSERT(c)  { if (!(c)) { \
		       cerr << "At line " << __LINE__ << ":" << endl << "  "; \
                       cerr << "This condition should hold: " #c << endl; \
                       exit(1); \
		     } \
                   }

//----------------------------------------
// Constructor of the class BufMgr
//----------------------------------------

BufMgr::BufMgr(const int bufs)
{
    numBufs = bufs;

    bufTable = new BufDesc[bufs];
    memset(bufTable, 0, bufs * sizeof(BufDesc));
    for (int i = 0; i < bufs; i++)         printf("%d\n", clockHand);

    {
        bufTable[i].frameNo = i;
        bufTable[i].valid = false;
    }

    bufPool = new Page[bufs];
    memset(bufPool, 0, bufs * sizeof(Page));

    int htsize = ((((int) (bufs * 1.2))*2)/2)+1;
    hashTable = new BufHashTbl (htsize);  // allocate the buffer hash table

    clockHand = bufs - 1;
}


BufMgr::~BufMgr() {

    // flush out all unwritten pages
    for (int i = 0; i < numBufs; i++) 
    {
        BufDesc* tmpbuf = &bufTable[i];
        if (tmpbuf->valid == true && tmpbuf->dirty == true) {

#ifdef DEBUGBUF
            cout << "flushing page " << tmpbuf->pageNo
                 << " from frame " << i << endl;
#endif

            tmpbuf->file->writePage(tmpbuf->pageNo, &(bufPool[i]));
        }
    }

    delete [] bufTable;
    delete [] bufPool;
}


/* *
 * Allocates a free frame through the implementation of the clock
 * algorithm. It returns the newly freed frame through the parameter "frame".
 *
 * @param frame - holds the allocated frame number
 *
 * @return status - returns...
 *      BUFFEREXCEEDED if there are no buffer frames free to allocate.
 *      UNIXERR if there is an error when writing the dirty page to disk.
 *      OK if successful.
 *
 * */
const Status BufMgr::allocBuf(int & frame) 
{
    Status status = OK;
    BufDesc* tmpbuf;
    // Need to go through the frames at most twice each.
    for (int i = 0; i < numBufs * 2; i++) {
        advanceClock();
        tmpbuf = &bufTable[clockHand];  // Get the frame buffer description
        if (tmpbuf->valid == false) {
            frame = clockHand;
            return OK;
        }
        // frame has been referenced, set to false
        if (tmpbuf->refbit == true) {
            tmpbuf->refbit = false;
        } else if (tmpbuf->pinCnt == 0) {
            // Check if page needs to be written to disk.
            if (tmpbuf->dirty == true) {

                #ifdef DEBUGBUF
            	cout << "Writing page " << tmpbuf->pageNo
                << " from frame " << clockHand << endl;
                #endif

                // Attempt to write page to disk
                status = tmpbuf->file->writePage(tmpbuf->pageNo,
                    &(bufPool[clockHand]));
                // Check more errors?
                if (status == UNIXERR) {
        	        return UNIXERR;
                }
        	    tmpbuf->dirty = false;  // Wrote successfully, no longer dirty.
            }
            hashTable->remove(tmpbuf->file,tmpbuf->pageNo);
            frame = clockHand;
            return OK; 
        }
    }
    return BUFFEREXCEEDED;  // Didn't find an unpinned page.
}


/* *
 * Reads a page from the buffer pool. If the page does not exist, a new page is
 * created and returned.
 *
 * @param file - the pointer to the file object.
 * @param PageNo - the page number to look up.
 * @param page - the fetched page is returned through this parameter
 *
 * @return status - returns...
 *      BUFFEREXCEEDED if there are no buffer frames free to allocate.
 *      HASHTBLERROR if a hash table error occurs.
 *      UNIXERR if there is an error when writing the dirty page to disk.
 *      OK if successful.
 *
 * */
const Status BufMgr::readPage(File* file, const int PageNo, Page*& page)
{
    Status status = OK;
    int frameNo = -1;
    status = hashTable->lookup(file, PageNo, frameNo);

    if (status == HASHNOTFOUND) {
        // Page not in buffer pool
        status = allocBuf(frameNo);
        if (status == OK) {
            // fetch page from disk.
            status = file->readPage(PageNo, &(bufPool[frameNo]));
            // ??? on errors
            if (status == BADPAGENO) {     
                 


                return BADPAGENO;
            } else if (status == BADPAGEPTR) {
                return BADPAGEPTR;    
            } else if (status == UNIXERR) {
                return UNIXERR;
            }
            status = hashTable->insert(file, PageNo, frameNo);
            if (status == HASHTBLERROR) {
                return HASHTBLERROR;
            }
            bufTable[frameNo].Set(file, PageNo);

        } else if (status == BUFFEREXCEEDED) {
            return BUFFEREXCEEDED;
        } else if (status == UNIXERR) {
            return UNIXERR;
        }
    } else if (status == OK) {
        // Page exists, set to referenced and increase pin count.
        bufTable[frameNo].refbit = true;
        bufTable[frameNo].pinCnt++;
    }
    page = &(bufPool[frameNo]);
    return OK;
}

/*
 * Decrements the pinCnt of the frame containing (file, PageNo) 
 * and, if dirty == true, sets the dirty bit
 *
 * @param file - the pointer to the file object 
 * @param pageNo - the number of the page withing the file
 * #param dirty - says if the file needs to be flushed to the 
 *                      disk true if dirty false if not
 *
 * @return status - returns..
 *                  OK if unpinned correctly 
 *                  HASHNOTFOUND if the page is not in the buffer pool hash table
 *                  PAGENOTPINNED if the pin count is already 0
 *
 *
 * */
const Status BufMgr::unPinPage(File* file, const int PageNo, 
			       const bool dirty) 
{
    Status status = OK;
    int frameNo;
    status = hashTable->lookup(file, PageNo, frameNo);
   
    if (status == HASHNOTFOUND) 
    {
        // Page not in buffer pool
        return HASHNOTFOUND;
    }
    else if (status == OK) 
    {
        // Page in buffer pool
        if (bufTable[frameNo].pinCnt == 0)
        {
            return PAGENOTPINNED;
        } 
        else
        {
            bufTable[frameNo].pinCnt--; 
            if (dirty == true)
            {
                bufTable[frameNo].dirty = true;
            }        
        }
    } 
    return OK;
}
/*
 * allocates an empty page in the specified file
 *
 *
 *
 * @param file   - the pointer to the file object 
 * @param pageNo - the number of the page withing the file is returned through this parameter
 * @param page   - the allocated page is returned through this parameter 
 *
 *
 * @return status - returns..
 *                  OK if unpinned correctly 
 *                  HASHNOTFOUND if the page is not in the buffer pool hash table
 *                  PAGENOTPINNED if the pin count is already 0
 *
 *
 * */

const Status BufMgr::allocPage(File* file, int& pageNo, Page*& page) 
{
    Status status;
    int frameNo; // holds the frame number of the page in the bufpool
    status = file->allocatePage(pageNo); 
    if (status == UNIXERR)
    {
        return UNIXERR;
    }
    status = allocBuf(frameNo); 

    if (status == UNIXERR)
    {
        return UNIXERR;
    }
    else if ( status == BUFFEREXCEEDED )
    {
        return BUFFEREXCEEDED; 
    }
    status = hashTable->insert(file, pageNo, frameNo);

    if (status == HASHTBLERROR) 
    {
        return HASHTBLERROR;
    }
    bufTable[frameNo].Set(file, pageNo);

    page = &(bufPool[frameNo]);
    return OK;
}

const Status BufMgr::disposePage(File* file, const int pageNo) 
{
    // see if it is in the buffer pool
    Status status = OK;
    int frameNo = 0;
    status = hashTable->lookup(file, pageNo, frameNo);
    if (status == OK)
    {
        // clear the page
        bufTable[frameNo].Clear();
    }
    status = hashTable->remove(file, pageNo);

    // deallocate it in the file
    return file->disposePage(pageNo);
}

const Status BufMgr::flushFile(const File* file) 
{
    Status status;

    for (int i = 0; i < numBufs; i++) {
        BufDesc* tmpbuf = &(bufTable[i]);
        if (tmpbuf->valid == true && tmpbuf->file == file) {

            if (tmpbuf->pinCnt > 0)
    	        return PAGEPINNED;

            if (tmpbuf->dirty == true) {
                #ifdef DEBUGBUF
        	    cout << "flushing page " << tmpbuf->pageNo
                << " from frame " << i << endl;
                #endif
                if ((status = tmpbuf->file->writePage(tmpbuf->pageNo,
					      &(bufPool[i]))) != OK)
    	            return status;

                tmpbuf->dirty = false;
            }

            hashTable->remove(file,tmpbuf->pageNo);

            tmpbuf->file = NULL;
            tmpbuf->pageNo = -1;
            tmpbuf->valid = false;
        }
        else if (tmpbuf->valid == false && tmpbuf->file == file)
            return BADBUFFER;
    }
  
    return OK;
}


void BufMgr::printSelf(void) 
{
    BufDesc* tmpbuf;
  
    cout << endl << "Print buffer...\n";
    for (int i=0; i<numBufs; i++) {
        tmpbuf = &(bufTable[i]);
        cout << i << "\t" << (char*)(&bufPool[i]) 
             << "\tpinCnt: " << tmpbuf->pinCnt;
    
        if (tmpbuf->valid == true)
            cout << "\tvalid\n";
        cout << endl;
    };
}


