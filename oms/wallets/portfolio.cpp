//
// Created by dewe on 7/1/21.
//

#include <exchanges/exchange.h>
#include "instruments/exchange_pair.h"
#include "portfolio.h"
#include "wallet.h"
#include "ledger.h"

namespace ttc
{
    Portfolio::Portfolio(const ttc::Instrument &instrument,
                         const Wallets &wallets,
                         OrderListener *order_listener,
                         const std::optional<PerformanceListener> &performance_listener):
                         performance_listener(performance_listener),
                         order_listener(order_listener),
                         base_instrument(instrument),
                         initial_balance(baseBalance())
                         {
        for(auto const& wallet: wallets)
            add(wallet);
    }

    std::vector<Exchange*> Portfolio::exchanges() {
        std::vector<Exchange*> e;

        for(auto& [k, v] : _wallets)
        {
            auto exchange_ = v.exchange();
            if(std::find(e.begin(), e.end(), exchange_) == e.end())
                e.push_back(exchange_);
        }
        return e;
    }

    Wallet* Portfolio::getWallet(const string &exchangeID, const Instrument &instrument)  {
        std::pair<string, string> p{exchangeID, instrument.symbol};
        return &_wallets.at(p);
    }

    void Portfolio::add(const Wallet &wallet) {
        _wallets.insert_or_assign({wallet.exchange()->ID(), wallet.instrument().symbol}, wallet);
    }

    void Portfolio::remove(const Wallet &wallet) {
        _wallets.erase({wallet.exchange()->ID(), wallet.instrument().symbol});
    }

    void Portfolio::add(const std::tuple<Exchange *, Instrument, int> &wallet) {
        return add(Wallet(wallet));
    }

    void Portfolio::remove(const std::tuple<Exchange *, Instrument, int> &wallet) {
        return remove(Wallet(wallet));
    }

    std::vector<string> Portfolio::removePair(const Exchange &exchange, const Instrument &instrument) {
        return std::vector<string>();
    }

    void Portfolio::reset() {

        initial_balance = baseBalance();
        initial_net_worth = 0;
        net_worth = 0;
        m_performance.clear();

        ledger().reset();
        for(auto& wallet : _wallets)
            wallet.second.reset();
    }

    std::vector<string> Portfolio::findKeys(std::unordered_map<string, float>) {
        return std::vector<string>();
    }

    Wallets Portfolio::wallets() const
    {
        Wallets w;
        std::transform(_wallets.begin(), _wallets.end(), std::back_inserter(w),[](auto const& e) { return e.second;});
        return w;
    }

    Quantity<float> Portfolio::balance(const Instrument &instrument) const
    {
        auto bal = Quantity{instrument, 0.f};
        for(auto const&[str_, w] : _wallets)
        {
            if(str_.second == instrument.symbol)
                bal += w.Balance();
        }
        return bal;
    }

    Quantities Portfolio::balances() const {
        Quantities b;
        std::transform(_wallets.begin(), _wallets.end(), std::back_inserter(b), [](auto const& w){
            return w.second.Balance();
        });
        return b;
    }

    std::vector<Quantity<int>> Portfolio::lockedBalances() const {
        vector<Quantity<int>> b;
        std::transform(_wallets.begin(), _wallets.end(), std::back_inserter(b), [](auto const& w){
            return w.second.lockedBalance();
        });
        return b;
    }

    Quantities Portfolio::totalBalances() const {
        Quantities b;
        std::transform(_wallets.begin(), _wallets.end(), std::back_inserter(b), [](auto const& w){
            return w.second.totalBalance();
        });
        return b;
    }

    Ledger Portfolio::ledger() {
        return Wallet::ledger();
    }

    std::vector<ExchangePair> Portfolio::exchangePairs() {
       vector<ExchangePair> exchange_pairs;

       for(auto const&[k, v] : _wallets)
       {
           if(v.instrument() != base_instrument)
               exchange_pairs.emplace_back(v.exchange(), base_instrument/v.instrument());
       }

        return exchange_pairs;
    }

    Quantity<float> Portfolio::lockedBalance(const Instrument &instrument) const  {
        auto bal = Quantity{instrument, 0.f};
        for(auto const&[str_, w] : _wallets)
        {
            if(str_.second == instrument.symbol)
                bal += w.lockedBalance().as<float>();
        }
        return bal;
    }

    Quantity<float> Portfolio::totalBalance(const Instrument &instrument) const  {
        auto bal = Quantity{instrument, 0.f};
        for(auto const&[str_, w] : _wallets)
        {
            if(str_.second == instrument.symbol)
                bal += w.totalBalance().as<float>();
        }
        return bal;
    }

    void Portfolio::onNext(const std::unordered_map<string, std::unordered_map<string, float>> &_data) {

        auto data = _data.at("internal");
        if(keys.empty())
            keys = findKeys(data);

        auto index = _clock.Step();
        std::unordered_map<string, string>  performance_data;
        for(auto const& k : keys)
            performance_data[k] = data.at(k);


        performance_data["base_symbol"] = base_instrument.symbol;
        std::map<int, std::unordered_map<string, string> > performance_step;

        performance_step[index] = performance_data;

        float _net_worth = data["net_worth"];

        if( m_performance.empty())
        {
            m_performance = performance_step;
            initial_net_worth = _net_worth;
            net_worth = _net_worth;
        }else{
            m_performance.merge(performance_step);
            net_worth = _net_worth;
        }

        if(performance_listener)
            performance_listener.value()(performance_step);

    }

}