/**
 * @file   vitdec.hh
 * @author bdsatish
 * @date   Oct 20, 2012
 * 
 * @brief  API for Viterbi decoder
 * 
 */

#ifndef VITDEC_HH_
#define VITDEC_HH_

#include <cassert>   // for assert()
#include <utility>   // for make_pair()
#include <vector>

//... Forward Declarations ...
class Branch;

//... Typedefs ...
typedef std::vector<int>::size_type size_type;
typedef bool Bit;          // 0 = false, 1 = true
typedef unsigned DataType;
typedef unsigned Metric;
typedef unsigned State;
typedef std::pair<State, State> NextState;
typedef std::pair<Metric, Metric> Output;
typedef std::vector< std::vector<Branch> > BranchPairs;

//... Inline functions....
inline Metric distanceBetween(const Metric a, const Metric b) { return (a ^ b); }

class Branch
{
public:
  //... Constructors ...
  Branch(State from, State to, Bit bit, Metric metric)
  : mFrom(from)
  , mTo(to)
  , mBit(bit)
  , mMetric(metric)
  { /* empty */ }

  Branch() { /* empty */ };

  //... Getters and Setters ...
  Metric metric() { return mMetric; }
  State to() { return mTo; }
  State from() { return mFrom; }
  Bit bit() { return mBit; }
private:
  State mFrom;      // left node of the branch
  State mTo;        // right node of the branch
  Bit mBit;         // input bit
  Metric mMetric;   // output bits
};

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

  //... Methods ...
  size_type findMinLoc()
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

  //... Getters and Setters ...
  Stage* previousStage()  {  return mPrevious; }
  Metric totalMetric(size_type i)  { return mTotalMetrics[i]; }
  Branch survivor(size_type i)  { return mSurvivors[i]; }

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

class Trellis
{
public:
  //... Constructors ...
  Trellis() {}

  //... Methods ...
  void push_back(State state0, State state1, Metric metric0, Metric metric1)
  {
    mNextStates.push_back(std::make_pair(state0, state1));
    mOutputs.push_back(std::make_pair(metric0, metric1));

    assert(mNextStates.size() == mOutputs.size());
  }

  /**
   *  mBranchPairs[i] contains the two Branches entering the state i
   */
  void build()
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

  //... Getters ans Setters ...
  BranchPairs branchPairs()   { return mBranchPairs; }

private:
  std::vector<NextState> mNextStates;
  std::vector<Output> mOutputs;
  BranchPairs mBranchPairs;
};

//... Global functions ...
Stage* updateSurvivors(Stage* stage, DataType outBits, const BranchPairs& branchPairs);
std::vector<Bit> tracebackDecode(Stage* finalStage);

#endif /* VITDEC_HH_ */
