#include "Buffer/StaticBuffer.h"      // Buffer layer (used in later stages)
#include "Buffer/BlockBuffer.h"       // Provides getHeader() and getRecord()
#include "Cache/OpenRelTable.h"        // Cache layer (not used yet)
#include "Disk_Class/Disk.h"           // Physical layer (disk I/O)
#include "FrontendInterface/FrontendInterface.h" // Frontend (disabled here)

#include <iostream>
#include <cstring>
#include <cstdio>

/*
 * ============================================================
 * STAGE 2 – EXTENDED (Q1 + Q2)
 * ============================================================
 * Q1: Read Attribute Catalog across MULTIPLE blocks
 * Q2: Rename Students.Class → Students.Batch
 *
 * Demonstrates:
 *  - How metadata is stored as relations
 *  - Catalog linked-list traversal (rblock pointer)
 *  - Metadata update (schema change)
 *  - Disk-safe read–modify–write
 *
 * VIVA POINT:
 * This is NOT a frontend feature.
 * This is an INTERNAL DBMS operation.
 */

int main(int argc, char *argv[]) {

    /*
     * Disk object (MANDATORY):
     * ------------------------
     * Constructor:
     *   - Creates runtime copy of disk.xfs → disk_run_copy
     * Destructor:
     *   - Writes changes back ONLY on graceful exit
     *
     * VIVA POINT:
     * Disk access must NEVER bypass Disk class.
     */
    Disk disk_run;   // Runtime disk copy (MANDATORY)

    // ----------------------------------------------------------
    // RELATION CATALOG (Single block: Block 4)
    // ----------------------------------------------------------
    /*
     * Relation Catalog is guaranteed to fit in ONE block.
     * Each entry represents ONE relation in the database.
     */
    RecBuffer relCatBuffer(RELCAT_BLOCK);

    HeadInfo relCatHeader;

    /*
     * Header gives:
     *   - numEntries → total number of relations
     *   - numAttrs   → attributes per catalog record (fixed = 6)
     *
     * VIVA POINT:
     * DBMS NEVER assumes counts — it always reads metadata.
     */
    relCatBuffer.getHeader(&relCatHeader);

    // ----------------------------------------------------------
    // ATTRIBUTE CATALOG (May span MULTIPLE blocks)
    // ----------------------------------------------------------
    /*
     * Attribute Catalog is a LINKED LIST of record blocks.
     * Traversal is done using rblock field in header.
     *
     * Why linked list?
     * → Attribute count is not bounded by one block.
     */
    int32_t attrBlockNum = ATTRCAT_BLOCK;

    // ----------------------------------------------------------
    // Traverse Relation Catalog (OUTER LOOP)
    // ----------------------------------------------------------
    /*
     * For each relation:
     *   - Print relation name
     *   - Scan Attribute Catalog to find its attributes
     *
     * OUTER LOOP → Relations
     * INNER LOOP → Attributes
     */
    for (int32_t i = 0; i < relCatHeader.numEntries; i++) {

        /*
         * Relation Catalog record format:
         * [ RelName | #Attrs | #Records | FirstBlock | LastBlock | #Slots ]
         */
        Attribute relCatRecord[RELCAT_NO_ATTRS];
        relCatBuffer.getRecord(relCatRecord, i);

        printf("Relation: %s\n",
               relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

        /*
         * IMPORTANT (VIVA POINT):
         * Attribute catalog traversal must RESET
         * for EACH relation.
         */
        attrBlockNum = ATTRCAT_BLOCK;

        // ------------------------------------------------------
        // Q1: Traverse Attribute Catalog BLOCK BY BLOCK
        // ------------------------------------------------------
        /*
         * Sequential scan because:
         *   - No index on catalogs
         *   - Catalogs are system relations
         *   - Small size → linear scan acceptable
         */
        while (attrBlockNum != INVALID_BLOCKNUM) {

            RecBuffer attrCatBuffer(attrBlockNum);
            HeadInfo attrCatHeader;

            /*
             * Header provides:
             *   - numEntries → attributes in THIS block
             *   - rblock     → pointer to next catalog block
             */
            attrCatBuffer.getHeader(&attrCatHeader);

            for (int32_t j = 0; j < attrCatHeader.numEntries; j++) {

                /*
                 * Attribute Catalog record format:
                 * [ RelName | AttrName | AttrType | PrimaryFlag | RootBlock | Offset ]
                 */
                Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
                attrCatBuffer.getRecord(attrCatRecord, j);

                /*
                 * Attribute belongs to relation IF:
                 *   attr.RelName == rel.RelName
                 *
                 * strcmp is required because names are C-strings.
                 */
                if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal,
                           relCatRecord[RELCAT_REL_NAME_INDEX].sVal) == 0) {

                    // --------------------------------------------------
                    // Q2: Rename Students.Class → Students.Batch
                    // --------------------------------------------------
                    /*
                     * VIVA POINT:
                     * Schema changes affect ONLY catalogs,
                     * NOT record blocks.
                     *
                     * Hence we modify Attribute Catalog,
                     * NOT data blocks.
                     */
                    if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, "Students") == 0 &&
                        strcmp(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, "Class") == 0) {

                        /*
                         * Update attribute name IN-MEMORY
                         */
                        strcpy(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, "Batch");

                        /*
                         * Disk-safe update procedure:
                         *   1) Read full block
                         *   2) Validate slot occupancy
                         *   3) Modify only target record
                         *   4) Write full block back
                         */
                        unsigned char rawBlock[BLOCK_SIZE];
                        Disk::readBlock(rawBlock, attrBlockNum);

                        /*
                         * SLOT MAP SAFETY CHECK (IMPORTANT FOR VIVA)
                         * -----------------------------------------
                         * Even though catalog slots are dense,
                         * DBMS design mandates correctness.
                         */
                        unsigned char *slotMap = rawBlock + HEADER_SIZE;
                        if (slotMap[j] == SLOT_UNOCCUPIED) {
                            // Defensive check (should not happen in catalogs)
                            continue;
                        }

                        int recordSize = ATTRCAT_NO_ATTRS * ATTR_SIZE;
                        int offset = HEADER_SIZE + attrCatHeader.numSlots
                                     + (j * recordSize);

                        memcpy(rawBlock + offset,
                               attrCatRecord,
                               recordSize);

                        Disk::writeBlock(rawBlock, attrBlockNum);
                    }

                    /*
                     * AttributeType:
                     * NUMBER → NUM
                     * STRING → STR
                     */
                    const char *attrType =
                        (attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER)
                        ? "NUM" : "STR";

                    printf("  %s: %s\n",
                           attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,
                           attrType);
                }
            }

            /*
             * Move to NEXT attribute catalog block
             * using linked-list pointer.
             */
            attrBlockNum = attrCatHeader.rblock;
        }

        printf("\n");
    }

    /*
     * FrontendInterface is NOT invoked here.
     * This is INTERNAL DBMS testing code.
     */
    return 0;
}

