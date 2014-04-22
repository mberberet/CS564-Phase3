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
    for (int i = 0; i < projCnt; i++)
    {
        status = attrCat -> getInfo(projNames[i].relName,
                                    proNames[i].attrName,
                                    attrDescArray[i]);
        if (status != OK)
        {   
            return status;
        }
        reclen += attrDescArray[i].AttrLen; 

    }
    attrCat -> getRelInfo(
//cout << "Doing QU_Select " << endl;

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


}
