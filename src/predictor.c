//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"
//test
//
// TODO:Student Information
//
const char *studentName = "Shanchuan You";
const char *studentID   = "A14903804";
const char *email       = "shy228@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History -> the history of previous m prediction outcome
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//
//
//TODO: Add your own Branch Predictor data structures here
//the total history of the gshare
uint32_t gshare_history;
//the gshare table
uint8_t *gs_table;
//local history table -> first level table of local history
uint32_t *lht;
//local history predictor table -> second level table of local history
uint8_t *lhp;
//global history predictor
uint32_t *ghp;
//the predictor makes choice based on the result form lhp and ghp
uint32_t *choice_predictor;
//used to get the ghisotryBits index (lower)
uint32_t gshare_mask;
//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the gshare predictor
void init_gshare()
{
  //the history is initialized as strong not taken (all 0's) per the instruction.
  gshare_history = SN; 
  // the predictor size is: 2^(ghistoryBits);
  uint32_t size;
  size = 1<<ghistoryBits;
  //each entry is either 0,1,2,3 -> only two bits, but the smallest we can have is 1 byte (8 bits).
  gs_table = (uint8_t *) malloc(sizeof(uint8_t) * size);
  printf("size is: %d\n", size);
  //each index (entry)
  uint8_t mask = 0b0;
  for(unsigned int i = 0; i < size; i++){
    //initilize as weak taken (which is 1) by default
    gs_table[i] = mask | WN;
  }
  printf("successfully assign value to gs_table\n");
  gshare_mask = 0;
  //if ghistoryBits is 6 -> then we get 0b111111
  for(int i = 0; i < ghistoryBits; i++){
    gshare_mask = (gshare_mask << 1) | 1;
  }
  
}

/*
a 2-level branch predictor that 

1th level: pcindex -> modify branch address as index -> 2^(pcIndexBits) entries and
each entry has length lhistoryBits;

2nd level: lhistoryBits as index -> 2^(lhistoryBits) entries and each entry has length 2 -> 2 - bits predictor.
In our case, would be a uint8_t size.

*/
void init_local()
{
  //local history table size -> lht_size
  uint32_t lht_size;
  //local history predictor size 
  uint32_t lhp_size;
  //used to cut the 4 bytes integer to 1 byte as our table only has 1 byte size for each entry
  uint8_t mask = 0b0;
  lht_size = 1 << pcIndexBits;
  //size is lht_size and each entry, for simple, use uint32_t since lhistoryBits at most uint32_t.
  lht = (uint32_t *) malloc(sizeof(uint32_t)*lht_size);
  //initialize each local history as 0 -> NONTAKEN
  for(unsigned int i = 0; i < lht_size; i++){
    lht[i] = NOTTAKEN;
  }
  lhp_size = 1 << lhistoryBits;
  //size is lhp_size and, for each entry, we only need 8 bits since the counter only has value 0 1 2 3
  lhp = (uint8_t *) malloc(sizeof(uint8_t)*lhp_size);
  for(unsigned int i = 0; i < lhp_size; i++){
    lhp[i] = mask | WN;
  }
}

/*
1. the global predictor which has a global_history table where each entry is the
global history prediction (k bits) -> there are 2^(globalhistoryBits) entries

2. another choice predictor is also initialized which has the same 2^(globalhistoryBits) entries
but it will choose the result based on the localhistory table and the globle history table
*/
void init_global()
{
  //size is 2^ghistoryBits 
  uint32_t size = 1 << ghistoryBits;
  uint8_t mask = 0b0;
  ghp = malloc(sizeof(uint8_t)*size);
  choice_predictor = malloc(sizeof(uint8_t)*size);
  //set as WN by default
  for(unsigned int i = 0; i < size; i++){
    ghp[i] = mask | WN;
    choice_predictor[i] =  mask |   WN;
  }
  
}

void
init_predictor()
{
  //printf("it runs here\n");
  if(bpType == GSHARE){
    init_gshare();
  }
  else if(bpType == TOURNAMENT){
    init_local();
    init_global();
  }
  //printf("gshare successfully initialized\n");
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //
  //printf("mask is: %d\n", gshare_mask);
  uint32_t gshare_index = (pc & gshare_mask) ^ (gshare_history & gshare_mask);
  //printf("index is: %d\n", gshare_index);
  uint8_t gshare_prediction;
  uint8_t result = 0;
  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      gshare_prediction = gs_table[gshare_index];
      //printf("get the prediction\n");
      //cast the 4 bytes integers to be 1 byte
      if(gshare_prediction >= 2) return result | 1;
      return result;
    case TOURNAMENT:
    case CUSTOM:
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

/*
train the gshare such that every time we make a prediction of the branch.
1. Update the prediction history -> gshare_history, for every prediction
2. Update the pattern history table if necessary
*/
void train_gshare(uint32_t pc, uint8_t outcome)
{
  //get the index
  uint32_t index = (pc & gshare_mask) ^ (gshare_history & gshare_mask);
  //update the gshare_history
  gshare_history = (gshare_history << 1) | outcome;
  //update the gshare_table
  uint8_t element = gs_table[index];
  //there are 8 possible situations
  if(element == SN){
    //if the outcome is taken but our element of this entry is strongly nontaken
    if(outcome == TAKEN) gs_table[index]++;
  }
  else if(element == WN){
    //change to weak taken
    if(outcome == TAKEN) gs_table[index]++;
    //change to strongly not taken
    else gs_table[index]--;
  }
  else if(element == WT){
    if(outcome == TAKEN) gs_table[index]++;
    else gs_table[index]--;
  }
  //this is the last case
  else{
    if(outcome == NOTTAKEN) gs_table[index]--;
  } 
}

/*
train the tournament predictor which is bascially a selector to select
between gshare and local.
*/
void train_tournament(uint32_t pc, uint8_t outcome)
{

}


// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//each take one branch and make prediction, then compare with the outcome and 
//see if we need to update the predictor
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //
  if(bpType == GSHARE){
    train_gshare(pc,outcome);
  }
  else if(bpType == TOURNAMENT){
    train_tournament(pc,outcome);
  }
}
