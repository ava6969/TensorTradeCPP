//
// Created by dewe on 7/5/21.
//

#include "tensortrade/env/default/rewards.h"
#include "tensortrade/oms/wallets/portfolio.h"
#include "tensortrade/env/generic/environment.h"
#include "tensortrade/feed/core/base.h"
#include "tensortrade/core/context.h"

namespace ttc
{
    double TensorTradeRewardScheme::reward(TradingEnv *env) {
        return getReward(env->actionScheme()->portfolio());
    }

    double SimpleProfit::getReward(Portfolio const& portfolio) {

        const std::map<int, std::unordered_map<std::string, double>> & performances = portfolio.performance();
        assert(performances.size() > 1);

        auto net_worth_vector = SimpleProfit::get_pct_change(performances, window_size);
        returns.resize(window_size);

        double prod=1;
        std::transform(net_worth_vector.begin(), net_worth_vector.end() - 1,
                       net_worth_vector.begin() + 1, returns.begin(), [&prod](double const& b, double const& a)
                       {
                           auto&& ret = ((b-a)/a) + 1;
                           prod *= ret;
                           return ret;
                       });

        prod -= 1;
        return returns.empty() ? 0 : prod;

    }

    double RiskAdjustedReturns::getReward(Portfolio const& portfolio) {

        const std::map<int, std::unordered_map<std::string, double>>& performance = portfolio.performance();

        returns.resize(window_size);

        auto net_worth_vector = SimpleProfit::get_pct_change(performance, window_size);

        std::transform(net_worth_vector.begin(), net_worth_vector.end() - 1,
                       net_worth_vector.begin() + 1, returns.begin(), [](auto const& b, auto const& a)
                       {
                           return (b-a)/a;
                       });

        return return_algorithm == PerformanceMetric::SHARPE ? sharpe_ratio() : sortino_ratio();

    }


    PBR::PBR(Float64Stream _price):
    iter_price(std::move(_price)),
    position(-1),
    prev_return(std::nullopt)
    {

    }

    void PBR::onAction(int action) {
        position = action == 0 ? -1 : 1;
    }

    double PBR::getReward(struct Portfolio const& portfolio) {
        iter_price->forward();
        auto returns = iter_price->value();
        if(prev_return)
        {
            returns -= prev_return.value();
        }else
        {
            prev_return = returns;
            returns = 0;
        }
        auto _reward = returns * position;

        return _reward;
    }

    void PBR::reset() {

        position = -1;
        iter_price->reset();
    }

}
