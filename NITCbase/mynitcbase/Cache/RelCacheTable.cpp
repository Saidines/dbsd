#include "RelCacheTable.h"

#include <cstring>

/*
 * ============================================================
 * RELATION CACHE TABLE — STAGE 3 + STAGE 4
 * ============================================================
 *
 * PURPOSE:
 * --------
 * RelCacheTable caches Relation Catalog entries of OPEN relations
 * in main memory to avoid repeated disk access.
 *
 * STAGE-3:
 * --------
 * • Load and return Relation Catalog metadata
 *
 * STAGE-4 ADDITION:
 * -----------------
 * • Maintain searchIndex for linear search
 *
 * VIVA ONE-LINER:
 * ---------------
 * "RelCacheTable stores relation metadata and
 *  maintains search state for selection queries."
 */

/* ------------------------------------------------------------
 * Static member definition
 * ------------------------------------------------------------
 * relCache[i] == nullptr  → relation NOT open
 * relCache[i] != nullptr  → relation IS open
 */
RelCacheEntry* RelCacheTable::relCache[MAX_OPEN];

/*
 * ============================================================
 * getRelCatEntry()
 * ============================================================
 * Fetch Relation Catalog entry from cache.
 */
int RelCacheTable::getRelCatEntry(int relId, RelCatEntry* relCatBuf) {

    /* Validate relId */
    if (relId < 0 || relId >= MAX_OPEN) {
        return E_OUTOFBOUND;
    }

    /* Relation must be open */
    if (relCache[relId] == nullptr) {
        return E_RELNOTOPEN;
    }

    /* Copy cached schema metadata */
    *relCatBuf = relCache[relId]->relCatEntry;
    return SUCCESS;
}

/*
 * ============================================================
 * recordToRelCatEntry()
 * ============================================================
 * Converts raw catalog record → RelCatEntry struct
 */
void RelCacheTable::recordToRelCatEntry(
        union Attribute record[RELCAT_NO_ATTRS],
        RelCatEntry* relCatEntry) {

    strcpy(relCatEntry->relName,
           record[RELCAT_REL_NAME_INDEX].sVal);

    relCatEntry->numAttrs =
        (int) record[RELCAT_NO_ATTRIBUTES_INDEX].nVal;

    relCatEntry->numRecs =
        (int) record[RELCAT_NO_RECORDS_INDEX].nVal;

    relCatEntry->firstBlk =
        (int) record[RELCAT_FIRST_BLOCK_INDEX].nVal;

    relCatEntry->lastBlk =
        (int) record[RELCAT_LAST_BLOCK_INDEX].nVal;

    relCatEntry->numSlotsPerBlk =
        (int) record[RELCAT_NO_SLOTS_PER_BLOCK_INDEX].nVal;
}

/*
 * ============================================================
 * STAGE 4 — SEARCH INDEX FUNCTIONS
 * ============================================================
 *
 * searchIndex stores the RecId {block, slot} of the LAST MATCH
 * during a linear search.
 *
 * {-1, -1} → start search from beginning
 */

/*
 * ------------------------------------------------------------
 * getSearchIndex()
 * ------------------------------------------------------------
 * Returns current searchIndex of relation
 */
int RelCacheTable::getSearchIndex(int relId, RecId* searchIndex) {

    /* Validate relId */
    if (relId < 0 || relId >= MAX_OPEN) {
        return E_OUTOFBOUND;
    }

    /* Relation must be open */
    if (relCache[relId] == nullptr) {
        return E_RELNOTOPEN;
    }

    /* Copy search index */
    *searchIndex = relCache[relId]->searchIndex;
    return SUCCESS;
}

/*
 * ------------------------------------------------------------
 * setSearchIndex()
 * ------------------------------------------------------------
 * Updates searchIndex after a successful match
 */
int RelCacheTable::setSearchIndex(int relId, RecId* searchIndex) {

    /* Validate relId */
    if (relId < 0 || relId >= MAX_OPEN) {
        return E_OUTOFBOUND;
    }

    /* Relation must be open */
    if (relCache[relId] == nullptr) {
        return E_RELNOTOPEN;
    }

    /* Update search state */
    relCache[relId]->searchIndex = *searchIndex;
    return SUCCESS;
}

/*
 * ------------------------------------------------------------
 * resetSearchIndex()
 * ------------------------------------------------------------
 * Resets search state to beginning
 *
 * Used before starting a new SELECT query
 */
int RelCacheTable::resetSearchIndex(int relId) {

    RecId resetIdx;
    resetIdx.block = -1;
    resetIdx.slot  = -1;

    return setSearchIndex(relId, &resetIdx);
}

