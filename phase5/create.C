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
    if (relCat->getInfo(relation, rd) != OK) {
        return RELEXISTS;
    }

    // Add tuple to relcat relation
    //  make instance of RelDesc struct, then invoke RelCatalog::addInfo()
    if (relation.length() > MAXNAME) {
        return NAMETOOLONG;
    }
    relation.copy(rd.relName, relation.length(), 0);
    rd.attrCnt = attrCnt;
    relCat->addInfo(rd);

    // For the attributes, invoke AttrCatalog::addInfo() (attrCat)
    //  make instance of AttrDesc struct using info from attrList
    int offset = 0;
    for (int i = 0; i < attrCnt; i++) {
        ai = attrList[i];
        if (strlen(ai.attrName) > MAXNAME) {
            // remove things already added?
            attrCat->dropRelation(relation);
            relCat->removeInfo(relation);
            return NAMETOOLONG;
        }
        // potentially wrong
        strcpy(ad.relName, ai.relName);
        strcpy(ad.attrName, ai.attrName);
        ad.attrOffset = offset;
        ad.attrType = ai.attrType;
        ad.attrLen = ai.attrLen;

        offset += ai.attrLen;
    }

    //  Create a heapfile instace to hold the tuples of the relation (look at last project)
    if ((status = createHeapFile(relation)) != OK) {
        attrCat->dropRelation(relation);
        relCat->removeInfo(relation);
        return status;
    }
    return OK;
}

