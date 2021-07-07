//
// Created by dewe on 7/5/21.
//

#include <api/float.h>
#include "rewards.h"
#include "wallets/portfolio.h"
#include "../../feed/core/feed.h"
#include "generic/environment.h"
#include "../../feed/core/base.h"
namespace ttc
{
    float TensorTradeRewardScheme::reward(TradingEnv *env) {
        return getReward(env->actionScheme()->portfolio());
    }

    float SimpleProfit::getReward(Portfolio *portfolio) {
        //todo: optimization - why calculate all net worth and only use window size amount

       auto performance = portfolio->performance();
       net_worth.resize(performance.size());
       returns.resize(performance.size());

       std::transform(performance.begin(), performance.end(), net_worth.begin(),
                      [](std::pair<int, unordered_map<string, string>> const& dict)
       {
           return std::stof(dict.second.at("net_worth"));
       });

        std::transform(net_worth.begin(), net_worth.end()-1,
                       net_worth.begin()+1, returns.begin(), [](float const& a, float const& b)
        {
            return ((b-a)/a) + 1;
        });

        std::vector<float> trunc_return(returns.end() -
        std::min<size_t>(returns.size(), size_t(window_size)), returns.end());

        float prod=1;
        for(auto const& element: trunc_return) prod *= element;
        prod -= 1;

        return returns.empty() ? 0 : returns.back();

    }

    float RiskAdjustedReturns::getReward(Portfolio *portfolio) {

        auto performance = portfolio->performance();
        net_worth.resize(performance.size());
        returns.resize(performance.size());
        std::transform(performance.begin(), performance.end(), net_worth.begin(),[](auto const& dict)
        {
            return std::stod(dict.second.at("net_worth"));
        });

        std::transform(net_worth.begin(), net_worth.end()-1,
                       net_worth.begin()+1, returns.begin(), [](auto const& a, auto const& b)
                       {
                           return b-a;
                       });

        std::vector<float> trunc_return(returns.end() - std::min<size_t>(returns.size(), size_t(window_size + 1)),
                returns.end());

        returns = trunc_return;

        return return_algorithm == PerformanceMetric::SHARPE ? sharpe_ratio() : sortino_ratio();

    }


    PBR::PBR(IterableStream<float> *_price):
    position(-1),
    price(sensor<IterableStream<float>, float>(_price, [](auto* s) { return std::get<float>(s->Value()); } )),
    position_sensor(sensor<PBR, float>(this, [](PBR* pbr){ return pbr->position; })),
    reward(ttc::mul(position_sensor, this->price))
    {

    }

    void PBR::onAction(int action) {
        position = action == 0 ? -1 : 1;
    }

    float PBR::getReward(struct Portfolio *portfolio) {
        reward->run();
        return position * std::get<float>(reward->Value());
    }

    void PBR::reset() {
        position = -1;
        reward->reset();
    }

}
