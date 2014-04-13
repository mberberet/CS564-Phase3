#include "catalog.h"

//First make sure that a relation with the same name doesn't already exist (by using the getInfo() function described below). Next add a tuple to the relcat relation. Do this by filling in an instance of the RelDesc structure above and then invoking the RelCatalog::addInfo() method. Third, for each of the attrCnt attributes, invoke the AttrCatalog::addInfo() method of the attribute catalog table (remember that this table is referred to by the global variable attrCat), passing the appropriate attribute information from the attrList[] array as an instance of the AttrDesc structure (see below). Finally, create a HeapFile instance to hold tuples of the relation (hint: there is a procedure to do this which we have seen in the last project stage; you need to give it a string that is the relation name). Implement this function in create.C


const Status RelCatalog::createRel(const string & relation,
				   const int attrCnt,
				   const attrInfo attrList[])
{
    Status status;
    RelDesc rd;
    AttrDesc ad;
    attrInfo ai;

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

    int width = 0;

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

    // Add tuple to relcat relation
    //  make instance of RelDesc struct, then invoke RelCatalog::addInfo()
    if (relation.length() > MAXNAME) {
        return NAMETOOLONG;
    }

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
            // remove things already added?
            attrCat->dropRelation(relation);
            relCat->removeInfo(relation);
            return NAMETOOLONG;
        }
        strcpy(ad.attrName, ai.attrName);
        ad.attrOffset = offset;
        ad.attrType = attrList[i].attrType;
        ad.attrLen = attrList[i].attrLen;

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

