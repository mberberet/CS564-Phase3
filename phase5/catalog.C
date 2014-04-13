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


const Status AttrCatalog::getInfo(const string & relation,
				  const string & attrName,
				  AttrDesc &record)
{
    Status status;
    RID rid;
    Record rec;
    HeapFileScan*  hfs;

    if (relation.empty() || attrName.empty()) return BADCATPARM;
//start frank///////////////////////////////////////////////
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

        AttrDesc * attrDesc = (AttrDesc *) rec.data;

        if (strcmp(attrDesc->attrName, attrName.c_str()) == 0) {
            memcpy(&record, rec.data, rec.length);
            delete hfs;
            return status;
        }
    }
    delete hfs;
    return status;
//end frank//////////////////////////////////////////////
}


const Status AttrCatalog::addInfo(AttrDesc & record)
{
    RID rid;
    InsertFileScan*  ifs;
    Status status;
//start frank////////////////////////////////////////////////
    Record rec;
    ifs = new InsertFileScan(ATTRCATNAME, status);
    if (status != OK) {
        delete ifs;
        return status;
    }
/*    int len = strlen(record.relName);
    memset(&record.attrName[len], 0, sizeof record.relName - len);
    len = strlen(record.attrName);
    memset(&record.attrName[len], 0, sizeof record.attrName - len);*/
    //memcpy(rec.data, &record, sizeof(record));
    rec.data = &record;
    rec.length = sizeof(AttrDesc);

    status = ifs->insertRecord(rec, rid);
    if (status != OK) {
        delete ifs;
        return status;
    }

    delete ifs;
    return status;
//end frank////////////////////////////////////////////////
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
//start frank///////////////////////////////////////////////
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
//end frank/////////////////////////////////////////////////
}


const Status AttrCatalog::getRelInfo(const string & relation,
				     int &attrCnt,
				     AttrDesc *&attrs)
{
    Status status;
    RID rid;
    Record rec;
    HeapFileScan*  hfs;

    if (relation.empty()) return BADCATPARM;
//start frank////////////////////////////////////////////
    RelDesc  rel;
    status = relCat->getInfo(relation, rel);
    if (status != OK) {
        return status;
    }

    attrCnt = rel.attrCnt;

/*    hfs = new HeapFileScan(ATTRCATNAME, status);
    if (status != OK) {
        delete hfs;
        return status;
    }

    status = hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
     if (status != OK) {
        delete hfs;
        return status;
    }

    status = hfs->markScan();
    if (status != OK) {
        delete hfs;
        return status;
    }

    while(1) {
        status = hfs->scanNext(rid);
        if (status == FILEEOF || status != OK) {
           break;
        }

        attrCnt ++;
    }
*/
    attrs = new AttrDesc[attrCnt];

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

    for (int i = 0; i < attrCnt; i++) {
        status = hfs->scanNext(rid);
        if (status == FILEEOF) {
            delete hfs;
            return OK;
        } else if (status != OK) {
            break;
        }
        status = hfs->getRecord(rec);
        if (status != OK) {
            break;
        }

        attrs[i] = *((AttrDesc *) rec.data);
    }

    delete hfs;
    return status;
//end frank//////////////////////////////////////////////
}


AttrCatalog::~AttrCatalog()
{
// nothing should be needed here
}

