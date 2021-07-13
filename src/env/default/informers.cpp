//
// Created by dewe on 7/5/21.
//
#include "tensortrade/env/default/informers.h"
#include "tensortrade/env/generic/environment.h"
#include "tensortrade/oms/wallets/portfolio.h"

namespace ttc

{
    std::unordered_map<std::string, float> ttc::TTInformer::info(TradingEnv *env) const {
        return {{"step", _clock->Step() },
                {"net_worth", env->actionScheme()->portfolio().netWorth()}};
    }

}

