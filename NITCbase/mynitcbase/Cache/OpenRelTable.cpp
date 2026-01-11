#include "OpenRelTable.h"
#include<stdlib.h>
#include <cstring>

OpenRelTable::OpenRelTable() {

  // initialize relCache and attrCache with nullptr
  for (int i = 0; i < MAX_OPEN; ++i) {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
  }

  /************ Setting up Relation Cache entries ************/
  // (we need to populate relation cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Relation Cache Table****/
  RecBuffer relCatBlock(RELCAT_BLOCK);

  Attribute relCatRecord[RELCAT_NO_ATTRS];
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);

  struct RelCacheEntry relCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

  // allocate this on the heap because we want it to persist outside this function
  RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;

  /**** setting up Attribute Catalog relation in the Relation Cache Table ****/
        Attribute attrCatRecord[RELCAT_NO_ATTRS];
        relCatBlock.getRecord(attrCatRecord,RELCAT_SLOTNUM_FOR_ATTRCAT);
        struct RelCacheEntry attrRelCacheEntry;
        RelCacheTable::recordToRelCatEntry(attrCatRecord,&attrRelCacheEntry.relCatEntry);
        attrRelCacheEntry.recId.block=RELCAT_BLOCK;
        attrRelCacheEntry.recId.slot=RELCAT_SLOTNUM_FOR_ATTRCAT;
        RelCacheTable::relCache[ATTRCAT_RELID]= (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
        *(RelCacheTable::relCache[ATTRCAT_RELID])=attrRelCacheEntry;
  // set up the relation cache entry for the attribute catalog similarly
  // from the record at RELCAT_SLOTNUM_FOR_ATTRCAT

  // set the value at RelCacheTable::relCache[ATTRCAT_RELID]


  /************ Setting up Attribute cache entries ************/
  // (we need to populate attribute cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Attribute Cache Table ****/
  RecBuffer attrCatBlock(ATTRCAT_BLOCK);

  //Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

  // iterate through all the attributes of the relation catalog and create a linked
  // list of AttrCacheEntry (slots 0 to 5)
  // for each of the entries, set
  //    attrCacheEntry.recId.block = ATTRCAT_BLOCK;
  //    attrCacheEntry.recId.slot = i   (0 to 5)
  //    and attrCacheEntry.next appropriately
  // NOTE: allocate each entry dynamically using malloc
  struct AttrCacheEntry*head=NULL;
  struct AttrCacheEntry*prev=NULL;
for(int i=0;i<RELCAT_NO_ATTRS;i++){
    attrCatBlock.getRecord(attrCatRecord,i);
    struct AttrCacheEntry attrCacheEntry;
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&attrCacheEntry.attrCatEntry);
    attrCacheEntry.recId.block=ATTRCAT_BLOCK;
    attrCacheEntry.recId.slot=i;
    if(head==NULL){ head=(struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
    prev=head;
    *head=attrCacheEntry;
}
    else{
        prev->next=(struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
    prev=prev->next;
    *prev=attrCacheEntry;
    }
    }
prev->next=NULL;
  // set the next field in the last entry to nullptr

  AttrCacheTable::attrCache[RELCAT_RELID] = head;/* head of the linked list */;

  /**** setting up Attribute Catalog relation in the Attribute Cache Table ****/
  

  //struct AttrCacheEntry *head=NULL;
  //struct AttrCacheEntry *prev=NULL;


  for(int i=RELCAT_NO_ATTRS;i<RELCAT_NO_ATTRS+ATTRCAT_NO_ATTRS;i++){
  	attrCatBlock.getRecord(attrCatRecord,i);
	struct AttrCacheEntry attrCacheEntry;
	AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&attrCacheEntry.attrCatEntry);
	struct AttrCacheEntry*temp= (struct AttrCacheEntry*)malloc(sizeof(struct AttrCacheEntry));
	*(temp) = attrCacheEntry ;
	if(head==NULL){
		head = temp;
		prev = temp;
	}
	else{
		
		prev->next=temp;
		prev=prev->next;
	}
	
  }

	AttrCacheTable::attrCache[ATTRCAT_RELID]=head;

  // set up the attributes of the attribute cache similarly.
  // read slots 6-11 from attrCatBlock and initialise recId appropriately

  // set the value at AttrCacheTable::attrCache[ATTRCAT_RELID]
}

OpenRelTable::~OpenRelTable() {

	for(int i=0;i<MAX_OPEN;i++){
		if(RelCacheTable::relCache[i]!=NULL)
		free(RelCacheTable::relCache[i]);
}

	for(int i=0;i<MAX_OPEN;i++){
		if(AttrCacheTable::attrCache[i]!=NULL){
			struct AttrCacheEntry*head=AttrCacheTable::attrCache[i];
			while(head!=NULL){
				struct AttrCacheEntry * temp=head->next;
				free(head);
				head=temp;
			}


		}
	}
}
