#include "catalog.h"
#include "query.h"


/*
 * Deletes records from a specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Delete(const string & relation,
		       const string & attrName,
		       const Operator op,
		       const Datatype type,
		       const char *attrValue)
{
// part 6

    HeapFileScan* hfs;
    RID rid;
    Record rec;
    AttrDesc attr;
    Status status;
    const char* filter;
    // Get info needed for the heapfile scanner
    attrCat->getInfo(relation, attrName, attr);
    if (status != OK) {
        return status;
    }

    // Want to scan the relation passed in
    hfs = new HeapFileScan(relation, status);
    if (status != OK) {
        return status;
    }

    // Convert the string attrValue into the proper type
    if (type == INTEGER) {
        int val = atoi(attrValue);
        filter = (char *)&val;
    } else if (type == FLOAT) {
        double val = atof(attrValue);
        filter = (char *)&val;
    } else {
        filter = attrValue;
    }

    // Start the scan, looking for the filter to match the attribute
    // stored at attrOffset with a maxLen of attrLen
    hfs->startScan(attr.attrOffset, attr.attrLen, type, filter, op);
    if (status != OK) {
        return status;
    }

    // Retrieve all matching tuples
    while ((status = hfs->scanNext(rid)) == OK) {
        if (status == FILEEOF) {
            return OK;
        } else if (status != OK) {
            return status;
        }

        // Ensure the record is currently stored in curRec of heapfile scanner
        status = hfs->getRecord(rec);
        if (status != OK) {
            return status;
        }

        // Delete the record
        status = hfs->deleteRecord();
        if (status != OK) {
            return status;
        }
    }
    return OK;
}


