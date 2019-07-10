#ifndef __AUI_ID_H_
#define __AUI_ID_H_

#include "moroccan_names.h"

#include "stdlib.h" /* rand() */ 

// arrays used to make out random ids

char semesters[16][4] = { "001", "002", "011", "012", "021", "022", "031", "032",
						  "041", "042", "051", "052", "061", "062", "071", "072" };

char cities[8][3] = { "A", "B", "BK", "BE", "G", "K", "X", "T" };

char *GetRandomIdentity()
{
  char *identity = new char[50];
  char id[20];
  sprintf(id,"%s%s%d",semesters[rand()%16],
					  cities[rand()%8],
					  15536+rand()%23584);				  
  strcpy(identity, id);	
  strcat(identity," ");
  strcat(identity,moroccan_last_names[rand()%25]);
  strcat(identity," ");
  strcat(identity,moroccan_first_names[rand()%25]);
  return identity;
}

#endif