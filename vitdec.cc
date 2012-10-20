/**
 * @file   vitdec.cc
 * @author bdsatish
 * @date   Oct 20, 2012
 * 
 * @brief  Implementation file for the Viterbi decoder
 * 
 */

#include "vitdec.hh"
using namespace std;

void updateSurvivors(Stage& stage, DataType outBits, const BranchPairs& branchPairs)
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
    Metric distance0 = stage.totalMetric(b0.from()) + distanceBetween(b0.metric(), outBits);
    Metric distance1 = stage.totalMetric(b1.from()) + distanceBetween(b1.metric(), outBits);

    // Step COMPARE and SELECT:
    // TODO toss a coin if distance0 == distance1
    if (distance0 < distance1) {
      survivors.push_back(b0);
      totalMetrics.push_back(distance0);
    }
    else {
      survivors.push_back(b1);
      totalMetrics.push_back(distance1);
    }

  }
}

bool operator==(Branch &b1, Branch &b2)
{
  return (b1.mFrom == b2.mFrom &&
          b1.mTo == b2.mTo &&
          b1.mMetric == b2.mMetric &&
          b1.mBit == b2.mBit
         );
}

