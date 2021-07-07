//
// Created by dewe on 7/5/21.
//
#include "informers.h"
#include "generic/environment.h"
#include "wallets/portfolio.h"

namespace ttc

{
    std::unordered_map<std::string, float> ttc::TTInformer::info(TradingEnv *env) const {
        return {{"step", _clock.Step() },
                {"net_worth", env->actionScheme()->portfolio()->netWorth()}};
    }

}

