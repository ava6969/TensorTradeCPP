//
// Created by dewe on 7/9/21.
//

#ifndef TENSORTRADECPP_CONTEXT_H
#define TENSORTRADECPP_CONTEXT_H

#include <iostream>
#include "pybind11/embed.h"
#include "string"

#include <tensortrade/oms/exchanges/exchange.h>
#include "tensortrade/oms/wallets/wallet.h"
#include <tensortrade/feed/core/feed.h>
#include "tensortrade/env/default/actions.h"
#include "tensortrade/env/default/observers.h"
#include "tensortrade/env/default/informers.h"

namespace py = pybind11;
using namespace pybind11::literals;
using std::vector;
using std::string;

namespace ttc {

    struct TickerSpec
    {
        string ticker, market_type;
    };

    struct TechIndicatorSpec
    {
        py::list tech_indicators_spec;

        template<class ... Args>
        void add(string const& name, py::dict const& dict)
        {
            tech_indicators_spec.template append(std::make_tuple(name, dict));
        }
    };

    struct ExchangeSpec
    {
        string name;
        TechIndicatorSpec features_param;
        vector<string> features;
        vector<TickerSpec> ticker_specs;
        vector<Quantity> qty;
    };

    class Ctx
    {

    private:

        std::vector<double> data(py::object const& wrapper, std::string const& column)
        {
            std::vector<double> result;
            auto d_list = wrapper.attr("get_series")(column).cast<py::list>();
            std::transform(d_list.begin(), d_list.end(), std::back_inserter(result), [](auto& f)
            { return f.template cast<double>(); });
            return result;
        }

        std::vector<DataFeed> feeds;
        std::unordered_map<string, Exchange> m_exchanges;
        std::unordered_map<string, Wallet> m_wallets;
        std::unique_ptr<class Ledger> m_ledger;

        explicit Ctx();

    private:

        static Ctx* m_instance;
        py::module_ make_module;

    public:
        static std::string sys_path;
        static auto* ctx()
        {
            if(m_instance) return m_instance;
            m_instance = new Ctx();
            return m_instance;
        }

        std::tuple<Portfolio, Broker, Wallet*, Wallet*, DataFeed*> build(vector<ExchangeSpec> const& exchange_specs);

        void registerExchange(Exchange&& exchange) { m_exchanges[exchange.ID()] = exchange; }

        void registerWallet(Wallet const& wallet) { m_wallets.insert_or_assign(wallet.ID(), wallet); }

        inline bool isPairTradable(string const& exchange_name, TradingPair const& pair_){
            return m_exchanges[exchange_name].isPairTradable(pair_);
        }

        inline Exchange&    exchanges(std::string const& exchange_id)    { return m_exchanges.at(exchange_id); }

        inline Wallet&      wallet(std::string const& wallet_id)           { return m_wallets.at(wallet_id); }

        Ledger*      ledger();

        ~Ctx();

    };

};


#endif //TENSORTRADECPP_CONTEXT_H
