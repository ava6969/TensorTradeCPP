//
// Created by dewe on 7/5/21.
//

#ifndef TENSORTRADECPP_REWARD_SCHEME_H
#define TENSORTRADECPP_REWARD_SCHEME_H

#include "../../core/base.h"

namespace ttc
{
    class RewardScheme : public TimeIndexed
    {

    public:

        virtual float reward(class TradingEnv* env) = 0;
        virtual void reset() {};
    };

}


#endif //TENSORTRADECPP_REWARD_SCHEME_H
