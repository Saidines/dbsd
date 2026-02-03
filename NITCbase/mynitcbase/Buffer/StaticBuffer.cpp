// the declarations for this class can be found at "StaticBuffer.h"
#include "StaticBuffer.h"

unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];
// declare the blockAllocMap array
unsigned char StaticBuffer::blockAllocMap[DISK_BLOCKS];

StaticBuffer::StaticBuffer() {
    // copy blockAllocMap blocks from disk to buffer (using readblock() of disk)
    // blocks 0 to 3
    for(int i = 0, blockAllocMapSlot = 0; i < 4; i++){
        unsigned char buffer[BLOCK_SIZE];
        Disk::readBlock(buffer, i);
        for(int slot = 0; slot < BLOCK_SIZE; slot++, blockAllocMapSlot++){
            StaticBuffer::blockAllocMap[blockAllocMapSlot] = buffer[slot];
        }
    }
    
    // initialise all blocks as free
    for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
        // set metainfo[bufferindex] with the following values
        //   free = true
        metainfo[bufferIndex].free = true;
        //   dirty = false
        metainfo[bufferIndex].dirty = false;
        //   timeStamp = -1
        metainfo[bufferIndex].timeStamp = -1;
        //   blockNum = -1
        metainfo[bufferIndex].blockNum = -1;
    }
}

/*
At this stage, we are not writing back from the buffer to the disk since we are
not modifying the buffer. So, we will define an empty destructor for now. In
subsequent stages, we will implement the write-back functionality here.
*/
// write back all modified blocks on system exit
StaticBuffer::~StaticBuffer() {
    // copy blockAllocMap blocks from buffer to disk(using writeblock() of disk)
    for(int i = 0, blockAllocMapSlot = 0; i < 4; i++){
        unsigned char buffer[BLOCK_SIZE];
        for(int slot = 0; slot < BLOCK_SIZE; slot++, blockAllocMapSlot++){
            buffer[slot] = StaticBuffer::blockAllocMap[blockAllocMapSlot];
        }
        Disk::writeBlock(buffer, i);
    }


  /*iterate through all the buffer blocks,
    write back blocks with metainfo as free=false,dirty=true
    using Disk::writeBlock()
    */
   for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
       if(!metainfo[bufferIndex].free && metainfo[bufferIndex].dirty) {
           Disk::writeBlock(blocks[bufferIndex],metainfo[bufferIndex].blockNum);
       }
   }
}

int StaticBuffer::getFreeBuffer(int blockNum){
    // Check if blockNum is valid (non zero and less than DISK_BLOCKS)
    // and return E_OUTOFBOUND if not valid.
    if(blockNum < 0 || blockNum > DISK_BLOCKS) {
        return E_OUTOFBOUND;
    }

    // increase the timeStamp in metaInfo of all occupied buffers.
    for(int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
        if(!metainfo[bufferIndex].free) {
            metainfo[bufferIndex].timeStamp += 1;
        }
    }

    // let bufferNum be used to store the buffer number of the free/freed buffer.
    int bufferNum = -1;

    // iterate through metainfo and check if there is any buffer free
    for(int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
        if(metainfo[bufferIndex].free) {
            bufferNum = bufferIndex;
            break;
        }
    }

    // if a free buffer is available, set bufferNum = index of that free buffer.

    // if a free buffer is not available,
    //     find the buffer with the largest timestamp
    //     IF IT IS DIRTY, write back to the disk using Disk::writeBlock()
    //     set bufferNum = index of this buffer
    if(bufferNum == -1) {
        int maxTimeStamp = -1;
        for(int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
            if(metainfo[bufferIndex].timeStamp > maxTimeStamp) {
                maxTimeStamp = metainfo[bufferIndex].timeStamp;
                bufferNum = bufferIndex;
            }
        }
        // if dirty, write back to disk
        if(metainfo[bufferNum].dirty) {
            Disk::writeBlock(blocks[bufferNum], metainfo[bufferNum].blockNum);
        }
    }

    // update the metaInfo entry corresponding to bufferNum with
    // free:false, dirty:false, blockNum:the input block number, timeStamp:0.
    metainfo[bufferNum].free = false;
    metainfo[bufferNum].dirty = false;
    metainfo[bufferNum].blockNum = blockNum;
    metainfo[bufferNum].timeStamp = 0;

    // return the bufferNum.
    return bufferNum;
}

/* Get the buffer index where a particular block is stored
   or E_BLOCKNOTINBUFFER otherwise
*/

int StaticBuffer::getBufferNum(int blockNum) {
    // Check if blockNum is valid (between zero and DISK_BLOCKS)
    // and return E_OUTOFBOUND if not valid.
    if(blockNum < 0 || blockNum > DISK_BLOCKS) {
        return E_OUTOFBOUND;
    }


    // find and return the bufferIndex which corresponds to blockNum (check metainfo)
    for(int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
        if(metainfo[bufferIndex].blockNum == blockNum) {
        return bufferIndex;
        }
    }
    // if block is not in the buffer
    return E_BLOCKNOTINBUFFER;
}

int StaticBuffer::setDirtyBit(int blockNum){
    // find the buffer index corresponding to the block using getBufferNum().
    int bufferNum = getBufferNum(blockNum);

    // if block is not present in the buffer (bufferNum = E_BLOCKNOTINBUFFER)
    //     return E_BLOCKNOTINBUFFER
    if(bufferNum == E_BLOCKNOTINBUFFER) {
        return E_BLOCKNOTINBUFFER;
    }

    // if blockNum is out of bound (bufferNum = E_OUTOFBOUND)
    //     return E_OUTOFBOUND
    if(bufferNum == E_OUTOFBOUND) {
        return E_OUTOFBOUND;
    }
    // else
    //     (the bufferNum is valid)
    //     set the dirty bit of that buffer to true in metainfo
    metainfo[bufferNum].dirty = true;
    // return SUCCESS
    return SUCCESS;
}