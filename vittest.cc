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
  const int numStages = sizeof(outBits)/sizeof(outBits[0]);

  vector<Bit> decoded = viterbiDecode(outBits, trellis, numStages);
  Bit expected[] = {1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1};

  for (size_type i = 0; i < decoded.size(); i++)
    cout << expected[i] << ' ';

  cout << endl;

  for (size_type i = 0; i < decoded.size(); i++)
    cout << decoded[i] << ' ';

  cout.flush();

  for (size_type i = 0; i < decoded.size(); i++)
    assert(expected[i] == decoded[i]);

}
