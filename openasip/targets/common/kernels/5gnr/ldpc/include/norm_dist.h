#pragma once

#include "normal_samples.h"
#include "ldpc_numeric.h"

class NormDistRandomEngine {
private:
    size_t index_;

public:
    NormDistRandomEngine() : index_(0) {}

    float operator()()
    {
        const float value = NORMAL_SAMPLES[index_];

        ++index_;

        if (index_ == NORMAL_SAMPLES_SIZE)
            index_ = 0;

        return value;
    }
};
