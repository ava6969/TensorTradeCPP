//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_FLOAT_H
#define TENSORTRADECPP_FLOAT_H

#include "../../feed/core/operators.h"
#include "../../feed/core/base.h"
#include "../../feed/api/generic.h"

namespace ttc {

    static FloatStream pow(FloatStream s, float power)
    {
        return apply<float>(s, [power](float x) -> float { return std::pow(x, power); });
    }

    static FloatStream mul(FloatStream s1, FloatStream s2)
    {
        return std::make_shared<BinOp<float>>( [](float x1, float x2) { return x1 * x2; },
                vector<FloatStream>{s1, s2} );
    }



}


#endif //TENSORTRADECPP_FLOAT_H
