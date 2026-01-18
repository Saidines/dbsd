#include "BlockBuffer.h"
#include <cstdlib>
#include <cstring>
// the declarations for these functions can be found in "BlockBuffer.h"

// Compare two attributes based on their type
// if attr1 < attr2 return -1
// if equal return 0
// else return 1
int compareAttrs(Attribute attr1, Attribute attr2, int attrType) {
  if (attrType == STRING) {
    return strcmp(attr1.sVal, attr2.sVal);
  }
  
  if (attrType == NUMBER) {
    if (attr1.nVal < attr2.nVal)
      return -1;
    else if (attr1.nVal == attr2.nVal)
      return 0;
    else
      return 1;
  }
  return 0;
}

BlockBuffer::BlockBuffer(int blockNum) {
  // initialise this.blockNum with the argument
    this->blockNum = blockNum;
}

// calls the parent class constructor
RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {}

// load the block header into the argument pointer
int BlockBuffer::getHeader(struct HeadInfo *head) {
/*  unsigned char buffer[BLOCK_SIZE];
    Disk::readBlock(buffer ,this->blockNum);*/
  // read the block at this.blockNum into the buffer

  // populate the numEntries, numAttrs and numSlots fields in *head

	unsigned char *bufferPtr;
	int ret = loadBlockAndGetBufferPtr(&bufferPtr);
	if(ret!=SUCCESS){
		return ret;
	}

  memcpy(&head->numSlots, bufferPtr + 24, 4);
  memcpy(&head->numEntries, bufferPtr+16, 4);
  memcpy(&head->numAttrs,bufferPtr+20, 4);
  memcpy(&head->rblock, bufferPtr+12, 4);
  memcpy(&head->lblock, bufferPtr+8, 4);
  memcpy(&head->pblock, bufferPtr+4,4);
  memcpy(&head->blockType, bufferPtr,4);

  return SUCCESS;
}

// load the record at slotNum into the argument pointer
int RecBuffer::getRecord(union Attribute *rec, int slotNum) {
  struct HeadInfo head;

  // get the header using this.getHeader() function
    this->getHeader(&head);
  int attrCount = head.numAttrs;
  int slotCount = head.numSlots;

  /*
  unsigned char buffer[BLOCK_SIZE];
    Disk::readBlock(buffer,this->blockNum );
    */
  unsigned char* bufferPtr;
	int ret = loadBlockAndGetBufferPtr(&bufferPtr);

	if(ret!=SUCCESS){
		
		return ret;
	}



  // read the block at this.blockNum into a buffer
         //   memcpy(&rec,buffer+HEADER_SIZE+head.numSlots+head.numAttrs*ATTR_SIZE*slotNum,ATTR_SIZE*head.numAttrs);
  /* record at slotNum will be at offset HEADER_SIZE + slotMapSize + (recordSize * slotNum)
     - each record will have size attrCount * ATTR_SIZE
     - slotMap will be of size slotCount
  */
  int recordSize = attrCount * ATTR_SIZE;
  unsigned char *slotPointer = bufferPtr+HEADER_SIZE+head.numSlots+head.numAttrs*ATTR_SIZE*slotNum/* calculate buffer + offset */;

  // load the record into the rec data structure
  memcpy(rec, slotPointer, recordSize);

  return SUCCESS;
}

int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **bufferPtr){

	int bufferNum = StaticBuffer::getBufferNum(this->blockNum);


	if(bufferNum == E_BLOCKNOTINBUFFER){
	
		bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);


		if(bufferNum==E_OUTOFBOUND){
			return E_OUTOFBOUND;
		}	

		Disk::readBlock(StaticBuffer::blocks[bufferNum],this->blockNum);

	}

	*bufferPtr=StaticBuffer::blocks[bufferNum];
	return SUCCESS;

}	


/* used to get the slotmap from a record block
NOTE: this function expects the caller to allocate memory for `*slotMap`
*/
int RecBuffer::getSlotMap(unsigned char *slotMap) {
  unsigned char *bufferPtr;

  // get the starting address of the buffer containing the block using loadBlockAndGetBufferPtr().
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }

  struct HeadInfo head;
  // get the header of the block using getHeader() function
  this->getHeader(&head);
  int slotCount = head.numSlots;
  unsigned char* slotMapInBuffer = bufferPtr + HEADER_SIZE;
  memcpy(slotMap, slotMapInBuffer, slotCount);
  return SUCCESS; 
 // int slotCount = /* number of slots in block from header */;

  // get a pointer to the beginning of the slotmap in memory by offsetting HEADER_SIZE
  //unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;

  // copy the values from `slotMapInBuffer` to `slotMap` (size is `slotCount`)

  //return SUCCESS;
}

