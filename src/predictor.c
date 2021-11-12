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
uint32_t global_history;
//the gshare table
uint8_t *gs_table;
//local history table -> first level table of local history
uint32_t *lht;
//local history predictor table -> second level table of local history
uint8_t *lhp;
//global history predictor
uint8_t *ghp;
//the predictor makes choice based on the result form lhp and ghp
uint8_t *choice_predictor;
//used to get the ghisotryBits index (lower)
uint32_t global_mask;
uint32_t local_mask;
uint32_t pc_mask;
//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the gshare predictor
void init_gshare()
{
  //the history is initialized as strong not taken (all 0's) per the instruction.
  global_history = NOTTAKEN; 
  // the predictor size is: 2^(ghistoryBits);
  uint32_t size;
  size = 1<<ghistoryBits;
  //each entry is either 0,1,2,3 -> only two bits, but the smallest we can have is 1 byte (8 bits).
  gs_table = (uint8_t *) malloc(sizeof(uint8_t) * size);
  //printf("size is: %d\n", size);
  //each index (entry)
  uint8_t mask = 0b0;
  for(unsigned int i = 0; i < size; i++){
    //initilize as weak taken (which is 1) by default
    gs_table[i] = mask | WN;
  }
  printf("successfully assign value to gs_table\n");
  global_mask = 0;
  //if ghistoryBits is 6 -> then we get 0b111111
  for(int i = 0; i < ghistoryBits; i++){
    global_mask = (global_mask << 1) | 1;
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
  //printf("lhp size is: %d\n", lhp_size);
  //size is lhp_size and, for each entry, we only need 8 bits since the counter only has value 0 1 2 3
  lhp = (uint8_t *) malloc(sizeof(uint8_t)*lhp_size);
  for(unsigned int i = 0; i < lhp_size; i++){
    lhp[i] = mask | WN;
  }

  //set up the local_history_mask for later use
  local_mask = 0;
  for(unsigned int i = 0; i < lhistoryBits; i++){
    local_mask = (local_mask << 1) | 1;
  }
  //same for set up pc
  pc_mask = 0;
  for(unsigned int i = 0; i < pcIndexBits; i++){
    pc_mask = (pc_mask << 1) | 1;
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
  global_history = NOTTAKEN;
  ghp = (uint8_t *) malloc(sizeof(uint8_t)*size);
  choice_predictor = (uint8_t *) malloc(sizeof(uint8_t)*size);
  //set as WN by default
  for(unsigned int i = 0; i < size; i++){
    ghp[i] = mask | WN;
    choice_predictor[i] =  mask |   WN;
  }
  //if ghistoryBits is 6 -> then we get 0b111111
  for(int i = 0; i < ghistoryBits; i++){
    global_mask = (global_mask << 1) | 1;
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
  uint32_t gshare_index;
  uint32_t tournament_index;
  uint32_t local_index;
  //uint32_t global_index;
  //printf("index is: %d\n", gshare_index);
  uint8_t gshare_prediction;
  uint8_t tournament_prediction;
  uint8_t local_prediction;
  uint8_t global_prediction;
  uint8_t result = 0;
  
  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      gshare_index = (pc & global_mask) ^ (global_history & global_mask);
      gshare_prediction = gs_table[gshare_index];
      //printf("get the prediction\n");
      //cast the 4 bytes integers to be 1 byte
      if(gshare_prediction >= 2) return result | TAKEN;
      return result;
    case TOURNAMENT:
      //printf("it runs here\n");
      tournament_index = global_history & global_mask;
      tournament_prediction = choice_predictor[tournament_index];
      //get only lower pcbits and get the local history -> then get the lower localbits 
      local_index = (lht[pc&pc_mask]) & local_mask;
      local_prediction = lhp[local_index];
      //tournament_index is the same as global history index
      global_prediction = ghp[tournament_index];
      //use the result of p1, where we set as local predictor
      if(tournament_prediction >= 2){
        if(local_prediction >= 2) return result | TAKEN;
        return result;
      }
      //otherwise, we choose the prediction result from the global predictor.
      else{
        if(global_prediction >= 2) return result | TAKEN;
        return result;
      }
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
  uint32_t index = (pc & global_mask) ^ (global_history & global_mask);
  //update the gshare_history
  global_history = (global_history << 1) | outcome;
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
  uint8_t local_prediction = lhp[lht[pc & pc_mask] & local_mask];
  uint8_t global_prediction = ghp[global_history & global_mask];
  if(local_prediction >= 2) local_prediction = TAKEN;
  else local_prediction = NOTTAKEN;

  if(global_prediction >= 2) global_prediction = TAKEN;
  else global_prediction = NOTTAKEN;
  //check the prediction is correct or not.
  uint8_t p1_correct = local_prediction & outcome;
  uint8_t p2_correct = global_prediction & outcome;
  //Increment the counter in the choice_predictor per the instruction
  if(p1_correct == 1 && p2_correct == 0){
    if(choice_predictor[global_history & global_mask] <= WT)
      choice_predictor[global_history & global_mask]++;
  }
  //Decrement
  else if(p1_correct == 0 && p2_correct == 1){
    if(choice_predictor[global_history & global_mask] >= WN)
      choice_predictor[global_history & global_mask]--;
  }
  //update the global predictor
  if(outcome == TAKEN){
    //update the ghp and lhp based on the true outcome
    if(ghp[global_history & global_mask] <= WT)
      ghp[global_history & global_mask]++;
    if(lhp[lht[pc & pc_mask] & local_mask] <= WT)
      lhp[lht[pc & pc_mask] & local_mask]++;
  }
  else{
    //same above
    if(ghp[global_history & global_mask] >= WN)
      ghp[global_history & global_mask]--;
    if(lhp[lht[pc & pc_mask] & local_mask] >= WN)
      lhp[lht[pc & pc_mask] & local_mask]--;
  }
  //update the global history
  global_history = (global_history << 1) | outcome;
  //update the local history
  lht[pc & pc_mask] = (lht[pc & pc_mask] << 1) | outcome;
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
