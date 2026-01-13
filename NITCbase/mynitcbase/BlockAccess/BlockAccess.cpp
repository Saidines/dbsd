#include "BlockAccess.h"

#include <cstring>

/*
 * ============================================================
 * BlockAccess::linearSearch()
 * ============================================================
 *
 * PURPOSE (VIVA):
 * ----------------
 * Performs a LINEAR SEARCH on a relation to find the NEXT record
 * that satisfies the condition:
 *
 *      value-in-record  op  attrVal
 *
 * Uses the searchIndex stored in RelCacheTable to resume search
 * from the previous hit.
 *
 * RETURNS:
 * --------
 *  RecId{block, slot} → next matching record
 *  RecId{-1, -1}      → no more matching records
 *
 * IMPORTANT (VIVA):
 * ------------------
 * • searchIndex is UPDATED on every successful hit
 * • searchIndex is SHARED with project()
 * • Caller must reset searchIndex manually if needed
 */
RecId BlockAccess::linearSearch(
        int relId,
        char attrName[ATTR_SIZE],
        union Attribute attrVal,
        int op) {

    /* --------------------------------------------------------
     * STEP 1: Get previous search index
     * -------------------------------------------------------- */
    RecId prevRecId;
    RelCacheTable::getSearchIndex(relId, &prevRecId);

    int block, slot;

    /* --------------------------------------------------------
     * STEP 2: Decide starting point
     * -------------------------------------------------------- */
    if (prevRecId.block == -1 && prevRecId.slot == -1) {
        /* Start from FIRST record of the relation */

        RelCatEntry relCatEntry;
        RelCacheTable::getRelCatEntry(relId, &relCatEntry);

        block = relCatEntry.firstBlk;
        slot  = 0;
    } else {
        /* Resume search from NEXT record */
        block = prevRecId.block;
        slot  = prevRecId.slot + 1;
    }

    /* --------------------------------------------------------
     * STEP 3: Linear scan over record blocks
     * -------------------------------------------------------- */
    while (block != -1) {

        RecBuffer recBuffer(block);

        HeadInfo head;
        recBuffer.getHeader(&head);

        int numSlots = head.numSlots;

        unsigned char slotMap[numSlots];
        recBuffer.getSlotMap(slotMap);

        /* ----------------------------------------------------
         * STEP 4: Scan slots within the block
         * ---------------------------------------------------- */
        while (slot < numSlots) {

            /* Skip free slots */
            if (slotMap[slot] == SLOT_UNOCCUPIED) {
                slot++;
                continue;
            }

            /* Read record */
            union Attribute record[head.numAttrs];
            recBuffer.getRecord(record, slot);

            /* ------------------------------------------------
             * STEP 5: Get attribute metadata
             * ------------------------------------------------ */
            AttrCatEntry attrCatEntry;
            AttrCacheTable::getAttrCatEntry(
                relId, attrName, &attrCatEntry);

            int attrOffset = attrCatEntry.offset;

            /* ------------------------------------------------
             * STEP 6: Compare attribute values
             * ------------------------------------------------ */
            int cmpVal = compareAttrs(
                record[attrOffset],
                attrVal,
                attrCatEntry.attrType
            );

            /* ------------------------------------------------
             * STEP 7: Check condition
             * ------------------------------------------------ */
            if (
                (op == NE && cmpVal != 0) ||
                (op == LT && cmpVal <  0) ||
                (op == LE && cmpVal <= 0) ||
                (op == EQ && cmpVal == 0) ||
                (op == GT && cmpVal >  0) ||
                (op == GE && cmpVal >= 0)
            ) {
                /* Update search index */
                RecId currRecId{block, slot};
                RelCacheTable::setSearchIndex(relId, &currRecId);

                return currRecId;
            }

            slot++;
        }

        /* ----------------------------------------------------
         * STEP 8: Move to next block
         * ---------------------------------------------------- */
        block = head.rblock;
        slot  = 0;
    }

    /* --------------------------------------------------------
     * STEP 9: No more matching records
     * -------------------------------------------------------- */
    return RecId{-1, -1};
}

