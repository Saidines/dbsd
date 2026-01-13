#include "AttrCacheTable.h"
#include <cstring>
#include<cstdio>
/*
 * ============================================================
 * ATTRIBUTE CACHE TABLE — STAGE 3 + STAGE 4
 * ============================================================
 *
 * PURPOSE:
 * --------
 * AttrCacheTable caches Attribute Catalog entries of OPEN relations.
 *
 * DESIGN (VIVA POINTS):
 * --------------------
 * • One linked list per open relation
 * • attrCache[relId] → head of linked list of attributes
 * • Linked list is used because:
 *     - Relations have variable number of attributes
 *     - Attribute order matters (offset)
 *
 * • Static class → single global cache
 */

/* ------------------------------------------------------------
 * Static member definition
 * ------------------------------------------------------------
 * attrCache[i] == nullptr → relation not open
 * attrCache[i] != nullptr → head of attribute linked list
 */
AttrCacheEntry* AttrCacheTable::attrCache[MAX_OPEN];

/*
 * ============================================================
 * getAttrCatEntry() — BY ATTRIBUTE OFFSET
 * ============================================================
 */
int AttrCacheTable::getAttrCatEntry(int relId,
                                    int attrOffset,
                                    AttrCatEntry* attrCatBuf) {

    /* Validate relId */
    if (relId < 0 || relId >= MAX_OPEN) {
        return E_OUTOFBOUND;
    }

    /* Relation must be open */
    if (attrCache[relId] == nullptr) {
        return E_RELNOTOPEN;
    }

    /* Traverse attribute linked list */
    for (AttrCacheEntry* entry = attrCache[relId];
         entry != nullptr;
         entry = entry->next) {

        if (entry->attrCatEntry.offset == attrOffset) {
            *attrCatBuf = entry->attrCatEntry;
            return SUCCESS;
        }
    }

    return E_ATTRNOTEXIST;
}

/*
 * ============================================================
 * getAttrCatEntry() — BY ATTRIBUTE NAME (STAGE 4)
 * ============================================================
 *
 * PURPOSE (VIVA):
 * ----------------
 * Used during SELECT WHERE queries to locate
 * the attribute participating in the condition.
 *
 * Example:
 *   SELECT * FROM RELATIONCAT WHERE #Records > 10;
 */
int AttrCacheTable::getAttrCatEntry(int relId,
                                    char attrName[ATTR_SIZE],
                                    AttrCatEntry* attrCatBuf) {

    /* Validate relId */
	if (relId < 0 || relId >= MAX_OPEN) {
        return E_OUTOFBOUND;
    }
    /* Relation must be open */
    if (attrCache[relId] == nullptr) {
        return E_RELNOTOPEN;
    }
    /* Traverse attribute linked list */
    for (AttrCacheEntry* entry = attrCache[relId];
         entry != nullptr;
         entry = entry->next) {

        /* Match attribute name */
        if (strcmp(entry->attrCatEntry.attrName, attrName) == 0) {
            *attrCatBuf = entry->attrCatEntry;
            return SUCCESS;
        }
	else{
	}
    }

    /* Attribute not found */
    return E_ATTRNOTEXIST;
}

/*
 * ============================================================
 * recordToAttrCatEntry()
 * ============================================================
 *
 * Converts raw Attribute Catalog record → AttrCatEntry struct
 */
void AttrCacheTable::recordToAttrCatEntry(
        union Attribute record[ATTRCAT_NO_ATTRS],
        AttrCatEntry* attrCatEntry) {

    strcpy(attrCatEntry->relName,
           record[ATTRCAT_REL_NAME_INDEX].sVal);

    strcpy(attrCatEntry->attrName,
           record[ATTRCAT_ATTR_NAME_INDEX].sVal);

    attrCatEntry->attrType =
        (int) record[ATTRCAT_ATTR_TYPE_INDEX].nVal;

    attrCatEntry->primaryFlag =
        (bool) record[ATTRCAT_PRIMARY_FLAG_INDEX].nVal;

    attrCatEntry->rootBlock =
        (int) record[ATTRCAT_ROOT_BLOCK_INDEX].nVal;

    attrCatEntry->offset =
        (int) record[ATTRCAT_OFFSET_INDEX].nVal;
}

