/**
 * @file   vittest.cc
 * @author bdsatish
 * @date   Oct 20, 2012
 * 
 * @brief  Test driver for Viterbi decoder
 * 
 */

#include "vitdec.hh"
#include <cstdlib>
#include <cassert>
using namespace std;

Stage* init_stage(unsigned outBits);
void delete_stage(Stage* stage);
Trellis* getTrellis();
void test_trellis(Trellis* trellis);

int main()
{
  DataType outBits[] = {0b01, 0b01};
  Stage* nextStage = init_stage(outBits[0]);
  Trellis* trellis = getTrellis();
  trellis->build();

  test_trellis(trellis);

  const int tracebackLen = 7;

  for (int i = 0; i < tracebackLen; i++)
  {
    Stage* stage = nextStage;
    nextStage = updateSurvivors(stage, outBits[i], trellis->branchPairs());
  }

  delete_stage(nextStage);
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

void test_trellis(Trellis* trellis)
{
  BranchPairs pairs = trellis->branchPairs();

  assert(pairs.size() == 4);
  vector<Branch> pair;

}
