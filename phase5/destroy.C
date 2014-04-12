#include "catalog.h"

//
// Destroys a relation. It performs the following steps:
//
// 	removes the catalog entry for the relation
// 	destroys the heap file containing the tuples in the relation
//
// Returns:
// 	OK on success
// 	error code otherwise
//

const Status RelCatalog::destroyRel(const string & relation)
{
  Status status;

    if (relation.empty() || relation == string(RELCATNAME) || 
        relation == string(ATTRCATNAME)) {
        return BADCATPARM;
    }

    if ((status = attrCat->dropRelation(relation)) != OK) {
        return status;
    }

    if ((status = relCat->destroyRel(relation)) != OK) {
        return status;
    }

    if ((status = destroyHeapFile(relation)) != OK) {
        return status;
    }

    return OK;
}


//
// Drops a relation. It performs the following steps:
//
// 	removes the catalog entries for the relation
//
// Returns:
// 	OK on success
// 	error code otherwise
//

const Status AttrCatalog::dropRelation(const string & relation)
{
    Status status;
    AttrDesc *attrs;
    int attrCnt, i;
    HeapFileScan *scan;
    RID rid;
    Record rec;

    if (relation.empty()) return BADCATPARM;

    scan = new HeapFileScan(relation, status)
    if (status != OK) {
        return
    }

    status = scan->startScan(0, MAXNAME, STRING, relation, EQ);
    if (status != OK) {
        return status;
    }

    while (status != FILEEOF) {
        status = scan->scanNext(rid);
        if (status != OK) {
            return status;
        }

        status = scan->deleteRecord();
        if (status != OK) {
            return status;
        }
    }

    return OK;
}


