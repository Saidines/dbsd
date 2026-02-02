#include "StaticBuffer.h"
	unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
	 struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];
	unsigned char StaticBuffer::blockAllocMap[DISK_BLOCKS];



	
int StaticBuffer::getFreeBuffer(int blockNum)
{
    // Check if blockNum is valid (non zero and less than DISK_BLOCKS)
	    // and return E_OUTOFBOUND if not valid.
    if (blockNum < 0 || blockNum >= DISK_BLOCKS)
    {
        return E_OUTOFBOUND;
    }

    // increase the timeStamp in metaInfo of all occupied buffers.
    for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++)
    {
        if (!metainfo[bufferIndex].free)
        {
            metainfo[bufferIndex].timeStamp++;
        }
    }

    // let bufferNum be used to store the buffer number of the free/freed buffer.
    int bufferNum;
    bool bufferFound = false;
    // iterate through metainfo and check if there is any buffer free
    for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++)
    {
        if (metainfo[bufferIndex].free)
        {
            bufferNum = bufferIndex;
            bufferFound = true;
            break;
        }
    }

    // if a free buffer is available, set bufferNum = index of that free buffer.

    // if a free buffer is not available,
    //     find the buffer with the largest timestamp
    //     IF IT IS DIRTY, write back to the disk using Disk::writeBlock()
    //     set bufferNum = index of this buffer

    if (!bufferFound)
    {
        int maxTimeStamp = -1;
        for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++)
        {
            if (metainfo[bufferIndex].timeStamp > maxTimeStamp)
            {
                maxTimeStamp = metainfo[bufferIndex].timeStamp;
                bufferNum = bufferIndex;
            }
        }
        if (metainfo[bufferNum].dirty)
        {
            Disk::writeBlock(blocks[bufferNum], metainfo[bufferNum].blockNum);
        }
    }

    // update the metaInfo entry corresponding to bufferNum with
    // free:false, dirty:false, blockNum:the input block number, timeStamp:0.
    metainfo[bufferNum].free = false;
    metainfo[bufferNum].dirty = false;
    metainfo[bufferNum].blockNum = blockNum;
    metainfo[bufferNum].timeStamp = 0; // or -1

    return bufferNum;

    // return the bufferNum.
}

	int StaticBuffer::getBufferNum(int blockNum){
		
		if(blockNum<0||blockNum>DISK_BLOCKS) return E_OUTOFBOUND;

		for(int i=0;i<BUFFER_CAPACITY;i++){
			if(metainfo[i].blockNum==blockNum) return i;

		}
		return E_BLOCKNOTINBUFFER;

	}


	StaticBuffer::StaticBuffer() {
int blockmapslot=0;
	for(int i=0;i<4;i++){
		unsigned char buffer[BLOCK_SIZE];
		Disk::readBlock(buffer,i);
		for(int slot=0;slot<BLOCK_SIZE;slot++){
			StaticBuffer::blockAllocMap[blockMapSlot] = buffer[slot];

		}

	}

  for (int bufferIndex = 0;bufferIndex<=BUFFER_CAPACITY-1;bufferIndex++) {
    // set metainfo[bufferindex] with the following values
    StaticBuffer::metainfo[bufferIndex].free = true;
   StaticBuffer::metainfo[bufferIndex].dirty = false;
    StaticBuffer::metainfo[bufferIndex].timeStamp = -1;
   StaticBuffer::metainfo[bufferIndex].blockNum = -1;
  }
}

// write back all modified blocks on system exit
StaticBuffer::~StaticBuffer() {
  /*iterate through all the buffer blocks,
    write back blocks with metainfo as free=false,dirty=true
    using Disk::writeBlock()

    */
    
    int blockmapslot=0;
	for(int i=0;i<4;i++){
		unsigned char buffer[BLOCK_SIZE];
		
		for(int slot=0;slot<BLOCK_SIZE;slot++){
	 buffer[slot] =	StaticBuffer::blockAllocMap[blockMapSlot] ;

		}
		Disk::writeBlock(buffer,i);

	}

	for(int i=0;i<BUFFER_CAPACITY;i++){
		if(metainfo[i].free==false&&metainfo[i].dirty==true){
			Disk::writeBlock(blocks[i],metainfo[i].blockNum);
		}

	}
}
