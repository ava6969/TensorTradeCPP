//
// Created by dewe on 7/1/21.
//

#include <tensortrade/oms/exchanges/exchange.h>
#include "tensortrade/core/context.h"
#include "tensortrade/oms/instruments/exchange_pair.h"
#include "tensortrade/oms/wallets/portfolio.h"
#include "tensortrade/oms/wallets/wallet.h"
#include <regex>
namespace ttc {
    Portfolio::Portfolio(const ttc::Instrument &instrument,
                         std::vector<string> const& wallets,
                         OrderListener *order_listener,
                         const std::optional<PerformanceListener> &performance_listener) :
            performance_listener(performance_listener),
            order_listener(order_listener),
            base_instrument(instrument) {

        for (auto const& wallet: wallets)
            add(wallet);

        initial_balance = baseBalance();
    }

    std::vector<std::string> Portfolio::exchanges() {
        std::vector<std::string> e;

        for (auto&[k, wallet_id] : m_wallet_ids)
        {
            auto& wallet = Ctx::ctx()->wallet(wallet_id);
            auto exchange_ = wallet.exchange_id();
            if (std::find(e.begin(), e.end(), exchange_) == e.end())
                e.push_back(exchange_);
        }
        return e;
    }

    Wallet &Portfolio::getWallet(const string &exchangeID, const Instrument &instrument) {
        std::pair<string, string> p{exchangeID, instrument.symbol()};
        auto& wallet = Ctx::ctx()->wallet(m_wallet_ids.at(p));
        return wallet;
    }

    void Portfolio::add(string const& wallet_id) {
        auto const& wallet = Ctx::ctx()->wallet(wallet_id);
        m_wallet_ids.insert_or_assign({wallet.exchange().ID(), wallet.instrument().symbol()}, wallet_id);
    }

    void Portfolio::remove(string const& wallet_id) {
        auto const& wallet = Ctx::ctx()->wallet(wallet_id);
        m_wallet_ids.erase({wallet.exchange().ID(), wallet.instrument().symbol()});
    }


    std::vector<string> Portfolio::removePair(const std::string &exchange, const Instrument &instrument) {
        return std::vector<string>();
    }

    void Portfolio::reset() {

        initial_balance = baseBalance();
        initial_net_worth = 0;
        net_worth = 0;
        m_performance.clear();

        Ctx::ctx()->ledger()->reset();

        for (auto& wallet_id : m_wallet_ids)
        {
            auto& wallet = Ctx::ctx()->wallet(wallet_id.second);
            wallet.reset();
        }

    }

    std::vector<string> Portfolio::findKeys(const std::unordered_map<string, double>& data) {
        std::vector<string> _keys, endings;

        endings = {":/free", ":/locked", ":/total", "worth"};

        auto pattern = std::regex("\\w+:/([A-Z]{3,4}).([A-Z]{3,4})");

        for (auto const &entry : data) {

            auto it = std::find_if(endings.begin(), endings.end(), [entry](string const &str) {
                return entry.first.ends_with(str);
            });

            if (it == endings.end()) {
                if (std::regex_match(entry.first, pattern)) {
                    _keys.push_back(entry.first);
                }
            } else
                _keys.push_back(entry.first);
        }

        return _keys;

    }

    vector<string> Portfolio::wallets() const {
        vector<string> w;
        std::transform(m_wallet_ids.begin(), m_wallet_ids.end(), std::back_inserter(w), [](auto const &e) {
            return e.second; });
        return w;
    }

    Quantity Portfolio::balance(const Instrument &instrument) const {
        auto bal = Quantity{instrument, 0};
        for (auto const&[key, wallet_id] : m_wallet_ids) {
            auto const& wallet = Wallet::wallet(wallet_id);
            if (key.second == instrument.symbol())
            {
                bal += wallet.Balance();
            }
        }
        return bal;
    }

    Quantities Portfolio::balances() const {
        Quantities b;
        std::transform(m_wallet_ids.begin(), m_wallet_ids.end(), std::back_inserter(b), [](auto const &w) {
            auto& wallet = Wallet::wallet(w.second);
            return wallet.Balance();
        });
        return b;
    }

    std::vector<Quantity> Portfolio::lockedBalances() const {
        vector<Quantity> b;
        std::transform(m_wallet_ids.begin(), m_wallet_ids.end(), std::back_inserter(b), [](auto const &w) {
            auto& wallet = Wallet::wallet(w.second);
            return wallet.lockedBalance();
        });
        return b;
    }

    Quantities Portfolio::totalBalances() const {
        Quantities b;
        std::transform(m_wallet_ids.begin(), m_wallet_ids.end(), std::back_inserter(b), [](auto const &w) {
            auto& wallet = Wallet::wallet(w.second);
            return wallet.totalBalance();
        });
        return b;
    }


    std::vector<ExchangePair> Portfolio::exchangePairs() const {
        vector<ExchangePair> exchange_pairs;

        for (auto const&[k, wallet_id] : m_wallet_ids) {
            auto const& wallet = Wallet::wallet(wallet_id);
            if (wallet.instrument() != base_instrument)
                exchange_pairs.emplace_back(wallet.exchange().Name(), wallet.exchange_id(),
                                            base_instrument / wallet.instrument());
        }

        return exchange_pairs;
    }

    Quantity Portfolio::lockedBalance(const Instrument &instrument) const {
        auto bal = Quantity{instrument, 0.f};
        for (auto const&[str_, wallet_id] : m_wallet_ids) {
            auto& wallet = Wallet::wallet(wallet_id);
            if (str_.second == instrument.symbol())
                bal += wallet.lockedBalance();
        }
        return bal;
    }

    Quantity Portfolio::totalBalance(const Instrument &instrument) const {
        auto bal = Quantity{instrument, 0.f};
        for (auto const&[str_, wallet_id] : m_wallet_ids) {
            auto& wallet = Wallet::wallet(wallet_id);
            if (str_.second == instrument.symbol())
                bal += wallet.totalBalance();
        }
        return bal;
    }

    void Portfolio::onNext(const std::unordered_map<string, std::unordered_map<string, double>> &_data) {

        auto performance_data = _data.at("internal");
        if (keys.empty())
            keys = findKeys(performance_data);

        auto index = _clock->Step();

//        performance_data["base_symbol"] = base_instrument.symbol();
        std::map<int, std::unordered_map<string, double> > performance_step;

        performance_step[index] = performance_data;

        double _net_worth = performance_data["net_worth"];

        if (m_performance.empty()) {
            m_performance = performance_step;
            initial_net_worth = _net_worth;
            net_worth = _net_worth;
        } else {
            m_performance.merge(performance_step);
            net_worth = _net_worth;
        }

        if (performance_listener)
            performance_listener.value()(performance_step);

    }




}