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


  int RecBuffer::setRecord(union Attribute *rec, int slotNum)
{
    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
    // return the value returned by the call.
    if (ret != SUCCESS)
    {
        return ret;
    }

    /* get the header of the block using the getHeader() function */
    struct HeadInfo head;
    this->getHeader(&head);

    // get number of attributes in the block.
    int numAttrs = head.numAttrs;

    // get the number of slots in the block.
    int numSlots = head.numSlots;

    // if input slotNum is not in the permitted range return E_OUTOFBOUND.
    if (slotNum >= numSlots || slotNum < 0)
    {
        return E_OUTOFBOUND;
    }

    /* offset bufferPtr to point to the beginning of the record at required
       slot. the block contains the header, the slotmap, followed by all
       the records. so, for example,
       record at slot x will be at bufferPtr + HEADER_SIZE + (x*recordSize)
       copy the record from `rec` to buffer using memcpy
       (hint: a record will be of size ATTR_SIZE * numAttrs)
    */
    int recordSize = numAttrs * ATTR_SIZE;
    int offset = HEADER_SIZE + numSlots + (recordSize * slotNum);
    memcpy(bufferPtr + offset, rec, recordSize);

    // update dirty bit using setDirtyBit()
    StaticBuffer::setDirtyBit(this->blockNum);

    /* (the above function call should not fail since the block is already
       in buffer and the blockNum is valid. If the call does fail, there
       exists some other issue in the code) */

    // return SUCCESS
    return SUCCESS;
}

int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr)
{
    /* check whether the block is already present in the buffer
       using StaticBuffer.getBufferNum() */
    int bufferNum = StaticBuffer::getBufferNum(this->blockNum);

    // if present (!=E_BLOCKNOTINBUFFER),
    // set the timestamp of the corresponding buffer to 0 and increment the
    // timestamps of all other occupied buffers in BufferMetaInfo.
    if (bufferNum != E_BLOCKNOTINBUFFER)
    {
        StaticBuffer::metainfo[bufferNum].timeStamp = 0;
        for (int i = 0; i < BUFFER_CAPACITY; i++)
        {
            if (StaticBuffer::metainfo[i].free == false)
            {
                StaticBuffer::metainfo[i].timeStamp++;
            }
        }
    }
    else
    {
        // if not present
        // get a free buffer using StaticBuffer.getFreeBuffer()
        // if the call returns E_OUTOFBOUND, return E_OUTOFBOUND here as
        // the blockNum is invalid
        // Read the block into the free buffer using readBlock()
        bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);
        if (bufferNum == E_OUTOFBOUND)
        {
            return E_OUTOFBOUND;
        }
        Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
    }
    *buffPtr = StaticBuffer::blocks[bufferNum];
    // return SUCCESS;
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

/*
int StaticBuffer::getFreeBuffer(int blockNum){
    // Check if blockNum is valid (non zero and less than DISK_BLOCKS)
    // and return E_OUTOFBOUND if not valid.
if(blockNum<0||blockNum>DISK_BLOCKS) return E_OUTOFBOUND;
    // increase the timeStamp in metaInfo of all occupied buffers.
int bufferNum=-1;
    for(int i=0;i<BUFFER_CAPACITY;i++){
    if(metainfo[i].free==false){
        metainfo[i].timeStamp+=1;
    }
    else bufferNum=i;
}

if(bufferNum!=-1) return bufferNum;
    // let bufferNum be used to store the buffer number of the free/freed buffer.
    bufferNum=0;
    for(int i=0;i<BUFFER_CAPACITY;i++){
        if(metainfo[i].timeStamp>metainfo[bufferNum].timeStamp ){
            bufferNum=i;
            break;
        }
    } 
    if(metainfo[bufferNum].dirty==true){
        unsigned char* bufferPtr;
      loadBlockAndGetBufferPtr(&bufferPtr);
        Disk::writeBlock(blocks[bufferNum],metainfo[bufferNum].blockNum);
        
      } 
      return bufferNum;
    // iterate through metainfo and check if there is any buffer free

    // if a free buffer is available, set bufferNum = index of that free buffer.

    // if a free buffer is not available,
    //     find the buffer with the largest timestamp
    //     IF IT IS DIRTY, write back to the disk using Disk::writeBlock()
    //     set bufferNum = index of this buffer

    // update the metaInfo entry corresponding to bufferNum with
    // free:false, dirty:false, blockNum:the input block number, timeStamp:0.

    // return the bufferNum.
}*/


int StaticBuffer::setDirtyBit(int blockNum){
    // find the buffer index corresponding to the block using getBufferNum().
    int bufferNum=StaticBuffer::getBufferNum(blockNum); 
    if(bufferNum==E_BLOCKNOTINBUFFER){
        return E_BLOCKNOTINBUFFER;
    }
    if(bufferNum==E_OUTOFBOUND){  
        return E_OUTOFBOUND;
    } 
    else{
        metainfo[bufferNum].dirty=true;
    } 
    return SUCCESS;
    // if block is not present in the buffer (bufferNum = E_BLOCKNOTINBUFFER)
    //     return E_BLOCKNOTINBUFFER

    // if blockNum is out of bound (bufferNum = E_OUTOFBOUND)
    //     return E_OUTOFBOUND

    // else
    //     (the bufferNum is valid)
    //     set the dirty bit of that buffer to true in metainfo

    // return SUCCESS
}

