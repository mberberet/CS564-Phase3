/*
 * File:		    help.C	
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
 * Purpose: prints a list of all the relations in relcat if the relation 
 * passed in is empty. if the relation is not empty, it prints all the 
 * tuples in attrcat that are relevant to relName
 *
 * */

#include <sys/types.h>
#include <functional>
#include <string.h>
#include <stdio.h>
using namespace std;

#include "error.h"
#include "utility.h"
#include "catalog.h"



/* *
 * prints the relation info.  
 *
 * @param relation - empty if we want to print out all the 
 *                   relations in relcat
 *                   or holds the relation we are going to print
 *                   all the tuples of
 * 
 * @return status - returns...
 *                  OK if successful.
 *                  status otherwise    
 * */


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
        status = attrCat->getRelInfo(relation, attrCnt, attrs);
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
            printf("%-13s%-11d%-11d%-10d\n", attrs[i].attrName, 
                attrs[i].attrOffset, attrs[i].attrType, attrs[i].attrLen);
        }
    }
    return OK;

}
