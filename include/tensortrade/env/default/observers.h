//
// Created by dewe on 7/5/21.
//

#ifndef TENSORTRADECPP_OBSERVERS_H
#define TENSORTRADECPP_OBSERVERS_H


#include <map>
#include <tensortrade/env/generic/components/observer.h>
#include <tensortrade/feed/api/generic.h>
#include "tensortrade/feed/api/float.h"
#include "torch/torch.h"
#include "vector"
#include "tensortrade/feed/core/base.h"
#include "tensortrade/oms/wallets/portfolio.h"
#include "tensortrade/oms/wallets/wallet.h"
#include "tensortrade/oms/exchanges/exchange.h"
#include "tensortrade/feed/core/feed.h"
#include <boost/algorithm/string.hpp>
namespace ttc {

    static std::vector<Float64Stream> create_wallet_source(Wallet const& wallet, bool include_worth=true)
    {
        auto exchange_name = wallet.exchange().Name();
        auto symbol = wallet.instrument().symbol();
        std::vector<Float64Stream> streams;

        NameSpace nameSpace(exchange_name + ":/" + symbol);
        streams.push_back(sensor<Wallet>(wallet,[](Wallet const& w) { return w.Balance().asDouble();} ));
        streams.back()->rename<Stream>("free");

        streams.push_back(sensor<Wallet>(wallet, [](Wallet const& w){ return w.lockedBalance().asDouble();}));
        streams.back()->rename<Stream>("locked");

        streams.push_back(sensor<Wallet >(wallet, [](Wallet const& w){ return w.totalBalance().asDouble(); }));
        streams.back()->rename<Stream>("total");

        if (include_worth)
        {
            auto price = ttc::select(wallet.exchange().streams(), [&symbol]( auto const& node) -> bool {
                return node->Name().ends_with(symbol) or node->Name().find(symbol) != string::npos; });
            streams.push_back(mul(price , streams.back() ));
            streams.back()->rename<Stream>("worth");
        }
        return streams;
    }

    static std::vector<Float64Stream>  create_internal_streams(Portfolio* portfolio)
    {

        auto base_symbol = portfolio->baseInstrument().symbol();
        std::vector<Float64Stream> sources;

        for(auto const& wallet_id : portfolio->wallets())
        {
            auto const& wallet = Wallet::wallet(wallet_id);
            auto symbol = wallet.instrument().symbol();

            auto streams = wallet.exchange().streams();
            sources.insert(sources.end(), streams.begin(), streams.end());

            auto wallet_sources = create_wallet_source(wallet, symbol != base_symbol);
            sources.insert(sources.end(), wallet_sources.begin(), wallet_sources.end());
        }
        vector<Float64Stream> worthStreams = {};
        for(auto const& s : sources)
        {
            if( (s->Name().find(base_symbol) != std::string::npos &&  s->Name().ends_with(":/total")) or
            s->Name().ends_with("worth"))
            {
                worthStreams.push_back(s);
            }
        }

        Reduce reducer(worthStreams);
        sources.push_back(reducer.sum());
        sources.back()->rename<Stream>("net_worth");

        return sources;
    }

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
        std::unique_ptr<class DataFeed> feed;
        ObservationHistory history;
        int window_size;
        std::optional<int> min_periods;


    public:

        explicit TensorTradeObserver(class Portfolio *portfolio,
                class DataFeed* dataFeed= nullptr,
                DataFeed* rendererFeed= nullptr,
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

        void reset();
    };

}

#endif //TENSORTRADECPP_OBSERVERS_H
