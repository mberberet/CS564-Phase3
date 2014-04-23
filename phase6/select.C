/*
 * File:		    select.C
 * Semester:		CS564 Spring 2014
 *
 * Author:		Michael Berberet
 * CS login:		berberet
 *
 * Partner:	    Casey Lanham
 * CS login:        lanham
 *
 * Partner:     Xuelong Zhang
 * CS login:        xuelong
 *
 * Purpose: implement the select function of a database
 */


#include "catalog.h"
#include "query.h"
#include "stdio.h"
#include "stdlib.h"

// forward declaration
const Status ScanSelect(const string & result,
			const int projCnt,
			const AttrDesc projNames[],
			const AttrDesc *attrDesc,
			const Operator op,
			const char *filter,
			const int reclen);

/* *
 * sets up things and then calls ScanSelect 
 *
 * @param projCnt  - the number of items in projNames
 * @param projNames- an array of the names
 * @param attrDesc - the AttrDesc of the attribute we are selecting from 
 * @param op       - the operator (=,>,<,...)
 * @param filter   - the filter
 * @param reclen   - the length of the record
 * @param result   - the table to return the results in 
 *
 * @return status  - returns...
 *                  OK if successful.
 *                  error status if not
 *                 
 * */

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
    AttrDesc attrDesc;
    int reclen = 0;
    const char* filter;

    cout << "Doing QU_Select " << endl;

    for (int i = 0; i < projCnt; i++)
    {
        string relation(projNames[i].relName);
        string attrName(projNames[i].attrName);
        status = attrCat -> getInfo(relation, attrName, attrDescArray[i]);

        if (status != OK)
        {
            return status;
        }
        reclen += attrDescArray[i].attrLen;
    }

    if (attr != NULL) {
        string relName(attr->relName);
        string attName(attr->attrName);
        status = attrCat -> getInfo(relName, attName, attrDesc);
        if (status != OK)
        {
            return status;
        }
        if (attr->attrType == INTEGER) {
            int val = atoi(attrValue);
            filter = (char *)&val;
        } else if (attr->attrType == FLOAT) {
            double val = atof(attrValue);
            filter = (char *)&val;
        } else {
            filter = attrValue;
        }
    } else {
        filter = NULL;
    }
    status = ScanSelect(result, projCnt, attrDescArray,
                        &attrDesc, op, filter, reclen);
    if (status == FILEEOF) {
        status = OK;
    }
    return status;
}
/*
 * Selects records from the specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */


const Status ScanSelect(const string & result,
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
    Record insRec;


    ifs = new InsertFileScan(result, status);
    if (status != OK) {
        delete ifs;
        return status;
    }
    hfs = new HeapFileScan(projNames[0].relName, status);
    if (status != OK) {
        delete ifs;
        delete hfs;
        return status;
    }
    status = hfs->startScan(attrDesc->attrOffset, attrDesc->attrLen,
                            (Datatype) attrDesc->attrType, filter, op);
    if (status != OK) {
        delete ifs;
        delete hfs;
        return status;
    }
    insRec.data = (char *) malloc(reclen);
    insRec.length = reclen;

    while (hfs->scanNext(rid) == OK) {
        status = hfs->getRecord(rec);
        if (status != OK) {
            delete ifs;
            delete hfs;
            return status;
        }

        int offset = 0;
        for (int i = 0; i < projCnt; i++){
            memcpy((char *)insRec.data + offset,(char *)rec.data + projNames[i].attrOffset,
                    projNames[i].attrLen);
            offset = offset + projNames[i].attrLen;
        }
        status = ifs->insertRecord(insRec, rid);
        if (status != OK) {
            delete ifs;
            delete hfs;
            return status;
        }

    }

    delete ifs;
    delete hfs;
    return status;
}
