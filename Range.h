// Range.h
#ifndef RANGE_H
#define RANGE_H

struct Range
{
    Range(int lo, int hi) : mLow(lo), mHigh(hi) {}
    Range(const Range& r) : mLow(r.mLow), mHigh(r.mHigh) {} // Copy constructor

    int mLow;
    int mHigh;
};

#endif //Range.h