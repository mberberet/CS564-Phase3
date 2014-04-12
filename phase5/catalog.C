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
    status = scan1->startScan(0, MAXNAME, STRING, relation, EQ); 
    if (status != OK) {
        return status;
    }
    status = scan1->scanNext(rid);
    while (status != OK){
        if (status == FILEEOF){ 
            return status;
        }
        status = scan1->scanNext(rid);
    }
    status = getRecord(rid, rec);
    if (status != OK){
        return status;
    }
    memcpy(&record, &rec, rec.length);
}


const Status RelCatalog::addInfo(RelDesc & record)
{
  RID rid;
  InsertFileScan*  ifs;
  Status status;




}

const Status RelCatalog::removeInfo(const string & relation)
{
  Status status;
  RID rid;
  HeapFileScan*  hfs;

  if (relation.empty()) return BADCATPARM;



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
    
    status = hfs->startScan(0, sizeof(relation), String, filter, operator);
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

        AttrDesc * attrDesc = rec.data;
        
        if (attrDesc->attrName == AttrName.c_str()) {
            memcpy(&record, rec.data,rec.length);
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

