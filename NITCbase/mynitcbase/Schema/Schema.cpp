#include "Schema.h"

#include <cmath>
#include <cstring>
#include <cstdio>
int Schema::openRel(char relName[ATTR_SIZE]) {
  int ret = OpenRelTable::openRel(relName);

  // the OpenRelTable::openRel() function returns the rel-id if successful
  // a valid rel-id will be within the range 0 <= relId < MAX_OPEN and any
  // error codes will be negative

  if(ret >= 0&&ret<=MAX_OPEN){
    return SUCCESS;
  }

  //otherwise it returns an error message
  return ret;
}

int Schema::closeRel(char relName[ATTR_SIZE]) {
  if ((strcmp(relName,RELCAT_RELNAME)&&strcmp(relName,ATTRCAT_RELNAME))==0) {
    return E_NOTPERMITTED;
  }

  // this function returns the rel-id of a relation if it is open or
  // E_RELNOTOPEN if it is not. we will implement this later.
  int relId = OpenRelTable::getRelId(relName);

  if (relId < 0 || relId>=MAX_OPEN /* relation is not open */) {
    return E_RELNOTOPEN;
  }

  return OpenRelTable::closeRel(relId);
}

int Schema::renameRel(char oldRelName[ATTR_SIZE], char newRelName[ATTR_SIZE]) {
    // if the oldRelName or newRelName is either Relation Catalog or Attribute Catalog,
    if( (strcmp(oldRelName,RELCAT_RELNAME)==0)||(strcmp(oldRelName,ATTRCAT_RELNAME)==0)) return E_NOTPERMITTED; 
        // return E_NOTPERMITTED

        // (check if the relation names are either "RELATIONCAT" and "ATTRIBUTECAT".
        // you may use the following constants: RELCAT_RELNAME and ATTRCAT_RELNAME)
int ret=OpenRelTable::getRelId(oldRelName);
  if(ret!=E_RELNOTOPEN) return E_RELOPEN;
    // if the relation is open
    //    (check if OpenRelTable::getRelId() returns E_RELNOTOPEN)
    //    return E_RELOPEN
ret= BlockAccess::renameRelation(oldRelName,newRelName);
return ret;
    // retVal = BlockAccess::renameRelation(oldRelName, newRelName);
    // return retVal
}

int Schema::renameAttr(char *relName, char *oldAttrName, char *newAttrName) {
    // if the relName is either Relation Catalog or Attribute Catalog,
        // return E_NOTPERMITTED
        if( (strcmp(relName,RELCAT_RELNAME)==0)||(strcmp(relName,ATTRCAT_RELNAME)==0)) return E_NOTPERMITTED; 
        // (check if the relation names are either "RELATIONCAT" and "ATTRIBUTECAT".
        // you may use the following constants: RELCAT_RELNAME and ATTRCAT_RELNAME)
int ret=OpenRelTable::getRelId(relName);
  if(ret!=E_RELNOTOPEN) return E_RELOPEN;
    // if the relation is open
        //    (check if OpenRelTable::getRelId() returns E_RELNOTOPEN)
        //    return E_RELOPEN
ret= BlockAccess::renameAttribute(relName,oldAttrName,newAttrName); 
return ret;
    // Call BlockAccess::renameAttribute with appropriate arguments.

    // return the value returned by the above renameAttribute() call
}









int Schema::createRel(char relName[],int nAttrs, char attrs[][ATTR_SIZE],int attrtype[]){
  Attribute relNameAsAttribute;
  strcpy(relNameAsAttribute.sVal, relName);   
    // declare variable relNameAsAttribute of type Attribute
    // copy the relName into relNameAsAttribute.sVal
RecId targetRelId ;
RelCacheTable::resetSearchIndex(RELCAT_RELID);
RecId relcatRecId = BlockAccess::linearSearch(RELCAT_RELID, (char *)"RelName", relNameAsAttribute, EQ);  
    // declare a variable targetRelId of type RecId
if(relcatRecId.block!=-1 && relcatRecId.slot!=-1) return E_RELEXIST;  
    // Reset the searchIndex using RelCacheTable::resetSearhIndex()
    // Search the relation catalog (relId given by the constant RELCAT_RELID)
    // for attribute value attribute "RelName" = relNameAsAttribute using
    // BlockAccess::linearSearch() with OP = EQ

    // if a relation with name `relName` already exists  ( linearSearch() does
    //                                                     not return {-1,-1} )
    //     return E_RELEXIST;

    // compare every pair of attributes of attrNames[] array
    // if any attribute names have same string value,
    //     return E_DUPLICATEATTR (i.e 2 attributes have same value)

   for(int i=0;i<nAttrs;i++){
      for(int j=i+1;j<nAttrs;j++){
        if(strcmp(attrs[i],attrs[j])==0)return E_DUPLICATEATTR;

      }
    }

    /* declare relCatRecord of type Attribute which will be used to store the
       record corresponding to the new relation which will be inserted
       into relation catalog */
    Attribute relCatRecord[RELCAT_NO_ATTRS];
    // fill relCatRecord fields as given below
    // offset RELCAT_REL_NAME_INDEX: relName
    // offset RELCAT_NO_ATTRIBUTES_INDEX: numOfAttributes
    // offset RELCAT_NO_RECORDS_INDEX: 0
    // offset RELCAT_FIRST_BLOCK_INDEX: -1
    // offset RELCAT_LAST_BLOCK_INDEX: -1
    // offset RELCAT_NO_SLOTS_PER_BLOCK_INDEX: floor((2016 / (16 * nAttrs + 1)))
    // (number of slots is calculated as specified in the physical layer docs)

        strcpy(relCatRecord[RELCAT_REL_NAME_INDEX].sVal,relName);
    relCatRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal=nAttrs;
    relCatRecord[RELCAT_NO_RECORDS_INDEX].nVal=0;
    relCatRecord[RELCAT_FIRST_BLOCK_INDEX].nVal=-1;
    relCatRecord[RELCAT_LAST_BLOCK_INDEX].nVal=-1;
    int numofslots=floor(2016*1.00/(16*nAttrs+1));
    relCatRecord[RELCAT_NO_SLOTS_PER_BLOCK_INDEX].nVal=numofslots;


    // retVal = BlockAccess::insert(RELCAT_RELID(=0), relCatRecord);
        int retVal=BlockAccess::insert(RELCAT_RELID,relCatRecord);
    if(retVal!=SUCCESS)return retVal;
    // if BlockAccess::insert fails return retVal
    // (this call could fail if there is no more space in the relation catalog)

    // iterate through 0 to numOfAttributes - 1 :
    for(int attrIndex=0;attrIndex<nAttrs;attrIndex++)
    {
      Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
        /* declare Attribute attrCatRecord[6] to store the attribute catalog
           record corresponding to i'th attribute of the argument passed*/
        // (where i is the iterator of the loop)
        // fill attrCatRecord fields as given below
        // offset ATTRCAT_REL_NAME_INDEX: relName
        // offset ATTRCAT_ATTR_NAME_INDEX: attrNames[i]
        // offset ATTRCAT_ATTR_TYPE_INDEX: attrTypes[i]
        // offset ATTRCAT_PRIMARY_FLAG_INDEX: -1
        // offset ATTRCAT_ROOT_BLOCK_INDEX: -1git 
        // offset ATTRCAT_OFFSET_INDEX: i
        strcpy(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal,relName);
        strcpy(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,attrs[attrIndex]);
        attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal=attrtype[attrIndex];
        attrCatRecord[ATTRCAT_PRIMARY_FLAG_INDEX].nVal=-1;
        attrCatRecord[ATTRCAT_ROOT_BLOCK_INDEX].nVal=-1;
        attrCatRecord[ATTRCAT_OFFSET_INDEX].nVal=attrIndex;
        // retVal = BlockAccess::insert(ATTRCAT_RELID(=1), attrCatRecord);
        retVal=BlockAccess::insert(ATTRCAT_RELID,attrCatRecord);
        if(retVal!=SUCCESS){
          Schema::deleteRel(relName);
          return E_DISKFULL;
        }
        /* if attribute catalog insert fails:
             delete the relation by calling deleteRel(targetrel) of schema layer
             return E_DISKFULL
             // (this is necessary because we had already created the
             //  relation catalog entry which needs to be removed)
        */
       
    }
return SUCCESS;
    // return SUCCESS
}





/*

OpenRelTable::~OpenRelTable()
{
	// free all the memory that you allocated in the constructor

	//? close all open relations (from rel-id = 2 onwards. Why?)
	for (int i = 2; i < MAX_OPEN; ++i)
		if (!tableMetaInfo[i].free)
			OpenRelTable::closeRel(i); // we will implement this function later

	if(RelCacheTable::relCache[ATTRCAT_RELID]->dirty==true){
		RelCatEntry relCatBuffer;
		RelCacheTable::getRelCatEntry(ATTRCAT_RELID,&relCatBuffer);
		Attribute relCatRecord[RELCAT_NO_ATTRS];
		RelCacheTable::relCatEntryToRecord(&relCatBuffer,relCatRecord);
		RecId recId=RelCacheTable::relCache[ATTRCAT_RELID]->recId;
		RecBuffer relCatBlock(recId.block);
		relCatBlock.setRecord(relCatRecord,recId.slot);
	}
	free(RelCacheTable::relCache[ATTRCAT_RELID]);

	if(RelCacheTable::relCache[RELCAT_RELID]->dirty==true){
		RelCatEntry relCatBuffer;
		RelCacheTable::getRelCatEntry(RELCAT_RELID,&relCatBuffer);
		Attribute relCatRecord[ATTRCAT_NO_ATTRS];
		RelCacheTable::relCatEntryToRecord(&relCatBuffer,relCatRecord);
		RecId recId=RelCacheTable::relCache[RELCAT_RELID]->recId;
		RecBuffer attrCatRecord(recId.block);
		attrCatRecord.setRecord(relCatRecord,recId.slot);

	}
	free(RelCacheTable::relCache[RELCAT_RELID]);
	// free the memory allocated for rel-id 0 and 1 in the caches
	for(int relID=RELCAT_RELID;relID<=ATTRCAT_RELID;relID++){
		AttrCacheEntry *curr=AttrCacheTable::attrCache[relID],*next=NULL;
		while(curr!=nullptr){
			next=curr->next;
			if(curr->dirty==true){
				AttrCatEntry attrCatEntry=curr->attrCatEntry;
				Attribute AttrCatrecord[ATTRCAT_NO_ATTRS];
				AttrCacheTable::attrCatEntryToRecord(&attrCatEntry,AttrCatrecord);
				RecBuffer attrCatBlock(curr->recId.block);
				attrCatBlock.setRecord(AttrCatrecord,curr->recId.slot);
			}
			free(curr);
			curr=next;
		}
	}
}
*/


void BlockBuffer::releaseBlock(){

    // if blockNum is INVALID_BLOCKNUM (-1), or it is invalidated already, do nothing
  if (blockNum == INVALID_BLOCKNUM or
      StaticBuffer::blockAllocMap[blockNum] == UNUSED_BLK) {
    printf("invalid block");
    return;
  }
    // else
        /* get the buffer number of the buffer assigned to the block
           using StaticBuffer::getBufferNum().
           (this function return E_BLOCKNOTINBUFFER if the block is not
           currently loaded in the buffer)
            */

        // if the block is present in the buffer, free the buffer
        // by setting the free flag of its StaticBuffer::tableMetaInfo entry
        // to true.
  int bufferNum = StaticBuffer::getBufferNum(blockNum);
  if (bufferNum >= 0 and bufferNum < BUFFER_CAPACITY) {
    StaticBuffer::metainfo[bufferNum].free = true;
  }
        // free the block in disk by setting the data type of the entry
        // corresponding to the block number in StaticBuffer::blockAllocMap
        // to UNUSED_BLK.
StaticBuffer::blockAllocMap[blockNum] = UNUSED_BLK;
  this->blockNum = INVALID_BLOCKNUM;
        // set the object's blockNum to INVALID_BLOCK (-1)
}



int Schema::deleteRel(char *relName) {
    // if the relation to delete is either Relation Catalog or Attribute Catalog,
    //     return E_NOTPERMITTED
    if (strcmp(relName, RELCAT_RELNAME) == 0 || strcmp(relName, ATTRCAT_RELNAME) == 0)
		return E_NOTPERMITTED;
        // (check if the relation names are either "RELATIONCAT" and "ATTRIBUTECAT".
        // you may use the following constants: RELCAT_NAME and ATTRCAT_NAME)

    // get the rel-id using appropriate method of OpenRelTable class by
    // passing relation name as argument
    int relId=OpenRelTable::getRelId(relName);
    if(relId>0 and relId<MAX_OPEN)return E_RELOPEN;

    // if relation is opened in open relation table, return E_RELOPEN
    int retVal=BlockAccess::deleteRelation(relName);
    if(retVal!=SUCCESS)return retVal;
    return retVal;
    // Call BlockAccess::deleteRelation() with appropriate argument.

    // return the value returned by the above deleteRelation() call

    /* the only that should be returned from deleteRelation() is E_RELNOTEXIST.
       The deleteRelation call may return E_OUTOFBOUND from the call to
       loadBlockAndGetBufferPtr, but if your implementation so far has been
       correct, it should not reach that point. That error could only occur
       if the BlockBuffer was initialized with an invalid block number.
    */
}