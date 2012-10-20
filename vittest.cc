/**
 * @file       vittest.cc
 * @author     bdsatish
 * @date       Oct 20, 2012
 * @copyright  Copyright (C) 2012, Satish BD, see LICENSE file for details
 * 
 * @brief      Test driver for Viterbi decoder
 */

#include "vitdec.hh"
#include <cstdlib>
#include <cassert>
#include <iostream>
using namespace std;

Stage* init_stage(unsigned outBits);
void delete_stage(Stage* stage);
Trellis* getTrellis();
void test_trellis(Trellis* trellis);
void test_small_sample(Trellis* trellis);

int main()
{
  Trellis* trellis = getTrellis();
  trellis->build();

  test_small_sample(trellis);
  delete trellis;

  return 0;
}

Stage* init_stage(unsigned outBits)
{
  vector<Branch> survivors;
  Metric branchMetrics[] = {0b00, 0b11, 0b10, 0b01};

  survivors.push_back(Branch(0, 0, 0, branchMetrics[0]));
  survivors.push_back(Branch(0, 2, 1, branchMetrics[1]));
  survivors.push_back(Branch(2, 1, 0, branchMetrics[2]));
  survivors.push_back(Branch(2, 3, 1, branchMetrics[3]));

  vector<Metric> totalMetrics;

  totalMetrics.push_back(outBits ^ branchMetrics[0]);
  totalMetrics.push_back(outBits ^ branchMetrics[1]);
  totalMetrics.push_back(outBits ^ branchMetrics[2]);
  totalMetrics.push_back(outBits ^ branchMetrics[3]);

  Stage* previousStage = nullptr;
  Stage* stage = new Stage(survivors, totalMetrics, previousStage);
  assert(stage->previousStage() == nullptr);

  return stage;
}

void delete_stage(Stage* stage)
{
  while (stage->previousStage() != nullptr)
  {
    Stage* temp = stage->previousStage();
    delete stage;
    stage = temp;
  }
  delete stage;
}

/*
 * From MATLAB:
 * trel = poly2trellis(3,[7 5]);
 * trel.nextStates = [0,2;0,2;1,3;1,3]
 * trel.outputs    = [0,3;3,0;2,1;1,2];
 */
Trellis* getTrellis()
{
  Trellis* trellis = new Trellis();
  trellis->push_back(0, 2, 0, 3);
  trellis->push_back(0, 2, 3, 0);
  trellis->push_back(1, 3, 2, 1);
  trellis->push_back(1, 3, 1, 2);

  return trellis;
}

/* MATLAB comparison:
 * >> trel = poly2trellis(3,[7 5]);    % Define trellis
 * >> msg = [1 1 0 0 1 0 0 1 0 0 1 0 1 0 1 1];
 * >> code = convenc(msg, trel)        % Corresponds to outBits[] below
 *      [1 1 0 1 0 1 1 1 1 1 1 0 1 1 1 1 1 0 1 1 1 1 1 0 0 0 1 0 0 0 0 1]
 * >> tblen = 1;                       % Must be as small as possible for the optimal decoding
 * >> deco = vitdec(code, trel, 1, 'trunc', 'hard')
 * >>   [1 1 0 0 1 0 0 1 0 0 1 0 1 0 1 1]     % same as expected[] below
 */
void test_small_sample(Trellis* trellis)
{
  DataType outBits[] = { 0b11, 0b01, 0b01, 0b11, 0b11, 0b10, 0b11, 0b11,
                         0b10, 0b11, 0b11, 0b10, 0b00, 0b10, 0b00, 0b01 };

  Stage* nextStage = init_stage(outBits[0]);
  const int numStages = sizeof(outBits)/sizeof(outBits[0]);

  // Should start from i=1 because the stage i=0 is already done by init_stage() ??
  for (int i = 0; i < numStages; i++)
  {
    Stage* stage = nextStage;
    nextStage = updateSurvivors(stage, outBits[i], trellis->branchPairs());
  }

  vector<Bit> decoded = tracebackDecode(nextStage);
  Bit expected[] = {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1};

  for (size_type i = 0; i < decoded.size(); i++)
  {
    assert(expected[i] == decoded[i]);
  }

  delete_stage(nextStage);

}
