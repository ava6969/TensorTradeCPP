//
// Created by dewe on 7/5/21.
//

#ifndef TENSORTRADECPP_REWARDS_H
#define TENSORTRADECPP_REWARDS_H

#include <numeric>
#include <tensortrade/feed/core/feed.h>
#include "tensortrade/feed/core/base.h"
#include "tensortrade/env/generic/components/reward_scheme.h"
#include "tensortrade/env/generic/components/action_listener.h"
#include "ranges"

namespace ttc
{

    enum class PerformanceMetric
    {
        SHARPE,
        SORTINO
    };

    class TensorTradeRewardScheme : public RewardScheme{

    public:

        double reward(class TradingEnv *env) override;
        virtual double getReward(class Portfolio const& portfolio) = 0;

    };

    class SimpleProfit : public TensorTradeRewardScheme
    {
        int window_size;
        std::vector<float>  returns;

    public:
        SimpleProfit(int window_size=1):window_size(window_size){}
        double getReward(class Portfolio const& portfolio) override;

        static inline std::vector<double>
                get_pct_change(std::map<int, std::unordered_map<std::string, double>> const & performances,
                               int window_size)
        {
            std::vector<double> net_worth_vector;

            for( auto const& data : performances |
                                    std::views::reverse |
                                    std::views::values  |
                                    std::views::transform([](auto const& x) { return x.at("net_worth"); }) |
                                    std::views::take(window_size + 1))
            {
                net_worth_vector.push_back(data);
            }
            return net_worth_vector;
        }
    };

    class RiskAdjustedReturns : public TensorTradeRewardScheme
    {
        int window_size;
        PerformanceMetric return_algorithm;
        float risk_free_rate, target_returns;
        std::vector<float> net_worth, returns;

        inline double mean() {
            assert(!returns.empty());
            return std::accumulate(returns.begin(), returns.end(), 0.f) / (returns.size());
        }

        inline double mean(std::vector<double> const& ret) {
            assert(!ret.empty());
            return std::accumulate(ret.begin(), ret.end(), 0) / float(ret.size());
        }

        inline double _std(double _mean) {
            std::vector<double> diff(returns.size());
            std::transform(returns.begin(), returns.end(), diff.begin(), [_mean](double x) { return x - _mean; });
            double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
            return std::sqrt(sq_sum / float(returns.size()));
        }

        inline double _std(double _mean, std::vector<double> const& ret) {
            std::vector<double> diff(ret.size());
            std::transform(ret.begin(), ret.end(), diff.begin(), [_mean](double x) { return x - _mean; });
            double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
            return std::sqrt(sq_sum / ret.size());
        }

        inline double sharpe_ratio()
        {
            auto&& _m = mean();
            float&& num = _m - risk_free_rate + 1e-9f;
            float&& dem = _std(_m) + 1e-9f;
            return std::move(num / dem);
        }



        inline double sortino_ratio()
        {
            auto original_return = returns;
            auto downside_returns = returns;
            std::transform(downside_returns.begin(), downside_returns.end(), downside_returns.begin(),
                           [this](auto  const& ret) { return ret < target_returns ? ret*ret : ret; });
            auto expected_return = mean();
            returns = downside_returns;

            auto downside_std = std::sqrt(_std(mean()));
            returns = original_return;
            return (expected_return - risk_free_rate + 1e-9f) / (downside_std + 1e-9f);

        }

    public:
        RiskAdjustedReturns(PerformanceMetric return_algorithm =PerformanceMetric::SHARPE,
                            float risk_free_rate=0, float target_returns=0, int window_size=1):
                            window_size(window_size),
                            risk_free_rate(risk_free_rate),
                            return_algorithm(return_algorithm),
                            target_returns(target_returns)
                            {}

        double getReward(class Portfolio const& portfolio) override;

        inline double sharpe_ratio(std::vector<double> const& ret)
        {
            assert(ret.size() > 1);
            auto _mean = mean(ret);
            return (_mean - risk_free_rate + 1e-9f) / (_std(_mean, ret) + 1e-9f);
        }

        inline double sortino_ratio(std::vector<double>  ret)
        {
            auto original_return = ret;
            auto downside_returns = ret;
            std::transform(downside_returns.begin(), downside_returns.end(), downside_returns.begin(),
                           [this](auto  const& ret) { return ret < target_returns ? ret*ret : ret; });
            auto expected_return = mean(original_return);
            ret = downside_returns;

            auto downside_std = std::sqrt(_std(mean(downside_returns), downside_returns));
            ret = original_return;
            return (expected_return - risk_free_rate + 1e-9f) / (downside_std + 1e-9f);

        }
    };

    class PBR: public TensorTradeRewardScheme, public ActionListener<int>
    {
        float position;
        std::optional<float>  prev_return;
        Float64Stream iter_price;

    public:

        explicit PBR(Float64Stream _price);

        void onAction(int action) override;

        double getReward(class Portfolio const& portfolio) override;

        void reset() override;

    };

}


#endif //TENSORTRADECPP_REWARDS_H
