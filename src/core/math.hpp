#ifndef MATH_HPP
#define MATH_HPP

inline int floorDiv(int value, int divisor)
{
  int q = value / divisor;
  int r = value % divisor;

  if (r < 0)
    --q;

  return q;
}

inline int floorMod(int value, int divisor)
{
  int r = value % divisor;

  if (r < 0)
    r += divisor;

  return r;
}

#endif // MATH_HPP
