//
// Created by dewe on 7/5/21.
//

#ifndef TENSORTRADECPP_OBSERVERS_H
#define TENSORTRADECPP_OBSERVERS_H


#include <map>
#include <generic/components/observer.h>
#include <api/generic.h>
#include "api/float.h"
#include "torch/torch.h"
#include "vector"
#include "../../feed/core/base.h"
#include "wallets/portfolio.h"
#include "wallets/wallet.h"
#include "exchanges/exchange.h"
#include "core/feed.h"

namespace ttc {

    static std::vector<FloatStream> create_wallet_source(Wallet* wallet, bool include_worth=true)
    {
        auto exchange_name = wallet->exchange()->Name();
        auto symbol = wallet->instrument().symbol;
        std::vector<FloatStream> streams;

        NameSpace nameSpace(exchange_name + ":/" + symbol);
        streams.push_back(sensor<Wallet, float>(wallet,[](Wallet* w) { return w->Balance().asFloat();} ));
        streams.back()->rename<Stream<float>>("free");

        streams.push_back(sensor<Wallet, float >(wallet, [](Wallet *w){ return w->lockedBalance().asFloat();}));
        streams.back()->rename<Stream<float>>("locked");

        streams.push_back(sensor<Wallet, float >(wallet, [](Wallet *w){ return w->totalBalance().asFloat(); }));
        streams.back()->rename<Stream<float>>("total");

        if (include_worth)
        {
            auto price = ttc::select<float>(wallet->exchange()->streams(),
                                [&symbol]( auto const& node) -> bool { return node->Name().ends_with(symbol); });
            streams.push_back(mul(price, streams.back()));
            streams.back()->rename<Stream<float>>("worth");
        }
        return std::move(streams);
    }

    static std::vector<FloatStream>  create_internal_streams(Portfolio* portfolio)
    {
        auto base_symbol = portfolio->baseInstrument().symbol;
        std::vector<FloatStream> sources;

        for(auto & wallet : portfolio->wallets())
        {
            auto symbol = wallet.instrument().symbol;

            auto streams = wallet.exchange()->streams();
            sources.insert(sources.end(), streams.begin(), streams.end());

            auto wallet_sources = create_wallet_source(&wallet, symbol != base_symbol);
            sources.insert(sources.end(), wallet_sources.begin(), wallet_sources.end());
        }
        vector<FloatStream> worthStreams = {};
        for(auto const& s : sources)
        {
            if(s->Name().ends_with(base_symbol + ":/total") or s->Name().ends_with("worth"))
            {
                worthStreams.push_back(s);
            }
        }

        Reduce<float> reducer(worthStreams);
        sources.push_back(reducer.sum());
        sources.back()->rename<Stream<float>>("net_worth");

        return sources;
    }

    class ObservationHistory {

        int window_size;
        std::vector<torch::Tensor> rows;
        int index;
        vector<int64_t> m_observation_shape;
    public:
        ObservationHistory()=default;
        ObservationHistory(int window_size, vector<int64_t> const& observation_shape) : window_size(window_size) {
            m_observation_shape = observation_shape;
            rows.resize(window_size, torch::zeros(observation_shape, torch::kFloat32) );
        }

        void push(torch::Tensor const &row) {
            if (index >= window_size) {
                std::rotate(rows.begin(), rows.begin() + 1, rows.end());
                rows[0] = row;
            }
            rows[index++] = row;
        }

        torch::Tensor observe() {
            return torch::stack(rows, 0);
        }

        void reset() {
            rows.resize(window_size, torch::zeros(m_observation_shape, torch::kFloat32));
            index = 0;
        }
    };

    class TensorTradeObserver : public Observer<torch::Tensor> {

        std::vector<int> dshape;
        std::unique_ptr<class DataFeed> feed;
        ObservationHistory history;
        int window_size;
        std::optional<int> min_periods;


    public:

        TensorTradeObserver(class Portfolio *portfolio,
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

        torch::Tensor tensorView(unordered_map<string, float> const& inp);

        void reset();
    };

}

#endif //TENSORTRADECPP_OBSERVERS_H
