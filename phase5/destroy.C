/*
 * File:		    destroy.C
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
 * Purpose: The purpose of destroy.C is to provide the functionality for
 *  dropping relations from the database catalog
 * */

#include "catalog.h"
#include <stdlib.h>

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

    if ((status = relCat->removeInfo(relation)) != OK) {
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

    if (relation.empty()) return BADCATPARM;

    // Get the relevant attribute info from attrCat
    status = attrCat->getRelInfo(relation, attrCnt, attrs);
    if (status != OK) {
        return status;
    }
    // Remove the attributes from attrCat
    for (i = 0; i < attrCnt; i++) {
        string str(attrs[i].attrName);
        status = attrCat->removeInfo(relation, str);
        if (status != OK) {
            return status;
        }
    }

    free(attrs);

    return OK;
}


