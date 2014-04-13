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
   
    scan1 = new  HeapFileScan(relation, status);
    if (status != OK){
    return status;
    }
    status = scan1->startScan(0, MAXNAME, STRING, relation.c_str(), EQ); 
    if (status != OK) {
        return status;
    }
    status = scan1->scanNext(rid);
    if (status != OK){
        return status;
    }
    status = getRecord(rid, rec);
    if (status != OK){
        return status;
    }
    memcpy(&record, &rec, rec.length);
    delete scan1;
    return OK;
}


const Status RelCatalog::addInfo(RelDesc & record)
{
    RID rid;
    Record rec;
    InsertFileScan*  ifs;
    Status status;
    string str(record.relName);
    ifs = new InsertFileScan(str, status);
    if (status != OK){
        return status;
    }
    rec.data = &record;
    rec.length = sizeof(record);
    status = ifs -> insertRecord(rec, rid);
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
    hfs = new  HeapFileScan(relation, status);
    if (status != OK){
    return status;
    }
    status = hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ); 
    if (status != OK) {
        return status;
    }
    status = hfs->scanNext(rid);
    if (status != OK){
        return status;
    }
    status = hfs-> deleteRecord();
    if (status != OK){
        return status;
    }
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
    hfs = new HeapFileScan(ATTRCATNAME, status);
    if (status != OK) {
        return status;
    }
    
    status = hfs->startScan(0, MAXNAME, STRING, relation.c_str(), EQ);
    if (status != OK) {
        return status;
    }

    while (true) {
        status = hfs->scanNext(rid);
        if (status != OK) {
            break;
        }
        
        status = hfs->getRecord(rec);
        if (status != OK) {
            break;
        }

        AttrDesc * attrDesc = (AttrDesc *) rec.data;
        
        if (strcmp(attrDesc->attrName, attrName.c_str()) == 0) {
            memcpy(&record, rec.data, rec.length);
            return status;
        }
    }
    
    return status;

}


const Status AttrCatalog::addInfo(AttrDesc & record)
{
  RID rid;
  InsertFileScan*  ifs;
  Status status;





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




}


AttrCatalog::~AttrCatalog()
{
// nothing should be needed here
}

