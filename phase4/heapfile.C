/*
 * File:		    heapfile.C	
 * Semester:		CS564 Spring 2014
 *
 * Author:		Michael Berberet
 * CS login:		berberet
 *
 * Partner:	        Casey Lanham	
 * CS login:            lanham
 *
 * Partner:             Xuelong Zhang
 * CS login:            xuelong
 *
 * Purpose: The purpose of this program is to implement a file manager
 * for Heap Files that also provides a scan function that allows you
 * to search the heap files with a given filter
*/


#include "heapfile.h"
#include "error.h"


/* *
 * create an heap file with the header page and the first file page.
 * unpin the two pages after intialization and marking the dirty bits dirty.
 *
 * @param const string filename - the name of the file which is going to be created
 *
 * @return status - returns...
 *      OK if sucessful
 *      and will return all possible error code from 
 *      each method that would return a status
 *
 * */

const Status createHeapFile(const string fileName)
{
    File*       file;
    Status      status;
    FileHdrPage*    hdrPage;
    int         hdrPageNo;
    int         newPageNo;
    Page*       newPage;
    
    //check the fileName legth 
    if (fileName.size() > 50) {
        return BADFILE;
    } 
    // try to open the file. This should return an error
    status = db.openFile(fileName, file);
    if (status != OK)
    {
        // file doesn't exist. First create it and allocate
        // an empty header page and data page.
        status = db.createFile(fileName); //mignt be necessary to use the arrow ->
        if (status != OK){
            return status;
        }
        //open the file just created
        status = db.openFile(fileName, file);
        if (status != OK) {
            return status;
        }
        //allocate the page for the header
        status = bufMgr->allocPage(file, hdrPageNo, newPage);
        if (status != OK){
            return status;
        }
        //assign the hdrpage ptr and file name
        hdrPage = (FileHdrPage *)newPage;
        strcpy(hdrPage->fileName, fileName.c_str());
        //allocate the page for the first file page
        status = bufMgr->allocPage(file, newPageNo, newPage);
        if (status != OK){
            return status;
        }
        //fill the fields in header
        newPage->init(newPageNo); 
        hdrPage -> firstPage = newPageNo;
        hdrPage -> lastPage = newPageNo;
        hdrPage -> pageCnt = 1;
        hdrPage -> recCnt = 0;      
        //unpin the two pages after marking them dirty 
        status = bufMgr->unPinPage(file, hdrPageNo, true);
        if (status != OK){
            return status;
        }
        
        status = bufMgr->unPinPage(file, newPageNo, true);
        if (status != OK){
            return status;
        }
        //close the file
        status = db.closeFile(file);
        if (status != OK) {
            return status;
        }
        return OK;
    }
    return (FILEEXISTS);
}

// routine to destroy a heapfile
const Status destroyHeapFile(const string fileName)
{
    return (db.destroyFile (fileName));
}


/* *
 * Constructor for a HeapFile Object. Reads the header page and first page
 * and initializes all relevant variables.
 *
 * @param fileName - the name of the file to open
 * @param returnStatus - the status to be returned (see below)
 *
 * @return status - returns...
 *      OK if successful.
 *
 * */
HeapFile::HeapFile(const string & fileName, Status& returnStatus)
{
    Status  status;
    Page*   pagePtr;

    cout << "opening file " << fileName << endl;

    // open the file and read in the header page and the first data page
    if ((status = db.openFile(fileName, filePtr)) == OK)
    {
        // Read in header page
        if ((status = filePtr->getFirstPage(headerPageNo)) != OK) {
            returnStatus = status;
            return;
        }
        if ((status = bufMgr->readPage(filePtr, headerPageNo, pagePtr)) != OK) {
            returnStatus = status;
            return;
        }
        headerPage = (FileHdrPage *) pagePtr;
        hdrDirtyFlag = false;

        // Read in first page
        status = bufMgr->readPage(filePtr, headerPage->firstPage, curPage);
        if (status != OK) {
            returnStatus = status;
            return;
        }
        curPageNo = headerPage->firstPage;
        curDirtyFlag = false;
        curRec = NULLRID;
        returnStatus = OK;
        return;
    }
    else
    {
        cerr << "open of heap file failed\n";
        returnStatus = status;
        return;
    }
}


// the destructor closes the file
HeapFile::~HeapFile()
{
    Status status;
    cout << "invoking heapfile destructor on file " << headerPage->fileName << endl;

    // see if there is a pinned data page. If so, unpin it 
    if (curPage != NULL)
    {
        status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
        curPage = NULL;
        curPageNo = 0;
        curDirtyFlag = false;
        if (status != OK) cerr << "error in unpin of date page\n";
    }
    
     // unpin the header page
    status = bufMgr->unPinPage(filePtr, headerPageNo, hdrDirtyFlag);
    if (status != OK) cerr << "error in unpin of header page\n";
    
    // status = bufMgr->flushFile(filePtr);  // make sure all pages of the file are flushed to disk
    // if (status != OK) cerr << "error in flushFile call\n";
    // before close the file
    status = db.closeFile(filePtr);
    if (status != OK)
    {
        cerr << "error in closefile call\n";
        Error e;
        e.print (status);
    }
}


const int HeapFile::getRecCnt() const
{
  return headerPage->recCnt;
}


/* *
 * returns the record for the given RID 
 * 
 *
 * @param rid - holds the slot number and page number for the 
 *    record that needs to be found
 * @param rec - holds the data and length of the record that
 *    was found 
 * @return status - returns...
 *      OK if the record was found
 *      
 * */
const Status HeapFile::getRecord(const RID & rid, Record & rec)
{
    Status status;
//cout<< "getRecord. record (" << rid.pageNo << "." << rid.slotNo << ")" << endl;
//checks to see if current page has the record that we are looking for
//returns the record in the rec field and an ok status if it is  
    if (rid.pageNo == curPageNo){  
        status = curPage-> getRecord(rid, rec);
        if (status != OK) {
            return status;
        }
        curRec = rid;
        return OK;
    }
// if the record was not in the current page, we unpin the page and read in 
// the page that is needed.(the needed page is found using the page number 
// found in the RID)
    else {
        if ((status = bufMgr -> unPinPage (filePtr, curPageNo, false)) != OK){
            return status;
        } 
        curPageNo = rid.pageNo; 
        if ((status = bufMgr -> readPage (filePtr, curPageNo, curPage)) != OK){
            return status;
        }
        
        status =  curPage -> getRecord(rid, rec); // gets the record
        if (status != OK) {
            return status;
        }
        curRec = rid;
        return OK;
    }
}


HeapFileScan::HeapFileScan(const string & name,
               Status & status) : HeapFile(name, status)
{
    filter = NULL;
}


const Status HeapFileScan::startScan(const int offset_,
                     const int length_,
                     const Datatype type_, 
                     const char* filter_,
                     const Operator op_)
{
    if (!filter_) {                        // no filtering requested
        filter = NULL;
        return OK;
    }
    
    if ((offset_ < 0 || length_ < 1) ||
        ((type_ != STRING && type_ != INTEGER && type_ != FLOAT)) ||
        (type_ == INTEGER && length_ != sizeof(int)
         ||( type_ == FLOAT && length_ != sizeof(float))) ||
        (op_ != LT && op_ != LTE && op_ != EQ && op_ != GTE && op_ != GT && op_ != NE))
    {
        return BADSCANPARM;
    }

    offset = offset_;
    length = length_;
    type = type_;
    filter = filter_;
    op = op_;

    return OK;
}


const Status HeapFileScan::endScan()
{
    Status status;
    // generally must unpin last page of the scan
    if (curPage != NULL)
    {
        status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
        curPage = NULL;
        curPageNo = 0;
        curDirtyFlag = false;
        return status;
    }
    return OK;
}


HeapFileScan::~HeapFileScan()
{
    endScan();
}


const Status HeapFileScan::markScan()
{
    // make a snapshot of the state of the scan
    markedPageNo = curPageNo;
    markedRec = curRec;
    return OK;
}


const Status HeapFileScan::resetScan()
{
    Status status;
    if (markedPageNo != curPageNo) 
    {
        if (curPage != NULL)
        {
            status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
            if (status != OK) return status;
        }
        // restore curPageNo and curRec values
        curPageNo = markedPageNo;
        curRec = markedRec;
        // then read the page
        status = bufMgr->readPage(filePtr, curPageNo, curPage);
        if (status != OK) return status;
        curDirtyFlag = false; // it will be clean
    }
    else curRec = markedRec;
    return OK;
}


/* *
 * scan each record starting from the first one of the current page until finding a match
 * or reach end of file or end of records, move to the next page when encounter end of page
 *
 * @param RID& outRid - where the matched rid of stored
 *
 * @return status - returns...
 *      OK if sucessful
 *      and will return all possible error code from 
 *      each method that would return a status
 *
 * */
const Status HeapFileScan::scanNext(RID& outRid)
{
    Status  status = OK;
    RID     nextRid;
    RID     tmpRid;
    int     nextPageNo;
    Record      rec;
   //control of the scan while loop
    bool    found = false;

    tmpRid = curRec;

    while (!found){
        //check if it is the end of the page or end of records on that page
        status = curPage->nextRecord(tmpRid, nextRid);
        
        while (status == ENDOFPAGE || status == NORECORDS){
            //check if it is the end of file
            if (curPageNo == headerPage->lastPage) {
                return FILEEOF;
            }
            //get to the next page if there is any
            status = curPage->getNextPage(nextPageNo);
            if (status != OK){
                return status;
            } 
            //unpin the previous page
            status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
            if (status != OK){
                return status;
            }
            //read the page that is just switched to
            status = bufMgr->readPage(filePtr, nextPageNo, curPage);
            if (status != OK){
                return status;
            }
            //update the curPage Ptr and the dirty bit
            curPageNo = nextPageNo;
            curDirtyFlag = false;
            //get the first record of the new page
            status = curPage->firstRecord(nextRid);
        }
        //update the tmpRid to be used in getRecord
        tmpRid = nextRid;
        //check if record matches
        status = curPage->getRecord(tmpRid, rec);
        if (status != OK){
            return status;
        } 
        //return the Rid if record matches and stop the scan loop
        if (matchRec(rec)){
            curRec = tmpRid;
            outRid = curRec;
            found = true;
        }
    }

    return OK;
   
}


// returns pointer to the current record.  page is left pinned
// and the scan logic is required to unpin the page 
const Status HeapFileScan::getRecord(Record & rec)
{
    return curPage->getRecord(curRec, rec);
}


// delete record from file. 
const Status HeapFileScan::deleteRecord()
{
    Status status;

    // delete the "current" record from the page
    status = curPage->deleteRecord(curRec);
    curDirtyFlag = true;

    // reduce count of number of records in the file
    headerPage->recCnt--;
    hdrDirtyFlag = true; 
    return status;
}


// mark current page of scan dirty
const Status HeapFileScan::markDirty()
{
    curDirtyFlag = true;
    return OK;
}


const bool HeapFileScan::matchRec(const Record & rec) const
{
    // no filtering requested
    if (!filter) return true;

    // see if offset + length is beyond end of record
    // maybe this should be an error???
    if ((offset + length -1 ) >= rec.length)
    return false;

    float diff = 0;                       // < 0 if attr < fltr
    switch(type) {

    case INTEGER:
        int iattr, ifltr;                 // word-alignment problem possible
        memcpy(&iattr,
               (char *)rec.data + offset,
               length);
        memcpy(&ifltr,
               filter,
               length);
        diff = iattr - ifltr;
        break;

    case FLOAT:
        float fattr, ffltr;               // word-alignment problem possible
        memcpy(&fattr,
               (char *)rec.data + offset,
               length);
        memcpy(&ffltr,
               filter,
               length);
        diff = fattr - ffltr;
        break;

    case STRING:
        diff = strncmp((char *)rec.data + offset,
                       filter,
                       length);
        break;
    }

    switch(op) {
    case LT:  if (diff < 0.0) return true; break;
    case LTE: if (diff <= 0.0) return true; break;
    case EQ:  if (diff == 0.0) return true; break;
    case GTE: if (diff >= 0.0) return true; break;
    case GT:  if (diff > 0.0) return true; break;
    case NE:  if (diff != 0.0) return true; break;
    }

    return false;
}


InsertFileScan::InsertFileScan(const string & name,
                               Status & status) : HeapFile(name, status)
{
  //Do nothing. Heapfile constructor will bread the header page and the first
  // data page of the file into the buffer pool
}


InsertFileScan::~InsertFileScan()
{
    Status status;
    // unpin last page of the scan
    if (curPage != NULL)
    {
        status = bufMgr->unPinPage(filePtr, curPageNo, true);
        curPage = NULL;
        curPageNo = 0;
        if (status != OK) cerr << "error in unpin of data page\n";
    }
}


/* *
 * Inserts a record into the heap file. 
 * 
 * @return status - returns...
 *      OK if successful.
 *
 * */
const Status InsertFileScan::insertRecord(const Record & rec, RID& outRid)
{
    Page *  newPage; 
    int     newPageNo;
    Status  status; 
    RID     rid;

    // check for very large records
    if ((unsigned int) rec.length > PAGESIZE-DPFIXED)
    {
        // will never fit on a page, so don't even bother looking
        return INVALIDRECLEN;
    }
   
    // Check if the last page is already in the buffer pool
    if (headerPage->lastPage != curPageNo) {
        // Page isn't in buffer pool, unpin current page
        status = bufMgr->unPinPage(filePtr, headerPage->lastPage, curDirtyFlag);
        if (status != OK){
            return status;
        }
        // Get last page
        if ((status = bufMgr->readPage(filePtr, headerPage->lastPage, curPage)) != OK) {
            return status;
        }
        curPageNo = headerPage->lastPage;
        curDirtyFlag = false;
    }
    
    // Try inserting a record
    status = curPage->insertRecord(rec, rid);

    // If there is no space, need to create a new page
    if (status == NOSPACE) {
        status = bufMgr->allocPage(filePtr, newPageNo, newPage);
        if (status != OK){
            return status;
        }
        newPage->init(newPageNo);

        // Ensure previous last page points to new last page
        curPage->setNextPage(newPageNo);
        status = bufMgr->unPinPage(filePtr, headerPage->lastPage, curDirtyFlag);
        if (status != OK){
            return status;
        }

        // Ensure page is now last page of heapfile
        curPage = newPage;
        curPageNo = newPageNo;
        headerPage->lastPage = newPageNo;
        headerPage->pageCnt++;
      
        status = newPage->insertRecord(rec, rid);
        // In theory, should always be OK
        if (status != OK) {
            return status;
        }
    }
    curDirtyFlag = true;
    headerPage->recCnt++;
    hdrDirtyFlag = true;
    outRid = rid;
    curRec = rid;
    return OK;
}
