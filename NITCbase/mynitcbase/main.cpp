#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include "bits/stdc++.h"
int main(int argc, char *argv[]) {
  Disk disk_run;

  // create objects for the relation catalog and attribute catalog
  RecBuffer relCatBuffer(RELCAT_BLOCK);
  RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

  HeadInfo relCatHeader;
  HeadInfo attrCatHeader;

  // load the headers of both the blocks into relCatHeader and attrCatHeader.
  // (we will implement these functions later)
  relCatBuffer.getHeader(&relCatHeader);
  attrCatBuffer.getHeader(&attrCatHeader);
 std::vector<int> attrBlocks;
  attrBlocks.push_back(ATTRCAT_BLOCK);


 while(attrCatHeader.rblock!=-1){
    attrBlocks.push_back((attrCatHeader.rblock));
    RecBuffer temp(attrCatHeader.rblock);
    temp.getHeader(&attrCatHeader);
  }



int numOfRelations = relCatHeader.numEntries;
int numOfAttributes = attrCatHeader.numEntries;
  for (int i=0;i<numOfRelations;i++) {

    Attribute relCatRecord[RELCAT_NO_ATTRS]; // will store the record from the relation catalog

    relCatBuffer.getRecord(relCatRecord, i);

    printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

    for(int b=0;b<attrBlocks.size();b++){  
    RecBuffer attrCatBuffer(attrBlocks[b]);
    HeadInfo attrCatHeader;
    attrCatBuffer.getHeader(&attrCatHeader);  

    for (int j=0;j<attrCatHeader.numEntries;j++/* j = 0 to number of entries in the attribute catalog */) {
	    Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
      // declare attrCatRecord and load the attribute catalog entry into it
		attrCatBuffer.getRecord(attrCatRecord,j);
      if (strcmp(attrCatRecord[0].sVal,relCatRecord[0].sVal)==0/* attribute catalog entry corresponds to the current relation */) {
        const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
        printf("  %s: %s\n", /* get the attribute name */attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
      }
    }
    printf("\n");
  }
}

  return 0;
}
