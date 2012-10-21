/**
 * @file       vitdec.hh
 * @author     bdsatish
 * @date       Oct 20, 2012
 * @copyright  Copyright (C) 2012, Satish BD, see LICENSE file for details
 * 
 * @brief      API for Viterbi decoder
 * 
 */

#ifndef VITDEC_HH_
#define VITDEC_HH_

#include <vector>
#include <utility>
#include <cassert>   // for assert()

//... Forward Declarations ...
class Trellis;
class Stage;
class Branch;

//... Typedefs ...
typedef unsigned DataType;
typedef bool Bit;            // 0 = false, 1 = true
typedef std::vector<int>::size_type size_type;
typedef unsigned Metric;
typedef unsigned State;
typedef std::pair<State, State> NextState;
typedef std::pair<Metric, Metric> Output;
typedef std::vector< std::vector<Branch> > BranchPairs;

//... Global functions ...
void viterbiDecode(DataType outBits[], Trellis* trellis, size_type numStages, std::vector<Bit>& decoded);

//... Classes ...
//... Class Branch ...
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

//... Class Trellis ...
class Trellis
{
public:
  //... Constructors ...
  Trellis() {}

  //... Methods ...
  void push_back(State state0, State state1, Metric metric0, Metric metric1);
  void build();

  //... Getters and Setters ...
  BranchPairs branchPairs()
  {
    return mBranchPairs;
  }

private:
  std::vector<NextState> mNextStates;
  std::vector<Output> mOutputs;
  BranchPairs mBranchPairs;
};


#endif /* VITDEC_HH_ */
