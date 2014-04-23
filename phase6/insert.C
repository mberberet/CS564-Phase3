/*
 * File:		    insert.C
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
 * Purpose: implements insertion of a record into a relation.
 *
 */

#include "catalog.h"
#include "query.h"


/*
 * Inserts a record into the specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Insert(const string & relation,
	const int attrCnt,
	const attrInfo attrList[])
{
// part 6
	Status status = OK;
    Record rec;
    RID rid;
    int attrLen = 0;
    int len = 0;
    RelDesc relinfo;
    AttrDesc* attrinfo;
    InsertFileScan *ifs;

	// Get relevant information about the relation and attributes
    status = relCat->getInfo(relation, relinfo);
    if (status != OK) {
		return status;
    }

    status = attrCat->getRelInfo(relation, attrLen, attrinfo);
    if (status != OK) {
		return status;
    }

	// Determine the length of the new record
    for (int i = 0; i < attrLen; i++) {
		for (int j = 0; j < attrCnt; j++) {
			if (strcmp(attrinfo[i].attrName, attrList[j].attrName) == 0) {
				len = len + attrinfo[i].attrLen;
			}
		}
    }

	// Initialize record data
    rec.data = (char*) malloc(len);
    rec.length = len;

	// Create the record data in the proper order
    for (int i = 0; i < attrCnt; i++) {
		for (int j = 0; j < attrLen; j++) {
            // Determine if the attribute is the next to be added
			if (strcmp(attrList[i].attrName, attrinfo[j].attrName) == 0) {
				int type = attrinfo[j].attrType;
                char *value;
				if (type == INTEGER) {
					int val = atoi((char*)attrList[i].attrValue);
                    value = (char *)&val;
				} else if (type == FLOAT) {
					float val = atof((char*)attrList[i].attrValue);
                    value = (char *)&val;
				} else {
					value = (char *)attrList[i].attrValue;
				}
				memcpy((char*)rec.data + attrinfo[j].attrOffset, value, attrinfo[j].attrLen);
			}
		}
    }

    // Insert the new record into the relation
    ifs = new InsertFileScan(relation, status);
    if (status != OK) {
        delete ifs;
        return status;
    }

    status = ifs->insertRecord(rec, rid);
    delete ifs;
    return status;

}

