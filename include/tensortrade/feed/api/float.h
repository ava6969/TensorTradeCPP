//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_FLOAT_H
#define TENSORTRADECPP_FLOAT_H

#include "tensortrade/feed/core/operators.h"
#include "tensortrade/feed/core/base.h"
#include "generic.h"

namespace ttc {


    static Float64Stream mul(Float64Stream s1, Float64Stream s2)
    {

        return std::make_shared<BinOp>( [](double x1, double x2) {
            return x1 * x2;
            },
                vector<Float64Stream>{s1, s2} );
    }



}


#endif //TENSORTRADECPP_FLOAT_H
