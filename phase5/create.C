/*
 * File:		    create.C
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
 * Purpose: The purpose of the create.C class is to allow a relation to
 *  be created and added to the database.
*/


#include "catalog.h"


/* *
 * Adds a relation to the database.
 *
 * @param relation - the name of the relation to be added
 * @param attrCnt  - how many attributes are in the relation
 * @param attrList - the attributes of the relation
 *
 * @return status - returns...
 *      RELEXISTS if the relation already is in the database
 *      NAMETOOLONG if the relation name is to long
 *      BADCATPARM if no relation name or no attributes are provided
 *      DUPLATTR if two attributes have the same name
 *      ATTRTOOLONG if the relation is too large
 *      OK if successful.
 * */
const Status RelCatalog::createRel(const string & relation,
				   const int attrCnt,
				   const attrInfo attrList[])
{
    Status status;
    RelDesc rd;
    AttrDesc ad;

    if (relation.empty() || attrCnt < 1)
        return BADCATPARM;

    if (relation.length() >= sizeof rd.relName)
        return NAMETOOLONG;

    // Make sure relation with the same name doesn't exist(use getInfo())
    status = relCat->getInfo(relation, rd);
    if (status == OK) {
        return RELEXISTS;
    } else if (status != RELNOTFOUND) {
        return status;
    }

    // Make sure the tuple isn't too large and check for duplicates
    unsigned int width = 0;

    for (int i = 0; i < attrCnt; i++) {
        width += attrList[i].attrLen;
        for (int j = 0; j < i; j++) {
            if (strcmp(attrList[i].attrName, attrList[j].attrName) == 0) {
                return DUPLATTR;
            }
        }
    }
    if (width > PAGESIZE) {
        return ATTRTOOLONG;
    }

    if (relation.length() > MAXNAME) {
        return NAMETOOLONG;
    }

    // Add the relation to relCat
    strcpy(rd.relName, relation.c_str());
    rd.attrCnt = attrCnt;
    status = relCat->addInfo(rd);
    if (status != OK) {
        return status;
    }

    // For the attributes, invoke AttrCatalog::addInfo() (attrCat)
    //  make instance of AttrDesc struct using info from attrList
    int offset = 0;
    strcpy(ad.relName, relation.c_str());
    for (int i = 0; i < attrCnt; i++) {
        if (strlen(attrList[i].attrName) >= sizeof ad.attrName) {
            // remove things already added to remain consistent
            attrCat->dropRelation(relation);
            relCat->removeInfo(relation);
            return NAMETOOLONG;
        }
        // Prepare the attribute
        strcpy(ad.attrName, attrList[i].attrName);
        ad.attrOffset = offset;
        ad.attrType = attrList[i].attrType;
        ad.attrLen = attrList[i].attrLen;

        // Add the attribute to attrCat
        status = attrCat->addInfo(ad);
        if (status != OK) {
            return status;
        }
        offset += ad.attrLen;
    }

    //  Create a heapfile instace to hold the tuples of the relation (look at last project)
    if ((status = createHeapFile(relation)) != OK) {
        attrCat->dropRelation(relation);
        relCat->removeInfo(relation);
        return status;
    }
    return OK;
}

