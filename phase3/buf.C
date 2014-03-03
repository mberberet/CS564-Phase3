/*
 * File:		    buf.C	
 * Semester:		CS564 Spring 2014
 *
 * Author:		Michael Berberet
 * CS login:		berberet
 *
 * Partner:	    Casey Lanham	
 * CS login:        lanham
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

/*
 * getCommands parses the command line arguments stored in argv and returns
 * a character array of the valid commands. The array when all commands are
 * used will have "sUSvc". U and c are defaulted to be set in the array.
 * The characters will always be in the same order, and the return string
 * is null terminated. Any -p commands will store a PID value in the array
 * pointed to by pidPtr. pidPtr will point to a null pointer and the
 * function will return null on an error.
 *
 * @param argc - argc pasted from main
 * @param argv - character string array of commandline arguments
 * #param pidPtr - pointer to address of an array of pid ints provided by
 * 	the -p <pid> command in the commandline
 *
 * @return cmd - returns address of the first character in the command
 * 	list
 *
 *
 * */

const Status BufMgr::allocBuf(int & frame) 
{
/*
 * Allocates a free frame using the clock algorithm; if necessary, writing a dirty page back to disk. Returns BUFFEREXCEEDED if all buffer frames are pinned, UNIXERR if the call to I/O layer
 * returned an error when a dirty page was being written to disk and OK otherwise. This private method will get called by the readPage() and allocPage() methods.
 *
 * Make sure that if the buffer frame allocated has a valid page in it, that you remove the appropriate entry from the hash table.
 */
    Status status = OK;
    BufDesc* tmpbuf;
    for (int i = 0; i < numBufs * 2; i++) {
        advanceClock();
        tmpbuf = &bufTable[clockHand];
        if (tmpbuf->valid == false) {
            frame = clockHand;
            return OK;
        }
        if (tmpbuf->refbit == true) {
            tmpbuf->refbit = false;
        } else if (tmpbuf->pinCnt == 0) {
            if (tmpbuf->dirty == true) {

                #ifdef DEBUGBUF
            	cout << "Writing page " << tmpbuf->pageNo
                << " from frame " << clockHand << endl;
                #endif

                // Attempt to write page to disk
                status = tmpbuf->file->writePage(tmpbuf->pageNo,
                    &(bufPool[clockHand]));
                // Consider more error checking (badpageno, badpageptr)
                if (status == UNIXERR) {
        	        return UNIXERR;
                }
        	    tmpbuf->dirty = false;
            }
            hashTable->remove(tmpbuf->file,tmpbuf->pageNo);
            frame = clockHand;
            return OK; 
        }
    }
    return BUFFEREXCEEDED;
}

	
const Status BufMgr::readPage(File* file, const int PageNo, Page*& page)
{
/* 
 * First check whether the page is already in the buffer pool by invoking the lookup() method on the hashtable to get a frame number. There are two cases to be handled depending on the outcome
 * of the lookup() call.
 *
 * Case 1) Page is not the buffer pool. Call allocBuf() to allocate a buffer frame and then call the method file->readPage() to read the page from disk into the buffer pool frame. Next insert the
 * page into the hashtable. Finally, invoke Set() on the frame to set it up properly. Set() will leave the pinCnt for the page set to 1. Return a pointer to the frame containing the page via the page
 * parameter.
 *
 * Case 2) Page is in the buffer pool. In this case set the appropriate refbit, increment the pinCnt for the page, and then return a pointer to the frame containing the page via the page parameter.
 *
 * Returns OK if no errors occurred, UNIXERR if a Unix error occured, BUFFEREXCEEDED if all buffer frames are pinned, HASHTBLERROR if a hash table error occurred.
 */
    Status status = OK;
    int frameNo = -1;
    status = hashTable->lookup(file, PageNo, frameNo);

    // Case #1
    if (status == HASHNOTFOUND) {
        // Page not in buffer pool
        status = allocBuf(frameNo);
        if (status == OK) {
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
        bufTable[frameNo].refbit = true;
        bufTable[frameNo].pinCnt++;
    }
    // might be wrong?
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


