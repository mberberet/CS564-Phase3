#include <sys/types.h>
#include <functional>
#include <string.h>
#include <stdio.h>
using namespace std;

#include "error.h"
#include "utility.h"
#include "catalog.h"

// define if debug output wanted


//
// Retrieves and prints information from the catalogs about the for the
// user. If no relation is given (relation is NULL), then it lists all
// the relations in the database, along with the width in bytes of the
// relation, the number of attributes in the relation, and the number of
// attributes that are indexed.  If a relation is given, then it lists
// all of the attributes of the relation, as well as its type, length,
// and offset, whether it's indexed or not, and its index number.
//
// Returns:
// 	OK on success
// 	error code otherwise
//

const Status RelCatalog::help(const string & relation)
{
    Status status;
   // RelDesc rd;
    AttrDesc *attrs;
    int attrCnt;
   int* attrWidth;
    if (relation.empty()) {
        return UT_Print(RELCATNAME);
    } else{
        status = attrCat -> getRelInfo(relation, attrCnt, attrs);
        if (status != OK){
            return status;
        }
        status = UT_computeWidth(attrCnt, attrs, attrWidth);
        if (status != OK){
            return status;
        }
        printf("relation %s\n\n", relation.c_str());
        printf("attrName    attrOffset   attrType    attrLen \n");
        printf("----------- ---------- ---------- ----------\n");
        for (int i = 0;i < attrCnt; i++){
            printf("%s %-*d %-*d %-*d\n ", attrs[i].attrName, 20, attrs[i].attrOffset, 22 , attrs[i].attrType, 43, attrs[i].attrLen);
        }
    }   
    return OK;

}
