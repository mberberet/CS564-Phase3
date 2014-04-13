#include "catalog.h"
#include "heapfile.h"

RelCatalog::RelCatalog(Status &status) :
	 HeapFile(RELCATNAME, status)
{
// nothing should be needed here
}


const Status RelCatalog::getInfo(const string & relation, RelDesc &record)
{
    if (relation.empty())
        return BADCATPARM;

    Status status;
    Record rec;
    RID rid;
    HeapFileScan *scan1;

    scan1 = new  HeapFileScan(RELCATNAME, status);
    if (status != OK){
        return status;
    }

    status = scan1->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
    if (status != OK) {
        delete scan1;
        return status;
    }

    status = scan1->scanNext(rid);
    if (status != OK){
        if (status == FILEEOF) {
            status = RELNOTFOUND;
        }
        return status;
    }

    status = getRecord(rid, rec);
    if (status != OK){
        return status;
    }
    memcpy(&record, rec.data, rec.length);
    delete scan1;
    return OK;
}


const Status RelCatalog::addInfo(RelDesc & record)
{
    RID rid;
    Record rec;
    InsertFileScan*  ifs;
    Status status;

    ifs = new InsertFileScan(RELCATNAME, status);
    if (status != OK){
        return status;
    }

    rec.data = &record;
    rec.length = sizeof(RelDesc);

    status = ifs->insertRecord(rec, rid);
    if (status != OK){
        return status;
    }
    delete ifs;
    return OK;

}

const Status RelCatalog::removeInfo(const string & relation)
{
    Status status;
    RID rid;
    HeapFileScan*  hfs;

    if (relation.empty()) return BADCATPARM;

    hfs = new  HeapFileScan(RELCATNAME, status);
    if (status != OK){
        return status;
    }

    status = hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
    if (status != OK) {
        return status;
    }
    status = hfs->scanNext(rid);
    if (status == FILEEOF){
        return RELNOTFOUND;
    }

    status = hfs->deleteRecord();
    if (status != OK){
        return status;
    }
    hfs->endScan();
    delete hfs;
    return OK;
}


RelCatalog::~RelCatalog()
{
// nothing should be needed here
}


AttrCatalog::AttrCatalog(Status &status) :
	 HeapFile(ATTRCATNAME, status)
{
// nothing should be needed here
}

/**
 * Return the attrbute description for attrName in relation to relName.
 *
 * @param const string & relation
 * @param const string & attrName
 * @param AttrDesc & record
 *
 * @return status - returns
 *      Ok if sucessful
 *      all possible error code from all method that
 *      returns a status
 */

const Status AttrCatalog::getInfo(const string & relation,
				  const string & attrName,
				  AttrDesc &record)
{
    Status status;
    RID rid;
    Record rec;
    HeapFileScan*  hfs;

    if (relation.empty() || attrName.empty()) return BADCATPARM;
    //create a new heapfilescan
    hfs = new HeapFileScan(ATTRCATNAME, status);
    if (status != OK) {
        delete hfs;
        return status;
    }
    //start the scan from the beginning of the file
    status = hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
    if (status != OK) {
        delete hfs;
        return status;
    }

    while (1) {
    //scan all records and get record info one by one
        status = hfs->scanNext(rid);
        if (status == FILEEOF) {
            status = ATTRNOTFOUND;//return NO Match if reach EOF
            break;
        } else if (status != OK) {
            break;
        }

        status = hfs->getRecord(rec);
        if (status != OK) {
            break;
        }

        AttrDesc * attrDesc = (AttrDesc *) rec.data;
    //compare the record with the attrName, copy the record to
    //return location if find match
        if (strcmp(attrDesc->attrName, attrName.c_str()) == 0) {
            memcpy(&record, rec.data, rec.length);
            delete hfs;
            return status;
        }
    }
    //delete the heapfilescan after done
    delete hfs;
    return status;
}



const Status AttrCatalog::addInfo(AttrDesc & record)
{
    RID rid;
    InsertFileScan*  ifs;
    Status status;

    Record rec;
    ifs = new InsertFileScan(ATTRCATNAME, status);
    if (status != OK) {
        delete ifs;
        return status;
    }

    rec.data = &record;
    rec.length = sizeof(AttrDesc);

    status = ifs->insertRecord(rec, rid);
    if (status != OK) {
        delete ifs;
        return status;
    }

    delete ifs;
    return status;
}


const Status AttrCatalog::removeInfo(const string & relation,
			       const string & attrName)
{
  Status status;
  Record rec;
  RID rid;
  AttrDesc record;
  HeapFileScan*  hfs;

  if (relation.empty() || attrName.empty()) return BADCATPARM;

    hfs = new HeapFileScan(ATTRCATNAME, status);
    if (status != OK) {
        delete hfs;
        return status;
    }

    status = hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
    if (status != OK) {
        delete hfs;
        return status;
    }

    while (1) {
        status = hfs->scanNext(rid);
        if (status == FILEEOF) {
            status = ATTRNOTFOUND;
            break;
        } else if (status != OK) {
            break;
        }

        status = hfs->getRecord(rec);
        if (status != OK) {
            break;
        }

        record = *((AttrDesc*) rec.data);

        if (strcmp(record.attrName, attrName.c_str()) == 0) {
            status = hfs->deleteRecord();
            delete hfs;
            return status;
        }
    }
    delete hfs;
    return status;
}


/**
 * Return all descriptors for all attrbutes of the relation
 * as an array of AttrDesc
 *
 * @param const string & relation
 * @param int & attrCnt
 * @param AttrDesc *& attrs
 *
 * @return status - returns
 *      Ok if sucessful
 *      all possible error code from all method that
 *      returns a status
 */


const Status AttrCatalog::getRelInfo(const string & relation,
				     int &attrCnt,
				     AttrDesc *&attrs)
{
    Status status;
    RID rid;
    Record rec;
    HeapFileScan*  hfs;

    if (relation.empty()) return BADCATPARM;
    //get information from the relCat for the attrCnt
    RelDesc  rel;
    status = relCat->getInfo(relation, rel);
    if (status != OK) {
        return status;
    }

    attrCnt = rel.attrCnt;
    //setting up the array to be returned
    attrs = new AttrDesc[attrCnt];
    //create a new heapfilescan
    hfs = new HeapFileScan(ATTRCATNAME, status);
    if (status != OK) {
        delete hfs;
        return status;
    }
    //start the scan from the beginning of the file
    status = hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
    if (status != OK) {
        delete hfs;
        return status;
    }
    //scan through all records and put the descriptors in the array
    for (int i = 0; i < attrCnt; i++) {
        status = hfs->scanNext(rid);
        if (status == FILEEOF) {//reach EOF, all file scaned
            delete hfs;
            return OK;
        } else if (status != OK) {
            break;
        }
        status = hfs->getRecord(rec);
        if (status != OK) {
            break;
        }
    //cast the record and put the descriptors into the array
        attrs[i] = *((AttrDesc *) rec.data);
    }
    //delete hfs after done
    delete hfs;
    return status;
}


AttrCatalog::~AttrCatalog()
{
// nothing should be needed here
}

