#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include "bits/stdc++.h"

int s_o_b(int blockNumber){
	if(blockNumber>8192) return -1;
	int mapBlockNumber=blockNumber/2048;
	blockNumber=blockNumber%20448;
	unsigned char buffer[BLOCK_SIZE];
	Disk::readBlock(buffer,mapBlockNumber);
	return buffer[blockNumber];
}

int main(int argc, char *argv[]) {
  /* Initialize the Run Copy of Disk */
  Disk disk_run;
  // StaticBuffer buffer;
  unsigned char buffer1[BLOCK_SIZE];
  Disk::readBlock(buffer1,0);
  char message[BLOCK_SIZE];
  std::cout<<(char*)buffer1<<std::endl;
    for (int i = 0; i < 2048; i++) {
	    std::cout << (int)buffer1[i] << " ";
        }
  Disk::writeBlock(buffer1,7000);
  unsigned char buffer2[BLOCK_SIZE];
  char message2[BLOCK_SIZE];
  Disk::readBlock(buffer2,7000);
  memcpy(message2,buffer2+20,6);
  std::cout<<message2<<std::endl<<(char*)buffer2;
  // OpenRelTable cache;

  return FrontendInterface::handleFrontend(argc, argv);
}
