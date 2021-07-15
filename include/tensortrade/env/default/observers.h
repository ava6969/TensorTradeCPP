//
// Created by dewe on 7/5/21.
//

#ifndef TENSORTRADECPP_OBSERVERS_H
#define TENSORTRADECPP_OBSERVERS_H

#include <map>
#include <tensortrade/env/generic/components/observer.h>
#include "tensortrade/core/context.h"
#include "torch/torch.h"
#include "vector"
#include "tensortrade/feed/core/base.h"
#include "tensortrade/oms/wallets/portfolio.h"
#include "tensortrade/oms/wallets/wallet.h"
#include "tensortrade/oms/exchanges/exchange.h"
#include "tensortrade/feed/core/feed.h"
#include <boost/algorithm/string.hpp>
namespace ttc {


    class ObservationHistory {

        int window_size;
        std::vector<torch::Tensor> rows;
        vector<int64_t> m_observation_shape;
    public:
        ObservationHistory()=default;
        explicit ObservationHistory(int window_size, vector<int64_t> const& observation_shape) : window_size(window_size) {
            m_observation_shape = observation_shape;
            rows.resize(window_size, torch::zeros(observation_shape, torch::kFloat32) );
        }

        void push(torch::Tensor const &row) {
            std::rotate(rows.rbegin(), rows.rbegin() + 1, rows.rend());
            rows[0] = row;
        }

        torch::Tensor observe() {
            return torch::stack(rows, 0);
        }

        void reset() {
            rows = std::vector<torch::Tensor>(window_size, torch::zeros(m_observation_shape, torch::kFloat32));
        }
    };

    class TensorTradeObserver : public Observer<torch::Tensor> {

        std::vector<int> dshape;
        std::unique_ptr<DataFeed<double>> feed;
        ObservationHistory history;
        int window_size;
        std::optional<int> min_periods;


    public:

        explicit TensorTradeObserver(
                class Portfolio *portfolio,
                BaseDataFeed<double>* dataFeed= nullptr,
                BaseDataFeed<double>* rendererFeed= nullptr,
                int window_size=1,
                std::optional<int> min_periods=std::nullopt);

        std::vector<int> observationSpace() override
        {
            return dshape;
        }

        void warmup();

        torch::Tensor observe(class TradingEnv *env) override;

        bool hasNext() override;

        torch::Tensor tensorView(unordered_map<string, double> const& inp);

        void reset() override;

        static std::vector<Float64Stream> create_wallet_source(Wallet const& wallet, bool include_worth=true);

        static std::vector<Float64Stream>  create_internal_streams(Portfolio* portfolio);

    };

}

#endif //TENSORTRADECPP_OBSERVERS_H
