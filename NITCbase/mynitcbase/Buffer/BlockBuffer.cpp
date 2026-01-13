#include "BlockBuffer.h"

#include <cstdlib>
#include <cstring>

/*
 * ============================================================
 * BLOCK BUFFER (BUFFER LAYER) — STAGE 3 + STAGE 4
 * ============================================================
 *
 * RESPONSIBILITIES (VIVA):
 * -----------------------
 * • Acts as the ONLY gateway from higher layers to disk blocks
 * • Ensures all disk I/O goes through StaticBuffer
 * • Provides safe access to:
 *     - Block headers
 *     - Records
 *     - Slot maps (Stage-4)
 *
 * IMPORTANT:
 * -----------
 * Higher layers NEVER call Disk::readBlock() directly.
 */

/* ============================================================
 * BlockBuffer Constructor
 * ============================================================
 * Used for ANY existing block on disk.
 */
BlockBuffer::BlockBuffer(int blockNum) {
    this->blockNum = blockNum;
}

/* ============================================================
 * RecBuffer Constructor
 * ============================================================
 * Used ONLY for record blocks.
 */
RecBuffer::RecBuffer(int blockNum) : BlockBuffer(blockNum) {
    // No extra initialization needed
}

/* ============================================================
 * BlockBuffer::getHeader()
 * ============================================================
 * Reads common header fields from ANY block.
 */
int BlockBuffer::getHeader(struct HeadInfo *head) {

    unsigned char *bufferPtr;

    int ret = loadBlockAndGetBufferPtr(&bufferPtr);
    if (ret != SUCCESS) {
        return ret;
    }

    /*
     * HEADER LAYOUT (32 bytes)
     */
    memcpy(&head->numEntries, bufferPtr + 16, 4);
    memcpy(&head->numAttrs,   bufferPtr + 20, 4);
    memcpy(&head->numSlots,   bufferPtr + 24, 4);
    memcpy(&head->lblock,     bufferPtr + 8,  4);
    memcpy(&head->rblock,     bufferPtr + 12, 4);

    return SUCCESS;
}

/* ============================================================
 * RecBuffer::getRecord()
 * ============================================================
 * Returns slotNum-th record if slot is occupied.
 */
int RecBuffer::getRecord(union Attribute *rec, int slotNum) {

    unsigned char *bufferPtr;

    int ret = loadBlockAndGetBufferPtr(&bufferPtr);
    if (ret != SUCCESS) {
        return ret;
    }

    HeadInfo head;
    getHeader(&head);

    int numAttrs = head.numAttrs;
    int numSlots = head.numSlots;

    if (slotNum < 0 || slotNum >= numSlots) {
        return E_OUTOFBOUND;
    }

    unsigned char *slotMap = bufferPtr + HEADER_SIZE;
    if (slotMap[slotNum] == SLOT_UNOCCUPIED) {
        return E_FREESLOT;
    }

    int recordSize = numAttrs * ATTR_SIZE;
    int offset = HEADER_SIZE + numSlots + (slotNum * recordSize);

    memcpy(rec, bufferPtr + offset, recordSize);

    return SUCCESS;
}

/* ============================================================
 * RecBuffer::getSlotMap()  (STAGE 4)
 * ============================================================
 * Copies the slot map of a record block.
 *
 * WHY THIS EXISTS (VIVA):
 * ----------------------
 * • Needed for linear search
 * • Allows skipping free slots
 * • Ensures correctness (no dense-slot assumption)
 */
int RecBuffer::getSlotMap(unsigned char *slotMap) {

    unsigned char *bufferPtr;

    /* Load block into buffer */
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);
    if (ret != SUCCESS) {
        return ret;
    }

    /* Read header to get number of slots */
    HeadInfo head;
    getHeader(&head);

    int slotCount = head.numSlots;

    /* Slotmap begins immediately after header */
    unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;

    /* Copy slotmap to caller buffer */
    memcpy(slotMap, slotMapInBuffer, slotCount);

    return SUCCESS;
}

/* ============================================================
 * BlockBuffer::loadBlockAndGetBufferPtr()
 * ============================================================
 * Core buffer manager interaction.
 */
int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr) {

    int bufferNum = StaticBuffer::getBufferNum(this->blockNum);

    if (bufferNum == E_BLOCKNOTINBUFFER) {

        bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);
        if (bufferNum == E_OUTOFBOUND) {
            return E_OUTOFBOUND;
        }

        Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
    }

    *buffPtr = StaticBuffer::blocks[bufferNum];

    return SUCCESS;
}

/* ============================================================
 * compareAttrs()
 * ============================================================
 * Compares two attributes based on attribute type.
 *
 * RETURNS:
 * --------
 * < 0  → attr1 < attr2
 * = 0  → attr1 == attr2
 * > 0  → attr1 > attr2
 *
 * VIVA:
 * -----
 * • NUMBER → numeric comparison
 * • STRING → lexicographic comparison
 */
int compareAttrs(Attribute attr1, Attribute attr2, int attrType) {

    if (attrType == NUMBER) {
        if (attr1.nVal < attr2.nVal) return -1;
        if (attr1.nVal > attr2.nVal) return  1;
        return 0;
    }

    // STRING comparison
    return strcmp(attr1.sVal, attr2.sVal);
}

