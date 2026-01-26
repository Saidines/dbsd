#include "StaticBuffer.h"
	unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
	 struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];




	
	int	StaticBuffer::getFreeBuffer(int blockNum){
		if(blockNum<0||blockNum>DISK_BLOCKS) return E_OUTOFBOUND;
			int allocatedBuffer=E_OUTOFBOUND;
		for(int i=0;i<BUFFER_CAPACITY;i++){
			if(metainfo[i].free==true){
				allocatedBuffer=i;
				break;
		}
		}

		if(allocatedBuffer!=E_OUTOFBOUND){
			metainfo[allocatedBuffer].free=false;
			metainfo[allocatedBuffer].blockNum=blockNum;
		}
					
		return allocatedBuffer;
	}

	int StaticBuffer::getBufferNum(int blockNum){
		
		if(blockNum<0||blockNum>DISK_BLOCKS) return E_OUTOFBOUND;

		for(int i=0;i<BUFFER_CAPACITY;i++){
			if(metainfo[i].blockNum==blockNum) return i;

		}
		return E_BLOCKNOTINBUFFER;

	}


	StaticBuffer::StaticBuffer() {

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

	for(int i=0;i<BUFFER_CAPACITY;i++){
		if(metainfo[i].free==false&&metainfo[i].dirty==true){
			Disk::writeBlock(blocks[i],metainfo[i].blockNum);
		}

	}
}