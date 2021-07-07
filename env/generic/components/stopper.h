//
// Created by dewe on 7/5/21.
//

#ifndef TENSORTRADECPP_STOPPER_H
#define TENSORTRADECPP_STOPPER_H
#include "../../core/base.h"

namespace ttc
{
    class Stopper : public TimeIndexed
    {

    public:

        virtual bool stop(class TradingEnv* env) = 0;
        virtual void reset() {};
    };

}
#endif //TENSORTRADECPP_STOPPER_H
