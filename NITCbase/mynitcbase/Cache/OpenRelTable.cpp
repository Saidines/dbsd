#include "OpenRelTable.h"
#include<stdlib.h>
#include <cstring>

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];

OpenRelTable::OpenRelTable() {

  // initialize relCache and attrCache with nullptr
  for (int i = 0; i < MAX_OPEN; ++i) {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
    OpenRelTable::tableMetaInfo[i].free = true;
  }
  OpenRelTable::tableMetaInfo[RELCAT_RELID].free = false;
  OpenRelTable::tableMetaInfo[ATTRCAT_RELID].free = false;  
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

/*for student relation into relation catalog

	Attribute relStudentRecord[RELCAT_NO_ATTRS];
	relCatBlock.getRecord(relStudentRecord,2);
	struct RelCacheEntry relStudentEntry;
	RelCacheTable::recordToRelCatEntry(relStudentRecord,&relStudentEntry.relCatEntry);

	RelCacheTable::relCache[2]=(struct RelCacheEntry*)malloc(sizeof(relCacheEntry));
	*(RelCacheTable::relCache[2])=relStudentEntry;
  */





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

head=NULL;
prev=NULL;
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
    prev->next=NULL;

  /*setting attribute catalog for student relation head=NULL;
    prev=NULL;
    
     struct RelCatEntry relCatEntry=RelCacheTable::relCache[2]->relCatEntry;

    for(int i=RELCAT_NO_ATTRS+ATTRCAT_NO_ATTRS;i<RELCAT_NO_ATTRS+ATTRCAT_NO_ATTRS+relCatEntry.numAttrs;i++){
    	
	    struct AttrCacheEntry attrCacheEntry;
	    Attribute Record[ATTRCAT_NO_ATTRS];
	    attrCatBlock.getRecord(Record,i);
	    AttrCacheTable::recordToAttrCatEntry(Record,&attrCacheEntry.attrCatEntry);
		if(head==NULL){
			head=(struct AttrCacheEntry*)malloc(sizeof(struct AttrCacheEntry));
			*(head)=attrCacheEntry;
			prev=head;
		}
		else{
			prev->next=(struct AttrCacheEntry*)malloc(sizeof(struct AttrCacheEntry));

			prev=prev->next;
			*(prev)=attrCacheEntry;

		}


    }

prev->next=NULL;
AttrCacheTable::attrCache[2]=head;
*/
// set up the attributes of the attribute cache similarly.
  // read slots 6-11 from attrCatBlock and initialise recId appropriately

  // set the value at AttrCacheTable::attrCache[ATTRCAT_RELID]
}

OpenRelTable::~OpenRelTable() {

	for(int i=0;i<MAX_OPEN;i++){
		if(RelCacheTable::relCache[i]!=NULL){
		free(RelCacheTable::relCache[i]);
		RelCacheTable::relCache[i]=NULL;
		}
}

	for(int i=0;i<MAX_OPEN;i++){
		if(AttrCacheTable::attrCache[i]!=NULL){
			struct AttrCacheEntry*head=AttrCacheTable::attrCache[i];
			while(head!=NULL){
				struct AttrCacheEntry * temp=head->next;
				free(head);
				head=temp;
			}
			AttrCacheTable::attrCache[i]=NULL;


		}
	}
}


int OpenRelTable::getFreeOpenRelTableEntry() {
  for(int i=0;i<MAX_OPEN;i++){
  	if(OpenRelTable::tableMetaInfo[i].free==true){
  	OpenRelTable::tableMetaInfo[i].free=false;
      return i;}
  	}
    return E_CACHEFULL; 
  } 
  /* traverse through the tableMetaInfo array,
    find a free entry in the Open Relation Table.*/

  // if found return the relation id, else return E_CACHEFULL.




int OpenRelTable::openRel(char relName[ATTR_SIZE])
{
    int ret = OpenRelTable::getRelId(relName);
    if (ret != E_RELNOTOPEN)
    {
        return ret;
    }

    /* find a free slot in the Open Relation Table
       using OpenRelTable::getFreeOpenRelTableEntry(). */
    ret = OpenRelTable::getFreeOpenRelTableEntry();
    if (ret == E_CACHEFULL)
    {
        return E_CACHEFULL;
    }

    // let relId be used to store the free slot.
    int relId = ret;

    /****** Setting up Relation Cache entry for the relation ******/

    /* search for the entry with relation name, relName, in the Relation Catalog using
        BlockAccess::linearSearch().
        Care should be taken to reset the searchIndex of the relation RELCAT_RELID
        before calling linearSearch().*/
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
    // relcatRecId stores the rec-id of the relation `relName` in the Relation Catalog.
    Attribute check_relname;
    strcpy(check_relname.sVal, relName);
    char def_RELCAT_ATTR_RELNAME[16] = "RelName";
    RecId relcatRecId = BlockAccess::linearSearch(RELCAT_RELID, def_RELCAT_ATTR_RELNAME, check_relname, EQ);

    if (relcatRecId.block == -1 && relcatRecId.slot == -1)
    {
        // (the relation is not found in the Relation Catalog.)
        return E_RELNOTEXIST;
    }

    /* read the record entry corresponding to relcatRecId and create a relCacheEntry
        on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
        update the recId field of this Relation Cache entry to relcatRecId.
        use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
      NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
    */
    RecBuffer recEntry(relcatRecId.block);
    struct RelCacheEntry recCacheEntry;
    Attribute attrRecord[RELCAT_NO_ATTRS];
    recEntry.getRecord(attrRecord, relcatRecId.slot);
    RelCacheTable::recordToRelCatEntry(attrRecord, &recCacheEntry.relCatEntry);
    recCacheEntry.recId = relcatRecId;

    RelCacheTable::relCache[relId] = (struct RelCacheEntry *)malloc(sizeof(struct RelCacheEntry));
    *(RelCacheTable::relCache[relId]) = recCacheEntry;

    /****** Setting up Attribute Cache entry for the relation ******/
    RecBuffer attrEntry(relcatRecId.block);
    // let listHead be used to hold the head of the linked list of attrCache entries.
    AttrCacheEntry *listHead, *curr;
    listHead = curr = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
    int totalAttrs = recCacheEntry.relCatEntry.numAttrs;
    for (int i = 0; i < totalAttrs - 1; i++)
    {
        curr->next = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
        curr = curr->next;
    }
    curr->next = nullptr;
    curr = listHead;
    /*iterate over all the entries in the Attribute Catalog corresponding to each
    attribute of the relation relName by multiple calls of BlockAccess::linearSearch()
    care should be taken to reset the searchIndex of the relation, ATTRCAT_RELID,
    corresponding to Attribute Catalog before the first call to linearSearch().*/
    char def_ATTRCAT_ATTR_RELNAME[16] = "RelName";
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    for (int i = 0; i < totalAttrs; i++)
    {
        /* let attrcatRecId store a valid record id an entry of the relation, relName,
        in the Attribute Catalog.*/
        Attribute check_relname;
        strcpy(check_relname.sVal, relName);
        RecId attrcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID, def_ATTRCAT_ATTR_RELNAME, check_relname, EQ);

        /* read the record entry corresponding to attrcatRecId and create an
        Attribute Cache entry on it using RecBuffer::getRecord() and
        AttrCacheTable::recordToAttrCatEntry().
        update the recId field of this Attribute Cache entry to attrcatRecId.
        add the Attribute Cache entry to the linked list of listHead .*/
        // NOTE: make sure to allocate memory for the AttrCacheEntry using malloc()
        RecBuffer recbuffer(attrcatRecId.block);
        Attribute record[ATTRCAT_NO_ATTRS];
        recbuffer.getRecord(record, attrcatRecId.slot);
        AttrCacheTable::recordToAttrCatEntry(record, &curr->attrCatEntry);
        curr->recId = attrcatRecId;
        curr = curr->next;
    }

    // set the relIdth entry of the AttrCacheTable to listHead.
    AttrCacheTable::attrCache[relId] = listHead;
    /****** Setting up metadata in the Open Relation Table for the relation******/

    // update the relIdth entry of the tableMetaInfo with free as false and
    // relName as the input.
    tableMetaInfo[relId].free = false;
    strcpy(tableMetaInfo[relId].relName, relName);

    return relId;
}


int OpenRelTable::getRelId(char relName[ATTR_SIZE])
{
    for (int i = 0; i < MAX_OPEN; i++)
    {
        if (tableMetaInfo[i].free)
            continue;
        if (strcmp(tableMetaInfo[i].relName, relName) == 0)
            return i;
    }

    return E_RELNOTOPEN;
}


int OpenRelTable::closeRel(int relId) {
  	if (relId == RELCAT_RELID || relId == ATTRCAT_RELID) return E_NOTPERMITTED;

  	if (0 > relId || relId >= MAX_OPEN) return E_OUTOFBOUND;

  	if (tableMetaInfo[relId].free) return E_RELNOTOPEN;

	if (RelCacheTable::relCache[relId]->dirty == true) {
		/* Get the Relation Catalog entry from RelCacheTable::relCache
		Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
		Attribute relCatBuffer [RELCAT_NO_ATTRS];
		RelCacheTable::relCatEntryToRecord(&(RelCacheTable::relCache[relId]->relCatEntry), relCatBuffer);

		// declaring an object of RecBuffer class to write back to the buffer
		RecId recId = RelCacheTable::relCache[relId]->recId;
		RecBuffer relCatBlock(recId.block);

		// Write back to the buffer using relCatBlock.setRecord() with recId.slot
		relCatBlock.setRecord(relCatBuffer, RelCacheTable::relCache[relId]->recId.slot);
	}

	// free the memory allocated in the relation and attribute caches which was
	// allocated in the OpenRelTable::openRel() function
	free (RelCacheTable::relCache[relId]);
	
	// // RelCacheEntry *relCacheBuffer = RelCacheTable::relCache[relId];

	//* because we are not modifying the attribute cache at this stage,
	//* write-back is not required. We will do it in subsequent
  	//* stages when it becomes needed)

	AttrCacheEntry *head = AttrCacheTable::attrCache[relId];
	AttrCacheEntry *next = head->next;

	while (next) {
		free (head);
		head = next;
		next = next->next;
	}
	free(head);	

	// update `tableMetaInfo` to set `relId` as a free slot
	// update `relCache` and `attrCache` to set the entry at `relId` to nullptr
	tableMetaInfo[relId].free = true;
	RelCacheTable::relCache[relId] = nullptr;
	AttrCacheTable::attrCache[relId] = nullptr;

  return SUCCESS;
}


int RelCacheTable::setRelCatEntry(int relId, RelCatEntry *relCatBuf) {

  if(relId<0||relId>=MAX_OPEN) {
    return E_OUTOFBOUND;
  }

  if(RelCacheTable::relCache[relId]==nullptr/*entry corresponding to the relId in the Relation Cache Table is free*/) {
    return E_RELNOTOPEN;
  }
RelCacheTable::relCache[relId].relCatEntry=*relCatBuf;
  // copy the relCatBuf to the corresponding Relation Catalog entry in
  // the Relation Cache Table.
RelCacheTable::relCache[relId].dirty=1;
  // set the dirty flag of the corresponding Relation Cache entry in
  // the Relation Cache Table.

  return SUCCESS;
}