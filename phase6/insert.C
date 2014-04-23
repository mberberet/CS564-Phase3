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

	// Get the information and relation information about the given relation
    status = relCat->getInfo(relation, relinfo);
    if (status != OK) {
		return status;
    }

    status = attrCat->getRelInfo(relation, attrLen, attrinfo);
    if (status != OK) {
		return status;
    }
    /*check the attribute count
    if (attrLen != attrCnt) {
    	return -1;
    }*/
	// Find matching attrs, to get the lenth of the new record
    for (int i = 0; i < attrLen; i++) {
		for (int j = 0; j < attrCnt; j++) {
			if (strcmp(attrinfo[i].attrName, attrList[j].attrName) == 0) {
				len = len + attrinfo[i].attrLen;
			}
		}
    }

	// Create array directly in record.
    rec.data = (char*) malloc(len);
    rec.length = len;

	// Find each attrValue, get the value at a string, and put it into data.
    for (int i = 0; i < attrCnt; i++) {
		for (int j = 0; j < attrLen; j++) {
			if (strcmp(attrList[i].attrName, attrinfo[j].attrName) == 0) {
				int type = attrinfo[j].attrType;
                char *filter;
				if (type == INTEGER) {
					int value = atoi((char*)attrList[i].attrValue);
                    filter = (char *)&value;
				} else if (type == FLOAT) {
					float value = atof((char*)attrList[i].attrValue);
                    filter = (char *)&value;
				} else {
					filter = (char *)attrList[i].attrValue;
				}
				memcpy((char*)rec.data + attrinfo[j].attrOffset, filter, attrinfo[j].attrLen);
			}
		}
    }

    InsertFileScan ifs(relation, status);
    if (status != OK) {
        return status;
    }
    status = ifs.insertRecord(rec, rid);

    return status;

}

