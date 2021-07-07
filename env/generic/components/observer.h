//
// Created by dewe on 7/5/21.
//

#ifndef TENSORTRADECPP_OBSERVER_H
#define TENSORTRADECPP_OBSERVER_H


#include "../../core/base.h"

namespace ttc
{
    template<typename T>
    class Observer : public TimeIndexed
    {

    public:
        virtual std::vector<int> observationSpace() = 0;
        virtual T observe(class TradingEnv* env) = 0;
        virtual void reset() {}
        virtual bool hasNext() = 0;
    };

}


#endif //TENSORTRADECPP_OBSERVER_H
