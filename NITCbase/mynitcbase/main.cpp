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
  Disk disk_run;
  StaticBuffer static_buffer;
  OpenRelTable cache;
  // create objects for the relation catalog and attribute catalog
/*  RecBuffer relCatBuffer(RELCAT_BLOCK);
  RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

  HeadInfo relCatHeader;
  HeadInfo attrCatHeader;

  // load the headers of both the blocks into relCatHeader and attrCatHeader.
  // (we will implement these functions later)
  relCatBuffer.getHeader(&relCatHeader);
  attrCatBuffer.getHeader(&attrCatHeader);
int numOfRelations = relCatHeader.numEntries;
int numOfAttributes = attrCatHeader.numEntries;

  for (int i=0;i<numOfRelations;i++) {

    Attribute relCatRecord[RELCAT_NO_ATTRS]; // will store the record from the relation catalog

    relCatBuffer.getRecord(relCatRecord, i);

    printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);
*/
  //  for (int j=0;j<numOfAttributes;j++/* j = 0 to number of entries in the attribute catalog */) {
//	    Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
      // declare attrCatRecord and load the attribute catalog entry into it
//		attrCatBuffer.getRecord(attrCatRecord,j);
  //    if (strcmp(attrCatRecord[0].sVal,relCatRecord[0].sVal)==0/* attribute catalog entry corresponds to the current relation */) {
    //    const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
      //  printf("  %s: %s\n", /* get the attribute name */attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
      /*}
    }
    printf("\n");
  }*/

  for(int i=0;i<=2;i++){
  	RelCatEntry relCatBuf;
	RelCacheTable::getRelCatEntry(i,&relCatBuf);
	char* relname = relCatBuf.relName;
	printf("Relation: %s\n",relname);
	
	for(int j=0;j < relCatBuf.numAttrs;j++){
		AttrCatEntry AttrCatBuf;
		AttrCacheTable::getAttrCatEntry(i,j,&AttrCatBuf);
		char attributeName[ATTR_SIZE];
		strcpy(attributeName,AttrCatBuf.attrName);
	      const char* attributeType=(AttrCatBuf.attrType == 0)? "NUM" : "STRING"; 
		printf(" %s: %s\n",attributeName,attributeType);
		
		
	}



  }

  return 0;
}
