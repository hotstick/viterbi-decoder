/**
 * @file       vitdec.cc
 * @author     bdsatish
 * @date       Oct 20, 2012
 * @copyright  Copyright (C) 2012, Satish BD, see LICENSE file for details
 * 
 * @brief      Implementation file for the Viterbi decoder
 * 
 */

#include "vitdec.hh"
#include <algorithm>
using namespace std;

static void delete_stage(Stage* stage);

Stage* updateSurvivors(Stage* stage, DataType outBits, const BranchPairs& branchPairs)
{
  vector<Branch> survivors;
  vector<Metric> totalMetrics;

  size_type sz = branchPairs.size();
  for (size_type i = 0; i < sz; ++i)
  {
    vector<Branch> branchPair = branchPairs[i];
    assert(branchPair.size() == 2);

    Branch b0 = branchPair[0];
    Branch b1 = branchPair[1];

    assert(b0.to() == b1.to() && b0.to() == i);

    // Step ADD:
    Metric distance0 = stage->totalMetric(b0.from()) + distanceBetween(b0.metric(), outBits);
    Metric distance1 = stage->totalMetric(b1.from()) + distanceBetween(b1.metric(), outBits);

    // Step COMPARE and SELECT:
    // TODO toss a coin if distance0 == distance1
    if (distance0 < distance1) {
      survivors.push_back(b0);
      totalMetrics.push_back(distance0);
    }
    else {
      survivors.push_back(b1);
      totalMetrics.push_back(distance1);
    } //~ end if
  } //~ end for

  Stage* nextStage = new Stage(survivors, totalMetrics, stage);
  return nextStage;
}

vector<Bit> tracebackDecode(Stage* finalStage)
{
  vector<Bit> decoded;

  // Find the state with minimum of all metrics
  size_type where = finalStage->findMinLoc();         // location of minimum metric
  Branch branch = finalStage->survivor(where);        // surviving branch with that minimum metric
  decoded.push_back(branch.bit());                    // 0 or 1

  while (finalStage->previousStage() != nullptr)
  {
    Stage* stage = finalStage->previousStage();
    State from = branch.from();
    branch = stage->survivor(from);
    decoded.push_back(branch.bit());
    finalStage = stage;
  }

  // TODO We need to discard log2(4) - 1 elements before reversing ??
  // decoded.pop_back();

  // Reversed because we start from finalStage and go backwards, decoding right-to-left
  // The sequence is read-out left-to-right, agrees with MATLAB output
  reverse(decoded.begin(), decoded.end());

  return decoded;
}

Stage* computeFirstStage(DataType outBits, const BranchPairs& branchPairs)
{
  vector<Branch> survivors;     // Empty

  vector<Metric> totalMetrics;
  for (size_type i = 0; i < branchPairs.size(); ++i)
  {
    totalMetrics.push_back(0);   // Initial metric is all-zeros
  }

  Stage* previousStage = nullptr;
  Stage* stage = new Stage(survivors, totalMetrics, previousStage);
  assert(stage->previousStage() == nullptr);

  // Compute one ACS step (first stage)
  // this method doesn't access stage->survivors(...) so we are OK
  Stage* nextStage = updateSurvivors(stage, outBits, branchPairs);
  assert(nextStage->previousStage() == stage);

  // Survivors and totalMetrics are updated in nextStage.
  // stage must be deleted because stage->survivors(...) are empty
  delete_stage(stage);
  nextStage->previousStage(nullptr);

  return nextStage;
}

vector<Bit> viterbiDecode(DataType outBits[], Trellis* trellis, size_type numStages)
{
  const BranchPairs& branchPairs = trellis->branchPairs();
  Stage* nextStage = computeFirstStage(outBits[0], branchPairs);

  // Start from i=1 because the stage i=0 is done above
  for (size_type i = 1; i < numStages; i++)
  {
    Stage* stage = nextStage;
    nextStage = updateSurvivors(stage, outBits[i], branchPairs);
  }

  vector<Bit> decoded = tracebackDecode(nextStage);
  delete_stage(nextStage);

  return decoded;
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

