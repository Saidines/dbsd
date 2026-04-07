#include "BlockAccess.h"
#include <stdlib.h>
#include <cstring>
#include<iostream>
RecId BlockAccess::linearSearch(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, int op) {
    // get the previous search index of the relation relId from the relation cache
    // (use RelCacheTable::getSearchIndex() function)
    RecId prevRecId;
    RelCacheTable::getSearchIndex(relId, &prevRecId);

    // let block and slot denote the record id of the record being currently checked
    int block = -1, slot = -1;

    // if the current search index record is invalid(i.e. both block and slot = -1)
    if (prevRecId.block == -1 && prevRecId.slot == -1)
    {
        // (no hits from previous search; search should start from the
        // first record itself)

        // get the first record block of the relation from the relation cache
        // (use RelCacheTable::getRelCatEntry() function of Cache Layer)
        RelCatEntry relCatEntry;
        RelCacheTable::getRelCatEntry(relId, &relCatEntry);

        // block = first record block of the relation
        block = relCatEntry.firstBlk;
        // slot = 0
        slot = 0;
    }
    else
    {
        // (there is a hit from previous search; search should start from
        // the record next to the search index record)

        // block = search index's block
        block = prevRecId.block;
        // slot = search index's slot + 1
        slot = prevRecId.slot + 1;
    }

    /* The following code searches for the next record in the relation
       that satisfies the given condition
       We start from the record id (block, slot) and iterate over the remaining
       records of the relation
    */
    while (block != -1)
    {
        /* create a RecBuffer object for block (use RecBuffer Constructor for
           existing block) */
        RecBuffer recBuffer(block);

        HeadInfo head;
        Attribute catRecord[RELCAT_NO_ATTRS];

        // get the record with id (block, slot) using RecBuffer::getRecord()
        recBuffer.getRecord(catRecord, slot);
        // get header of the block using RecBuffer::getHeader() function
        recBuffer.getHeader(&head);
        // get slot map of the block using RecBuffer::getSlotMap() function
        unsigned char slotMap[head.numSlots];
        recBuffer.getSlotMap(slotMap);

        // If slot >= the number of slots per block(i.e. no more slots in this block)
        if (slot >= head.numSlots) {
            // update block = right block of block
            block = head.rblock;
            // update slot = 0
            slot = 0;
            continue;  // continue to the beginning of this while loop
        }

        // if slot is free skip the loop
        // (i.e. check if slot'th entry in slot map of block contains SLOT_UNOCCUPIED)
        if (slotMap[slot] == SLOT_UNOCCUPIED) {
            // increment slot and continue to the next record slot
            slot++;
            continue;
        }

        // compare record's attribute value to the the given attrVal as below:
        /*
        firstly get the attribute offset for the attrName attribute
        from the attribute cache entry of the relation using
        AttrCacheTable::getAttrCatEntry()
        */
        AttrCatEntry attrCatEntry;
        AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);


        // use the attribute offset to get the value of the attribute from current record
        Attribute *record =(Attribute *)malloc(sizeof(Attribute) * head.numAttrs);
        recBuffer.getRecord(record, slot);
        int attrOffset=attrCatEntry.offset;

        int cmpVal;  // will store the difference between the attributes
        // set cmpVal using compareAttrs()
        cmpVal = compareAttrs(record[attrOffset], attrVal, attrCatEntry.attrType);

        /* Next task is to check whether this record satisfies the given condition.
           It is determined based on the output of previous comparison and
           the op value received.
           The following code sets the cond variable if the condition is satisfied.
        */
        if (
            (op == NE && cmpVal != 0) ||    // if op is "not equal to"
            (op == LT && cmpVal < 0) ||     // if op is "less than"
            (op == LE && cmpVal <= 0) ||    // if op is "less than or equal to"
            (op == EQ && cmpVal == 0) ||    // if op is "equal to"
            (op == GT && cmpVal > 0) ||     // if op is "greater than"
            (op == GE && cmpVal >= 0)       // if op is "greater than or equal to"
        ) {
            /*
            set the search index in the relation cache as
            the record id of the record that satisfies the given condition
            (use RelCacheTable::setSearchIndex function)
            */
            RecId newIndex = {block, slot};
            RelCacheTable::setSearchIndex(relId, &newIndex);

            return RecId{block, slot};
        }

        slot++;
    }

    // no record in the relation with Id relid satisfies the given condition
    return RecId{-1, -1};
}

int BlockAccess::renameRelation(char oldName[ATTR_SIZE], char newName[ATTR_SIZE])
{
     if((strcmp("RELATIONCAT",oldName)&&strcmp("ATTRIBUTECAT",oldName))==0){
    return E_NOTPERMITTED;    
 }

  if((strcmp("RELATIONCAT",newName)&&strcmp("ATTRIBUTECAT",newName))==0){
    return E_NOTPERMITTED;    
 }

    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute newRelationName; // set newRelationName with newName
    strcpy(newRelationName.sVal, newName);

    // search the relation catalog for an entry with "RelName" = newRelationName
    // TODO : Get this dynamically
    char def_RELCAT_ATTR_RELNAME[16] = "RelName";
    RecId recId = linearSearch(RELCAT_RELID, def_RELCAT_ATTR_RELNAME, newRelationName, EQ);

    // If relation with name newName already exists (result of linearSearch
    //                                               is not {-1, -1})
    //    return E_RELEXIST;

    if (recId.block != -1 && recId.slot != -1)
    {
        return E_RELEXIST;
    }

    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute oldRelationName; // set oldRelationName with oldName

    strcpy(oldRelationName.sVal, oldName);
    // search the relation catalog for an entry with "RelName" = oldRelationName
    recId = linearSearch(RELCAT_RELID, def_RELCAT_ATTR_RELNAME, oldRelationName, EQ);

    // If relation with name oldName does not exist (result of linearSearch is {-1, -1})
    //    return E_RELNOTEXIST;
    if (recId.block == -1 && recId.slot == -1)
    {
        return E_RELNOTEXIST;
    }

    /* get the relation catalog record of the relation to rename using a RecBuffer
       on the relation catalog [RELCAT_BLOCK] and RecBuffer.getRecord function
    */
    RecBuffer recBuffer(RELCAT_BLOCK);
    HeadInfo head;
    recBuffer.getHeader(&head);
    Attribute record[head.numAttrs];
    recBuffer.getRecord(record, recId.slot);

    /* update the relation name attribute in the record with newName.
       (use RELCAT_REL_NAME_INDEX) */
    // set back the record value using RecBuffer.setRecord
    strcpy(record[RELCAT_REL_NAME_INDEX].sVal, newName);
    recBuffer.setRecord(record, recId.slot);

    /*
    update all the attribute catalog entries in the attribute catalog corresponding
    to the relation with relation name oldName to the relation name newName
    */

    /* reset the searchIndex of the attribute catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

    // for i = 0 to numberOfAttributes :
    //     linearSearch on the attribute catalog for relName = oldRelationName
    //     get the record using RecBuffer.getRecord
    //
    //     update the relName field in the record to newName
    //     set back the record using RecBuffer.setRecord
    // TODO : Get this dynamically
    char def_ATTRCAT_ATTR_RELNAME[16] = "RelName";
    int numAttrs = record[RELCAT_NO_ATTRIBUTES_INDEX].nVal;
    for (int i = 0; i < numAttrs; i++)
    {
        recId = linearSearch(ATTRCAT_RELID, def_ATTRCAT_ATTR_RELNAME, oldRelationName, EQ);
        RecBuffer recBuffer(recId.block);
        recBuffer.getRecord(record, recId.slot);
        strcpy(record[ATTRCAT_REL_NAME_INDEX].sVal, newName);
        recBuffer.setRecord(record, recId.slot);
    }

    return SUCCESS;
}

int BlockAccess::renameAttribute(char relName[ATTR_SIZE], char oldName[ATTR_SIZE], char newName[ATTR_SIZE]) {

    if((strcmp("RELATIONCAT",relName)&&strcmp("ATTRIBUTECAT",relName))==0){
    return E_NOTPERMITTED;    
 }


    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
RelCacheTable::resetSearchIndex(RELCAT_RELID);
    Attribute relNameAttr;    // set relNameAttr to relName
    // Search for the relation with name relName in relation catalog using linearSearch()
    // If relation with name relName does not exist (search returns {-1,-1})
    //    return E_RELNOTEXIST;
strcpy((char *)relNameAttr.sVal, relName);
   // set relNameAttr to relName 
char def_ATTRCAT_ATTR_RELNAME[16]="RelName";
RecId recId=linearSearch(RELCAT_RELID,def_ATTRCAT_ATTR_RELNAME,relNameAttr,EQ);
if(recId.block==-1&&recId.slot==-1){
return E_RELNOTEXIST;
}

    /* reset the searchIndex of the attribute catalog using
       RelCacheTable::resetSearchIndex() */

    /* declare variable attrToRenameRecId used to store the attr-cat recId
    of the attribute to rename */
    RecId attrToRenameRecId{-1, -1};
    Attribute attrCatEntryRecord[ATTRCAT_NO_ATTRS];

    /* iterate over all Attribute Catalog Entry record corresponding to the
       relation to find the required attribute */
       RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    while (true) {
        // linear search on the attribute catalog for RelName = relNameAttr
RecId recId=linearSearch(ATTRCAT_RELID,def_ATTRCAT_ATTR_RELNAME,relNameAttr,EQ);
if(recId.block==-1||recId.slot==-1){
    break;
    return E_ATTRNOTEXIST;  
}
RecBuffer recBuffer(recId.block);
recBuffer.getRecord(attrCatEntryRecord,recId.slot);

        // if attrCatEntryRecord.attrName = newName
        //     return E_ATTREXIST;  
if(strcmp((char*)attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,oldName)==0){
    

strcpy((char*)attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,newName);
recBuffer.setRecord(attrCatEntryRecord,recId.slot);
break;
}

else if(strcmp((char*)attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,newName)==0){
    

return E_ATTREXIST;
}
        // if there are no more attributes left to check (linearSearch returned {-1,-1})
        //     break;

        /* Get the record from the attribute catalog using RecBuffer.getRecord
          into attrCatEntryRecord */

        // if attrCatEntryRecord.attrName = oldName
        //     attrToRenameRecId = block and slot of this record

        // if attrCatEntryRecord.attrName = newName
        //     return E_ATTREXIST;
    }

    // if attrToRenameRecId == {-1, -1}
    //     return E_ATTRNOTEXIST;


    // Update the entry corresponding to the attribute in the Attribute Catalog Relation.
    /*   declare a RecBuffer for attrToRenameRecId.block and get the record at
         attrToRenameRecId.slot */
    //   update the AttrName of the record with newName
    //   set back the record with RecBuffer.setRecord

    return SUCCESS;
}









int BlockAccess::insert(int relId, Attribute *record)
{
    RelCatEntry relCatBuf;
    int ret = RelCacheTable::getRelCatEntry(relId, &relCatBuf);

    if (ret != SUCCESS)
        return ret;

    int blockNum = relCatBuf.firstBlk;

    RecId recId = {-1, -1};

    int numSlots = relCatBuf.numSlotsPerBlk;
    int numAttrs = relCatBuf.numAttrs;

    int prevBlockNum = -1;

    while (blockNum != -1)
    {
        RecBuffer currentBlock(blockNum);

        HeadInfo currentHeader;
        currentBlock.getHeader(&currentHeader);

        unsigned char slotMap[numSlots];
        currentBlock.getSlotMap(slotMap);

        int freeSlot = -1;
        for (int i = 0; i < numSlots; i++)
        {
            if (slotMap[i] == SLOT_UNOCCUPIED)
            {
                freeSlot = i;
                break;
            }
        }

        if (freeSlot != -1)
        {
            recId.block = blockNum;
            recId.slot = freeSlot;
            break;
        }

        prevBlockNum = blockNum;
        blockNum = currentHeader.rblock;
    }

    if (recId.block == -1 || recId.slot == -1)
    {
        if (relId == RELCAT_RELID)
            return E_MAXRELATIONS;

        RecBuffer newBlock;

        int newBlockNum = newBlock.getBlockNum();

        if (newBlockNum == E_DISKFULL)
            return E_DISKFULL;

        recId.block = newBlockNum;
        recId.slot = 0;

        HeadInfo newBlockHeader;
        newBlock.getHeader(&newBlockHeader);
        newBlockHeader.lblock = prevBlockNum;
        newBlockHeader.numAttrs = numAttrs;
        newBlockHeader.numSlots = numSlots;
        newBlock.setHeader(&newBlockHeader);

        unsigned char newBlockSlotMap[numSlots];
        newBlock.getSlotMap(newBlockSlotMap);
        for (int i = 0; i < numSlots; i++)
            newBlockSlotMap[i] = SLOT_UNOCCUPIED;
        newBlock.setSlotMap(newBlockSlotMap);

        if (prevBlockNum != -1)
        {
            RecBuffer prevBlock(prevBlockNum);

            HeadInfo prevBlockHeader;
            prevBlock.getHeader(&prevBlockHeader);
            prevBlockHeader.rblock = recId.block;
            prevBlock.setHeader(&prevBlockHeader);
        }
        else
        {
            relCatBuf.firstBlk = recId.block;
            relCatBuf.lastBlk = recId.block;
            RelCacheTable::setRelCatEntry(relId, &relCatBuf);
        }
    }

    RecBuffer blockToInsert(recId.block);
    blockToInsert.setRecord(record, recId.slot);

    unsigned char slotMapToInsert[numSlots];
    blockToInsert.getSlotMap(slotMapToInsert);
    slotMapToInsert[recId.slot] = SLOT_OCCUPIED;
    blockToInsert.setSlotMap(slotMapToInsert);

    HeadInfo headerToInsert;
    blockToInsert.getHeader(&headerToInsert);
    headerToInsert.numEntries++;
    blockToInsert.setHeader(&headerToInsert);

    relCatBuf.numRecs++;
    RelCacheTable::setRelCatEntry(relId, &relCatBuf);

    int flag = SUCCESS;
    for (int attrOffset = 0; attrOffset < numAttrs; attrOffset++)
    {
        AttrCatEntry attrCatBuf;
        AttrCacheTable::getAttrCatEntry(relId, attrOffset, &attrCatBuf);
        if (attrCatBuf.rootBlock == -1)
            continue;

        int ret = BPlusTree::bPlusInsert(relId, attrCatBuf.attrName, record[attrOffset], recId);
        if (ret == E_DISKFULL)
            flag = E_INDEX_BLOCKS_RELEASED;
    }

    return flag;
}


int BlockAccess::search(int relId, Attribute *record, char attrName[ATTR_SIZE],
                        Attribute attrVal, int op) {
  // Declare a variable called recid to store the searched record
  RecId recId;
  AttrCatEntry attrCatEntry;    
  int ret = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);
  if(ret != SUCCESS){
      return ret;
  } 
  int rb = attrCatEntry.rootBlock; // just to avoid unused variable warning. This will be used in later stages when we implement indexing.
  
  if(rb==-1){
     
    recId = BlockAccess::linearSearch(relId, attrName, attrVal, op);
 
}

  else {
   
    recId = BPlusTree::bPlusSearch(relId, attrName, attrVal, op);   
 
  }
  /* search for the record id (recid) corresponding to the attribute with
  attribute name attrName, with value attrval and satisfying the condition op
  using linearSearch() */
  
  if (recId.block == -1 and recId.slot == -1)
    return E_NOTFOUND;

  // if there's no record satisfying the given condition (recId = {-1, -1})
  //    return E_NOTFOUND;
  RecBuffer recBuffer(recId.block);
   ret = recBuffer.getRecord(record, recId.slot);
  if (ret != SUCCESS)
    return ret;
  /* Copy the record with record id (recId) to the record buffer (record)
     For this Instantiate a RecBuffer class object using recId and
     call the appropriate method to fetch the record
  */

  return SUCCESS;
}






int BlockAccess::deleteRelation(char relName[ATTR_SIZE])
{
    if (
        strcmp(relName, (char *)RELCAT_RELNAME) == 0 ||
        strcmp(relName, (char *)ATTRCAT_RELNAME) == 0)
    {
        return E_NOTPERMITTED;
    }

    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute relNameAttribute;
    strcpy(relNameAttribute.sVal, relName);

    RecId recId = linearSearch(RELCAT_RELID, (char *)RELCAT_ATTR_RELNAME, relNameAttribute, EQ);

    if (recId.block == -1 || recId.slot == -1)
        return E_RELNOTEXIST;

    Attribute relCatEntryRecord[RELCAT_NO_ATTRS];

    RecBuffer recBuffer(recId.block);
    recBuffer.getRecord(relCatEntryRecord, recId.slot);

    int currentBlock = relCatEntryRecord[RELCAT_FIRST_BLOCK_INDEX].nVal;

    while (currentBlock != -1)
    {
        RecBuffer currentBlockBuffer(currentBlock);
        HeadInfo currentBlockHeader;
        currentBlockBuffer.getHeader(&currentBlockHeader);

        int nextBlock = currentBlockHeader.rblock;

        currentBlockBuffer.releaseBlock();
        currentBlock = nextBlock;
    }

    int numAttrsDeleted = 0;
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    while (true)
    {
        RecId attrCatRecId = linearSearch(ATTRCAT_RELID, (char *)ATTRCAT_ATTR_RELNAME, relNameAttribute, EQ);

        if (attrCatRecId.slot == -1 || attrCatRecId.block == -1)
            break;

        numAttrsDeleted++;

        RecBuffer currentBlock(attrCatRecId.block);

        HeadInfo currentBlockHeader;
        currentBlock.getHeader(&currentBlockHeader);

        Attribute record[ATTRCAT_NO_ATTRS];
        currentBlock.getRecord(record, attrCatRecId.slot);

        int rootBlock = record[ATTRCAT_ROOT_BLOCK_INDEX].nVal;

        unsigned char slotMap[currentBlockHeader.numSlots];

        currentBlock.getSlotMap(slotMap);
        slotMap[attrCatRecId.slot] = SLOT_UNOCCUPIED;
        currentBlock.setSlotMap(slotMap);

        currentBlockHeader.numEntries--;
        currentBlock.setHeader(&currentBlockHeader);

        if (currentBlockHeader.numEntries == 0)
        {
            int leftBlock = currentBlockHeader.lblock;
            int rightBlock = currentBlockHeader.rblock;

            if (leftBlock != -1)
            {
                RecBuffer prevBlock(leftBlock);
                HeadInfo prevBlockHeader;

                prevBlock.getHeader(&prevBlockHeader);
                prevBlockHeader.rblock = rightBlock;
                prevBlock.setHeader(&prevBlockHeader);
            }

            if (rightBlock != -1)
            {
                RecBuffer nextBlock(rightBlock);
                HeadInfo nextBlockHeader;

                nextBlock.getHeader(&nextBlockHeader);
                nextBlockHeader.lblock = leftBlock;
                nextBlock.setHeader(&nextBlockHeader);
            }

            currentBlock.releaseBlock();
        }

        // condition to handle b+ trees
        if (rootBlock != -1)
            BPlusTree::bPlusDestroy(rootBlock);
    }

    HeadInfo relCatHeader;
    recBuffer.getHeader(&relCatHeader);

    unsigned char recSlotMap[relCatHeader.numSlots];

    recBuffer.getSlotMap(recSlotMap);
    recSlotMap[recId.slot] = SLOT_UNOCCUPIED;
    recBuffer.setSlotMap(recSlotMap);

    relCatHeader.numEntries--;
    recBuffer.setHeader(&relCatHeader);

    RelCatEntry relCatBuf;
    RelCacheTable::getRelCatEntry(RELCAT_RELID, &relCatBuf);
    relCatBuf.numRecs--;
    RelCacheTable::setRelCatEntry(RELCAT_RELID, &relCatBuf);

    RelCatEntry attrCatBuf;
    RelCacheTable::getRelCatEntry(ATTRCAT_RELID, &attrCatBuf);
    attrCatBuf.numRecs -= numAttrsDeleted;
    RelCacheTable::setRelCatEntry(ATTRCAT_RELID, &attrCatBuf);

    return SUCCESS;
}



int BlockAccess::project(int relId, Attribute *record) {
    // get the previous search index of the relation relId from the relation
    // cache (use RelCacheTable::getSearchIndex() function)
    RecId prevRecId;
    RelCacheTable::getSearchIndex(relId, &prevRecId);

    // declare block and slot which will be used to store the record id of the
    // slot we need to check.
    int block, slot;

    /* if the current search index record is invalid(i.e. = {-1, -1})
       (this only happens when the caller reset the search index)
    */
    if (prevRecId.block == -1 && prevRecId.slot == -1)
    {
        // (new project operation. start from beginning)
        RelCatEntry relCatEntry;
        
        // get the first record block of the relation from the relation cache
        // (use RelCacheTable::getRelCatEntry() function of Cache Layer)
        RelCacheTable::getRelCatEntry(relId, &relCatEntry);

        // block = first record block of the relation
        block = relCatEntry.firstBlk;
        // slot = 0
        slot = 0;
    }
    else
    {
        // (a project/search operation is already in progress)

        // block = previous search index's block
        block = prevRecId.block;
        // slot = previous search index's slot + 1
        slot = prevRecId.slot + 1;
    }


    // The following code finds the next record of the relation
    /* Start from the record id (block, slot) and iterate over the remaining
       records of the relation */
    while (block != -1)
    {
        // create a RecBuffer object for block (using appropriate constructor!)
        RecBuffer recordBlockBuffer(block);

        // get header of the block using RecBuffer::getHeader() function
        HeadInfo head;
        recordBlockBuffer.getHeader(&head);
        // get slot map of the block using RecBuffer::getSlotMap() function
        unsigned char slotMap[head.numSlots];
        recordBlockBuffer.getSlotMap(slotMap);

        if(slot >= head.numSlots) // slot >= the number of slots per block
        {
            // (no more slots in this block)
            // update block = right block of block
            // update slot = 0
            block = head.rblock;
            slot = 0;
            // (NOTE: if this is the last block, rblock would be -1. this would
            //        set block = -1 and fail the loop condition )
        }
        else if (slotMap[slot] == SLOT_UNOCCUPIED) // slot is free
        { // (i.e slot-th entry in slotMap contains SLOT_UNOCCUPIED)

            // increment slot
            slot++;
        }
        else {
            // (the next occupied slot / record has been found)
            break;
        }
    }

    if (block == -1){
        // (a record was not found. all records exhausted)
        return E_NOTFOUND;
    }

    // declare nextRecId to store the RecId of the record found
    RecId nextRecId{block, slot};

    // set the search index to nextRecId using RelCacheTable::setSearchIndex
    RelCacheTable::setSearchIndex(relId, &nextRecId);

    /* Copy the record with record id (nextRecId) to the record buffer (record)
       For this Instantiate a RecBuffer class object by passing the recId and
       call the appropriate method to fetch the record
    */
    RecBuffer recBuffer(nextRecId.block);
    int ret = recBuffer.getRecord(record, nextRecId.slot);
    if (ret != SUCCESS) {
        return ret;
    }

    return SUCCESS;
}


