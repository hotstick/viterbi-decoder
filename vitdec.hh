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

#include <list>

typedef bool Bit;          // 0 = false, 1 = true
typedef unsigned Metric;
typedef unsigned Node;

class Branch
{
public:
  Branch(Node from, Node to, Bit bit, Metric metric)
  : mFrom(from)
  , mTo(to)
  , mBit(bit)
  , mMetric(metric)
  { /* empty */ }

private:
  Branch();
  Node mFrom;       // left node of the edge
  Node mTo;         // right node of the edge
  Bit mBit;         // input bit
  Metric mMetric;   // output bits
};

class Stage
{
public:
  Stage(const std::list<Branch>& survivors,
        const std::list<Metric>& totalMetrics,
        const Stage& previousStage
  )
  : mSurvivors(survivors)
  , mTotalMetrics(totalMetrics)
  , mPrevious(previousStage)
  { /* empty */ }

  // Getters and Setters
  const Stage& previousStage()  {  return mPrevious; }

private:
  Stage();
  std::list<Branch> mSurvivors;
  std::list<Metric> mTotalMetrics;
  const Stage& mPrevious;
};

#endif /* VITDEC_HH_ */
