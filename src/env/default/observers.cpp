//
// Created by dewe on 7/5/21.
//

#include "tensortrade/env/default/observers.h"

namespace ttc
{

    TensorTradeObserver::TensorTradeObserver(Portfolio *portfolio,
                                             BaseDataFeed<double> *dataFeed,
                                             BaseDataFeed<double> *rendererFeed,
                                             int window_size,
                                             std::optional<int> min_periods){

        auto internal_group = Ctx::ctx()->group(create_internal_streams(portfolio))->rename("internal");
        auto external_group = Ctx::ctx()->group(dataFeed->inputs())->rename("external");

        if(rendererFeed)
        {

        }else
        {
            feed = std::make_unique<DataFeed<double > >(std::vector<Stream<double>*>{internal_group, external_group});
        }

        this->window_size = window_size;
        this->min_periods = min_periods;

        auto initial_obs = feed->mmm_next().at("external");
        auto n_feature = initial_obs.size();

        dshape = {window_size, static_cast<int>(n_feature)};
        history = ObservationHistory(window_size, {static_cast<long>(n_feature)});
        feed->attach(portfolio);

        feed->reset();
        warmup();


    }

    void TensorTradeObserver::warmup() {

        if(min_periods)
        {
            int i = 0;
            while (i++ < min_periods.value())
            {
                if(hasNext())
                {
//                    history.push(tensorView(feed->next().at("external"))); // view of values
                }
            }
        }
    }

    torch::Tensor TensorTradeObserver::observe(struct TradingEnv *env) {

        auto data = feed->mmm_next()["external"];

        history.push(tensorView(data));

        return history.observe();

    }

    bool TensorTradeObserver::hasNext() {
        return feed->has_next();
    }

    void TensorTradeObserver::reset() {
        history.reset();
        feed->reset();
        warmup();
    }

    torch::Tensor TensorTradeObserver::tensorView(const unordered_map<string, double> &inp) {
        vector<double> values(inp.size());
        std::transform(inp.begin(), inp.end(), values.begin(),[](auto const& entry){
           return entry.second;
        });
        return torch::tensor(values);
    }

    std::vector<Float64Stream> TensorTradeObserver::create_internal_streams(Portfolio *portfolio)   {

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
            if( s->Name().ends_with(base_symbol + ":/total") or s->Name().ends_with("worth"))
            {
                worthStreams.push_back(s);
            }
        }

        sources.push_back(Ctx::ctx()->sum(worthStreams)->rename("net_worth"));

        return sources;
    }

    std::vector<Float64Stream> TensorTradeObserver::create_wallet_source(const Wallet &wallet, bool include_worth)
    {
        auto exchange_name = wallet.exchange().Name();
        auto symbol = wallet.instrument().symbol();
        std::vector<Float64Stream> streams;

        NameSpace nameSpace(exchange_name + ":/" + symbol);

        streams.push_back(Ctx::ctx()->sensor<Wallet>(wallet,
                                                     [](Wallet const& w)
                                                     { return w.Balance().asDouble();})->rename("free"));

        streams.push_back(Ctx::ctx()->sensor<Wallet>(wallet,
                                                     [](Wallet const& w)
                                                     { return w.lockedBalance().asDouble();})->rename("locked"));

        streams.push_back(Ctx::ctx()->sensor<Wallet >(wallet,
                                                      [](Wallet const& w)
                                                      { return w.totalBalance().asDouble(); })->rename("total"));

        if (include_worth)
        {
            std::function<bool(Stream<double>* )> pred = [&symbol]( Stream<double>* node) -> bool
            { return node->Name().ends_with(symbol) or
                     node->Name().find(symbol) != string::npos; };

            auto price = Ctx::ctx()->select<double>( wallet.exchange().streams(), pred);
            streams.push_back(Ctx::ctx()->mul(price, streams.back() )->rename<double>("worth"));
        }
        return streams;
    }

}
