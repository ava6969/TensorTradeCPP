//
// Created by dewe on 7/5/21.
//

#ifndef TENSORTRADECPP_ACTION_SCHEME_H
#define TENSORTRADECPP_ACTION_SCHEME_H

#include "../../core/base.h"

namespace ttc
{
    template<typename T>
    class ActionScheme : public TimeIndexed
    {

    public:
        virtual class std::vector<int> actionShape() = 0;
        virtual void perform(class TradingEnv* env, T const& action) = 0;
        virtual void reset() {};

        virtual class Portfolio* portfolio() const = 0;
    };

}



#endif //TENSORTRADECPP_ACTION_SCHEME_H
