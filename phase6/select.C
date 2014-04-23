#include "catalog.h"
#include "query.h"


// forward declaration
const Status ScanSelect(const string & result, 
			const int projCnt, 
			const AttrDesc projNames[],
			const AttrDesc *attrDesc, 
			const Operator op, 
			const char *filter,
			const int reclen);

/*
 * Selects records from the specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Select(const string & result, 
		       const int projCnt, 
		       const attrInfo projNames[],
		       const attrInfo *attr, 
		       const Operator op, 
		       const char *attrValue)//filter
{
   // Qu_Select sets up things and then calls ScanSelect to do the actual work
    Status status;
    AttrDesc attrDescArray[projCnt];
    AttrDesc *attrDesc;
    int attrCnt;
    int reclen = 0;
    const char* filter;
    cout << "Doing QU_Select " << endl;   
    for (int i = 0; i < projCnt; i++)
    {
        status = attrCat -> getInfo(projNames[i].relName,
                                    proNames[i].attrName,
                                    attrDescArray[i]);
        if (status != OK)
        {   
            return status;
        }
        reclen += attrDescArray[i].attrLen; 

    }
    status = attrCat -> getInfo(attr->relName, attr->attrName, attrDesc); 
    if (status != OK)
        {   
            return status;
        }

    if (type == INTEGER) {
        int val = atoi(attrValue);
        filter = (char *)&val;
    } else if (type == FLOAT) {
        double val = atof(attrValue);
        filter = (char *)&val;
    } else {
        filter = attrValue;
    }
    status = ScanSelect(result, projCnt, attrDescArray, 
                        attrDesc, op, filter, reclen);          
    return status;        
}
 


const Status ScanSelect(const string & result, 
#include "stdio.h"
#include "stdlib.h"
			const int projCnt, 
			const AttrDesc projNames[],
			const AttrDesc *attrDesc, 
			const Operator op, 
			const char *filter,
			const int reclen)
{
    cout << "Doing HeapFileScan Selection using ScanSelect()" << endl;
    HeapFileScan* hfs;
    Status status;
    Record rec;
    RID rid;
    InsertFileScan* ifs;

    ifs = new InsertFileScan(result, status);
    if (status != OK) {
        return status;
    }


    hfs = new HeapFileScan(projNames[1].relName, status);
    if (status != OK) {
        return status;
    }
    status = hfs->startScan(attrDesc->attrOffset,
                            attrDesc->attrLen, (Datatype) attrDesc->attrType,
                            filter, op);
    if (status != OK) {
        return status;
    }
    while ((status = hfs->scanNext(rid)) == OK) {
        status = hfs->getRecord(rec);
    if (status != OK) {
        return status;
    }
    

}
