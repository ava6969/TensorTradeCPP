//
// Created by dewe on 7/5/21.
//

#include "stoppers.h"
#include "generic/environment.h"
#include "wallets/portfolio.h"
#include "generic/components/observer.h"

namespace ttc {

    bool MaxLossStopper::stop( TradingEnv *env) {
        auto c1 = env->actionScheme()->portfolio()->profitLoss() > m_max_allowed_loss;
        auto c2 = env->observer()->hasNext();
        return c1 or c2;
    }
}