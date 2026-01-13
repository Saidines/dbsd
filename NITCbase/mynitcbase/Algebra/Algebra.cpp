#include "Algebra.h"

#include <cstring>
#include <cstdio>   // ✅ printf, sscanf
#include <cstdlib>  // ✅ atof

/* ------------------------------------------------------------
 * Forward declaration (REQUIRED in C++)
 * ------------------------------------------------------------ */
bool isNumber(char *str);

/*
 * ============================================================
 * Algebra::select()
 * ============================================================
 *
 * PURPOSE (VIVA):
 * ----------------
 * Selects and prints all records from a relation that satisfy
 * a given condition.
 *
 * NOTE:
 * -----
 * • In Stage-4, results are PRINTED (temporary)
 * • In later stages, results will be inserted into a relation
 */
int Algebra::select(char srcRel[ATTR_SIZE],
                    char targetRel[ATTR_SIZE],
                    char attr[ATTR_SIZE],
                    int op,
                    char strVal[ATTR_SIZE]) {

    /* --------------------------------------------------------
     * STEP 1: Get relId
     * -------------------------------------------------------- */
    int srcRelId = OpenRelTable::getRelId(srcRel);
    if (srcRelId == E_RELNOTOPEN) {
        return E_RELNOTOPEN;
    }
    /* --------------------------------------------------------
     * STEP 2: Get attribute metadata
     * -------------------------------------------------------- */
    AttrCatEntry attrCatEntry;
    int ret = AttrCacheTable::getAttrCatEntry(
                  srcRelId, attr, &attrCatEntry);

    if (ret != SUCCESS) {
	    return E_ATTRNOTEXIST;
    }

    /* --------------------------------------------------------
     * STEP 3: Convert condition value
     * -------------------------------------------------------- */
    Attribute attrVal;

    if (attrCatEntry.attrType == NUMBER) {
        if (isNumber(strVal)) {
            attrVal.nVal = atof(strVal);
        } else {
            return E_ATTRTYPEMISMATCH;
        }
    } else { // STRING
        strcpy(attrVal.sVal, strVal);
    }

    /* --------------------------------------------------------
     * STEP 4: Reset search index
     * -------------------------------------------------------- */
    RelCacheTable::resetSearchIndex(srcRelId);

    /* --------------------------------------------------------
     * STEP 5: Get relation metadata
     * -------------------------------------------------------- */
    RelCatEntry relCatEntry;
    RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry);

    /* --------------------------------------------------------
     * STEP 6: Print header
     * -------------------------------------------------------- */
    printf("|");
    for (int i = 0; i < relCatEntry.numAttrs; i++) {
        AttrCatEntry temp;
        AttrCacheTable::getAttrCatEntry(srcRelId, i, &temp);
        printf(" %s |", temp.attrName);
    }
    printf("\n");

    /* --------------------------------------------------------
     * STEP 7: Linear search loop
     * -------------------------------------------------------- */
    while (true) {

        RecId recId =
            BlockAccess::linearSearch(
                srcRelId, attr, attrVal, op);

        if (recId.block == -1 && recId.slot == -1) {
            break;
        }

        RecBuffer recBuffer(recId.block);
        Attribute record[relCatEntry.numAttrs];
        recBuffer.getRecord(record, recId.slot);

        printf("|");
        for (int i = 0; i < relCatEntry.numAttrs; i++) {
            AttrCatEntry temp;
            AttrCacheTable::getAttrCatEntry(srcRelId, i, &temp);

            if (temp.attrType == NUMBER)
                printf(" %.0f |", record[i].nVal);
            else
                printf(" %s |", record[i].sVal);
        }
        printf("\n");
    }

    return SUCCESS;
}

/* ------------------------------------------------------------
 * Helper: checks if string is numeric
 * ------------------------------------------------------------ */
bool isNumber(char *str) {
    int len;
    float ignore;
    int ret = sscanf(str, "%f %n", &ignore, &len);
    return ret == 1 && len == strlen(str);
}

