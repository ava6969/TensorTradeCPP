//
// Created by dewe on 7/5/21.
//

#ifndef TENSORTRADECPP_REWARDS_H
#define TENSORTRADECPP_REWARDS_H

#include <numeric>
#include <core/feed.h>
#include "../../feed/core/base.h"
#include "generic/components/reward_scheme.h"
#include "generic/components/action_listener.h"

namespace ttc
{

    enum class PerformanceMetric
    {
        SHARPE,
        SORTINO
    };

    class TensorTradeRewardScheme : public RewardScheme{

        float reward(class TradingEnv *env) override;
        virtual float getReward(class Portfolio* portfolio) = 0;

    };

    class SimpleProfit : public TensorTradeRewardScheme
    {
        int window_size;
        std::vector<float> net_worth, returns;
    public:
        SimpleProfit(int window_size=1):window_size(window_size){}
        float getReward(class Portfolio *portfolio) override;
    };

    class RiskAdjustedReturns : public TensorTradeRewardScheme
    {
        int window_size;
        PerformanceMetric return_algorithm;
        float risk_free_rate, target_returns;
        std::vector<float> net_worth, returns;

        inline float mean() {
            assert(returns.size() > 1);
            return float(std::accumulate(returns.begin(), returns.end(), 0)) / returns.size();
        }

        inline double _std(float _mean) {
            std::vector<float> diff(returns.size());
            std::transform(returns.begin(), returns.end(), diff.begin(), [_mean](double x) { return x - _mean; });
            double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
            return std::sqrt(sq_sum / returns.size());
        }

        inline float sharpe_ratio()
        {
            assert(returns.size() > 1);
            auto _mean = mean();
            return (_mean - risk_free_rate + 1e-9) / (_std(_mean) + 1e-9);
        }

        inline float sortino_ratio()
        {
            auto original_return = returns;
            auto downside_returns = returns;
            std::transform(downside_returns.begin(), downside_returns.end(), downside_returns.begin(),
                           [this](auto  const& ret) { return ret < target_returns ? ret*ret : ret; });
            auto expected_return = mean();
            returns = downside_returns;

            auto downside_std = std::sqrt(_std(mean()));
            returns = original_return;
            return (expected_return - risk_free_rate + 1e-9) / (downside_std + 1e-9);

        }

    public:
        RiskAdjustedReturns(PerformanceMetric return_algorithm =PerformanceMetric::SHARPE,
                            float risk_free_rate=0, float target_returns=0, int window_size=1):
                            window_size(window_size),
                            risk_free_rate(risk_free_rate),
                            return_algorithm(return_algorithm),
                            target_returns(target_returns)
                            {}

        float getReward(class Portfolio *portfolio) override;
    };

    class PBR: public TensorTradeRewardScheme, public ActionListener<int>
    {
        float position;
        FloatStream price;
        FloatStream position_sensor;
        FloatStream reward;

    public:

        explicit PBR(class IterableStream<float>* _price);

        void onAction(int action) override;

        float getReward(class Portfolio *portfolio) override;

        void reset() override;

    };

}


#endif //TENSORTRADECPP_REWARDS_H
