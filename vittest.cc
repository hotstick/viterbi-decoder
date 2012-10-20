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

void init_states();

int main()
{
  init_states();
  return 0;
}

void init_states()
{
  list<Branch> survivors;
  int outBits = 0b01;
  int branchMetrics[] = {0b00, 0b11, 0b10, 0b01};

  survivors.push_back(Branch(0, 0, 0, branchMetrics[0]));
  survivors.push_back(Branch(0, 1, 1, branchMetrics[1]));
  survivors.push_back(Branch(1, 2, 0, branchMetrics[2]));
  survivors.push_back(Branch(1, 3, 1, branchMetrics[3]));

  list<Metric> totalMetrics;

  totalMetrics.push_back(outBits ^ branchMetrics[0]);
  totalMetrics.push_back(outBits ^ branchMetrics[1]);
  totalMetrics.push_back(outBits ^ branchMetrics[2]);
  totalMetrics.push_back(outBits ^ branchMetrics[3]);

  Stage* previousStage = NULL;
  Stage stage(survivors, totalMetrics, *previousStage);
  assert(&stage.previousStage() == NULL);
}


