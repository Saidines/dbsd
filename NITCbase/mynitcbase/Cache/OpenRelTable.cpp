#include "OpenRelTable.h"

#include <cstdlib>
#include <cstring>

/*
 * ============================================================
 * OpenRelTable — STAGE 3
 * ============================================================
 *
 * PURPOSE (VIVA):
 * ----------------
 * Initializes and manages the Relation & Attribute caches.
 *
 * In Stage-3:
 * • Only RELATIONCAT and ATTRIBUTECAT are opened
 * • Other relations are NOT supported yet
 *
 * In Stage-4:
 * • getRelId() is used by SELECT to identify relations
 */

OpenRelTable::OpenRelTable() {

    /* --------------------------------------------------------
     * STEP 1: Initialize cache arrays
     * -------------------------------------------------------- */
    for (int i = 0; i < MAX_OPEN; ++i) {
        RelCacheTable::relCache[i]  = nullptr;
        AttrCacheTable::attrCache[i] = nullptr;
    }

    /* --------------------------------------------------------
     * STEP 2: Load RELATIONCAT into Relation Cache
     * -------------------------------------------------------- */
    RecBuffer relCatBlock(RELCAT_BLOCK);
    Attribute relCatRecord[RELCAT_NO_ATTRS];

    relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);

    RelCacheEntry relCatEntry;
    RelCacheTable::recordToRelCatEntry(
        relCatRecord, &relCatEntry.relCatEntry);

    relCatEntry.recId.block = RELCAT_BLOCK;
    relCatEntry.recId.slot  = RELCAT_SLOTNUM_FOR_RELCAT;

    RelCacheTable::relCache[RELCAT_RELID] =
        (RelCacheEntry*) malloc(sizeof(RelCacheEntry));
    *(RelCacheTable::relCache[RELCAT_RELID]) = relCatEntry;

    /* --------------------------------------------------------
     * STEP 3: Load ATTRIBUTECAT into Relation Cache
     * -------------------------------------------------------- */
    relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_ATTRCAT);

    RelCacheEntry attrCatRelEntry;
    RelCacheTable::recordToRelCatEntry(
        relCatRecord, &attrCatRelEntry.relCatEntry);

    attrCatRelEntry.recId.block = RELCAT_BLOCK;
    attrCatRelEntry.recId.slot  = RELCAT_SLOTNUM_FOR_ATTRCAT;

    RelCacheTable::relCache[ATTRCAT_RELID] =
        (RelCacheEntry*) malloc(sizeof(RelCacheEntry));
    *(RelCacheTable::relCache[ATTRCAT_RELID]) = attrCatRelEntry;

    /* --------------------------------------------------------
     * STEP 4: Populate Attribute Cache for both catalogs
     * -------------------------------------------------------- */
    RecBuffer attrCatBlock(ATTRCAT_BLOCK);
    Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

    /* ---- RELATIONCAT attributes (slots 0–5) ---- */
    AttrCacheEntry *head = nullptr, *prev = nullptr;

    for (int i = 0; i < 6; i++) {
        attrCatBlock.getRecord(attrCatRecord, i);

        AttrCacheEntry *node =
            (AttrCacheEntry*) malloc(sizeof(AttrCacheEntry));

        AttrCacheTable::recordToAttrCatEntry(
            attrCatRecord, &node->attrCatEntry);

        node->recId.block = ATTRCAT_BLOCK;
        node->recId.slot  = i;
        node->next = nullptr;

        if (head == nullptr) head = node;
        else prev->next = node;

        prev = node;
    }
    AttrCacheTable::attrCache[RELCAT_RELID] = head;

    /* ---- ATTRIBUTECAT attributes (slots 6–11) ---- */
    head = nullptr;
    prev = nullptr;

    for (int i = 6; i < 12; i++) {
        attrCatBlock.getRecord(attrCatRecord, i);

        AttrCacheEntry *node =
            (AttrCacheEntry*) malloc(sizeof(AttrCacheEntry));

        AttrCacheTable::recordToAttrCatEntry(
            attrCatRecord, &node->attrCatEntry);

        node->recId.block = ATTRCAT_BLOCK;
        node->recId.slot  = i;
        node->next = nullptr;

        if (head == nullptr) head = node;
        else prev->next = node;

        prev = node;
    }
    AttrCacheTable::attrCache[ATTRCAT_RELID] = head;
}

/*
 * ============================================================
 * OpenRelTable::getRelId() — STAGE 4
 * ============================================================
 *
 * PURPOSE (VIVA):
 * ----------------
 * Returns the relId of an OPEN relation.
 *
 * CURRENT STAGE LIMITATION:
 * -------------------------
 * • Only RELATIONCAT and ATTRIBUTECAT are open
 * • Other relations return E_RELNOTOPEN
 */
int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {

    if (strcmp(relName, RELCAT_RELNAME) == 0) {
        return RELCAT_RELID;
    }

    if (strcmp(relName, ATTRCAT_RELNAME) == 0) {
        return ATTRCAT_RELID;
    }

    /*
     * Any other relation is NOT open in Stage-3/4
     */
    return E_RELNOTOPEN;
}

/*
 * ============================================================
 * OpenRelTable Destructor
 * ============================================================
 */
OpenRelTable::~OpenRelTable() {

    for (int i = 0; i < MAX_OPEN; i++) {
        if (RelCacheTable::relCache[i]) {
            free(RelCacheTable::relCache[i]);
            RelCacheTable::relCache[i] = nullptr;
        }
    }

    for (int i = 0; i < MAX_OPEN; i++) {
        AttrCacheEntry *curr = AttrCacheTable::attrCache[i];
        while (curr) {
            AttrCacheEntry *tmp = curr;
            curr = curr->next;
            free(tmp);
        }
        AttrCacheTable::attrCache[i] = nullptr;
    }
}

