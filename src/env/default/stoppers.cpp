//
// Created by dewe on 7/5/21.
//

#include "tensortrade/env/default/stoppers.h"
#include "tensortrade/env/generic/environment.h"
#include "tensortrade/oms/wallets/portfolio.h"
#include "tensortrade/env/generic/components/observer.h"

namespace ttc {

    bool MaxLossStopper::stop( TradingEnv *env) {
        auto&& pnl = env->actionScheme()->portfolio().profitLoss();
        auto c1 = pnl > m_max_allowed_loss;
        auto c2 = not env->observer()->hasNext();
        return c1 or c2;
    }
}