//
// Created by dewe on 7/5/21.
//

#ifndef TENSORTRADECPP_INFORMER_H
#define TENSORTRADECPP_INFORMER_H


#include "../../core/base.h"

namespace ttc
{
    template<typename T>
    class Informer : public TimeIndexed
    {

    public:

        virtual std::unordered_map<std::string, T> info( class TradingEnv* env) const = 0;
        virtual void reset() {};
    };

}


#endif //TENSORTRADECPP_INFORMER_H
