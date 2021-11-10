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
int pcIndexBits;  // Number of bits used for PC index -> the branch adddress, assuming it is n bits
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//
//
//TODO: Add your own Branch Predictor data structures here
//the total history of the gshare
uint32_t gshare_history;
//the mapping table
uint32_t *gs_table;
//used to get the ghisotryBits index (lower)
uint32_t gshare_mask;
//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void init_gshare()
{
  //the history is initialized as strong not taken (all 0's) per the instruction.
  gshare_history = SN; 
  // the predictor size is: 2^(ghistoryBits);
  uint32_t size;
  size = 1<<ghistoryBits;
  //each entry is either 0,1,2,3 -> only two bits, but the smallest we can have is 1 byte (8 bits).
  gs_table = malloc(sizeof(uint8_t) * size);
  //each index (entry)
  for(unsigned int i = 0; i < size; i++){
    uint8_t mask = 0b0;
    //initilize as weak taken (which is 1) by default
    gs_table[i] = mask | WN;
  }
  gshare_mask = 0;
  //if ghistoryBits is 6 -> then we get 0b111111
  for(int i = 0; i < ghistoryBits; i++){
    gshare_mask = (gshare_mask << i) | 1;
  }
}
void
init_predictor()
{
  if(bpType == GSHARE){
    init_gshare();
  }

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
uint32_t actual_gshare_history = gshare_history & gshare_mask;
uint32_t gshare_index = pc ^ actual_gshare_history;
uint8_t gshare_prediction;
uint8_t result = 0;
  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      gshare_prediction = gs_table[gshare_index];
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
  uint8_t result = make_prediction(pc);
  uint32_t index = pc ^ (gshare_history & gshare_mask);
  gshare_history = gshare_history << 1 | result;
}
