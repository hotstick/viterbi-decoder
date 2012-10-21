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
#include <cassert>   // for assert()
#include <utility>   // for make_pair()
using namespace std;

//... Forward Declarations ...
class Branch;

//... Typedefs ...

//... Inline functions ...
inline Metric distanceBetween(const Metric a, const Metric b) { return (a ^ b); }

//... Static methods ...
static Stage* updateSurvivors(Stage* stage, DataType outBits, const BranchPairs& branchPairs);
static void tracebackDecode(Stage* finalStage, vector<Bit>& decoded);
static Stage* computeFirstStage(DataType outBits, const BranchPairs& branchPairs);
static void delete_stage(Stage* stage);

//... Internal classes ...

//... Class Stage ...
class Stage
{
public:
  //... Constructors ...
  Stage(const std::vector<Branch>& survivors,
        const std::vector<Metric>& totalMetrics,
        Stage* previousStage
  )
  : mSurvivors(survivors)
  , mTotalMetrics(totalMetrics)
  , mPrevious(previousStage)
  { /* empty */  }


  //... Getters and Setters ...
  Stage* previousStage()  {  return mPrevious; }
  void previousStage(Stage* previous)  { mPrevious = previous; }
  Metric totalMetric(size_type i)  { return mTotalMetrics[i]; }
  Branch& survivor(size_type i)  { return mSurvivors[i]; }

  //... Methods ...
  size_type findMinLoc();

  //... Destructor ...
  virtual ~Stage() {};

private:
  Stage();
  explicit Stage(const Stage& that);
  Stage& operator=(const Stage& that);
  // mSurvivors[i] is the incoming, surviving branch at node i
  std::vector<Branch> mSurvivors;
  std::vector<Metric> mTotalMetrics;
  Stage* mPrevious;
};


//... Class Methods ...

//... Class Stage ...
size_type Stage::findMinLoc()
{
  size_type minloc = 0;
  Metric minMetric = mTotalMetrics[minloc];

  for (size_type i = 0; i < mTotalMetrics.size(); ++i)
  {
    if (mTotalMetrics[i] < minMetric)
    {
      minMetric = mTotalMetrics[i];
      minloc = i;
    }
  }

  return minloc;
}

//... Class Trellis ...
void Trellis::push_back(State state0, State state1, Metric metric0, Metric metric1)
{
  mNextStates.push_back(std::make_pair(state0, state1));
  mOutputs.push_back(std::make_pair(metric0, metric1));

  assert(mNextStates.size() == mOutputs.size());
}

/**
 *  mBranchPairs[i] contains the two Branches entering the state i
 */
void Trellis::build()
{
  size_type sz = mNextStates.size();

  // First build an empty matrix
  mBranchPairs.reserve(sz);
  for (size_type i = 0; i < sz; i++)
  {
    std::vector<Branch> temp;
    mBranchPairs.push_back(temp);
  }

  size_type sz1 = mNextStates.size();
  size_type sz2 = mBranchPairs.size();
  assert(sz1 == sz2);

  for (size_type i = 0; i < sz; i++)
  {
    NextState nextState = mNextStates[i];
    Output output = mOutputs[i];

    State from = (State)i;
    State to[] = { nextState.first, nextState.second };
    Bit bit[] = { 0, 1 };
    Metric metric[] = { output.first, output.second };

    Branch b0(from, to[0], bit[0], metric[0]);
    Branch b1(from, to[1], bit[1], metric[1]);

    mBranchPairs[to[0]].push_back(b0);
    mBranchPairs[to[1]].push_back(b1);

    assert(mBranchPairs[to[0]].size() <= 2);
    assert(mBranchPairs[to[1]].size() <= 2);
  }

  for (size_type i = 0; i < mNextStates.size(); i++)
  {
    // There must be exactly 2 branches entering per node.
    assert(mBranchPairs[i].size() == 2);
  }
}



//... Static functions ...
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

void tracebackDecode(Stage* finalStage, vector<Bit>& decoded)
{
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

void viterbiDecode(DataType outBits[], Trellis* trellis, size_type numStages, vector<Bit>& decoded)
{
  const BranchPairs& branchPairs = trellis->branchPairs();
  Stage* nextStage = computeFirstStage(outBits[0], branchPairs);

  // Start from i=1 because the stage i=0 is done above
  for (size_type i = 1; i < numStages; i++)
  {
    Stage* stage = nextStage;
    nextStage = updateSurvivors(stage, outBits[i], branchPairs);
  }

  tracebackDecode(nextStage, decoded);
  delete_stage(nextStage);
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

