//
// Created by dewe on 7/9/21.
//

#ifndef TENSORTRADECPP_CONTEXT_H
#define TENSORTRADECPP_CONTEXT_H

#include <iostream>
#include <utility>
#include "pybind11/embed.h"
#include "string"
#include "tensortrade/feed/core/operators.h"
#include <tensortrade/oms/exchanges/exchange.h>
#include "tensortrade/oms/wallets/wallet.h"
#include <tensortrade/feed/core/feed.h>
#include "tensortrade/env/default/actions.h"

#include "tensortrade/env/default/informers.h"

namespace py = pybind11;
using namespace pybind11::literals;
using std::vector;
using std::string;

namespace ttc {

    struct TickerSpec
    {
        string ticker, market_type;
        bool operator==(const TickerSpec &p) const {
            return ticker == p.ticker and market_type == p.market_type;
        }
    };

    struct TechIndicatorSpec
    {
        py::list tech_indicators_spec;
        std::vector<py::dict> dicts;
        std::vector<string> names;

        TechIndicatorSpec()=default;
        template<class ... Args>
        void add(string const& name, py::dict const& dict)
        {
            names.push_back(name);
            dicts.push_back(dict);
            tech_indicators_spec.template append(std::make_tuple(name, dict));
        }

        bool operator==(const TechIndicatorSpec &p) const
        {
            auto dict_equal = std::equal(dicts.begin(), dicts.end(), p.dicts.begin());
            auto names_equal = std::equal(names.begin(), names.end(), p.names.begin());
            return tech_indicators_spec.equal(p.tech_indicators_spec) and dict_equal and names_equal; // todo: verify
        }

        [[nodiscard]] size_t hash() const
        {
            size_t h3;
            for(auto& f: names) h3 ^= std::hash<string>()(f);
            return h3;
        }

    };

    struct ExchangeSpec
    {
        string name{};
        TechIndicatorSpec features_param{};
        vector<string> features{};
        vector<TickerSpec> ticker_specs{};
        vector<Quantity> qty{};
        float commision{};
        bool skipLog{true};

        ExchangeSpec()=default;
        bool operator==(const ExchangeSpec &p) const = default;
    };

    struct hash_fn
    {

        std::size_t operator() (const ExchangeSpec &node) const
        {
            std::size_t h1 = std::hash<string>()(node.name);
            std::size_t h2 = node.features_param.hash();
            std::size_t h3{};
            for(auto& f: node.features) h3 ^= std::hash<string>()(f);

            return h1 ^ h2 ^ h3;
        }
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

        std::unordered_map<ExchangeSpec, std::pair<vector<Float64Stream>, vector<Float64Stream>>, hash_fn> feature_map;
        std::unordered_map<std::string, Stream<double>*> double_streams;
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

        std::tuple<Portfolio, Broker, Wallet*, Wallet*, BaseDataFeed<double>, Float64Stream>
        build(vector<ExchangeSpec> const& exchange_specs);

        std::string registerExchange(Exchange&& exchange) {
            m_exchanges[exchange.ID()] = exchange;
            return exchange.ID();}

        std::string registerWallet(Wallet const& wallet) {
            m_wallets.insert_or_assign(wallet.ID(), wallet);
            return wallet.ID(); }

        std::pair<std::string, Exchange*> createExchange(std::string exchange_name,
                                   vector<Stream<double>*> const& close_streams){
            Exchange ex{std::move(exchange_name), {}, {}, close_streams};
            auto id = ex.ID();
            m_exchanges[id] = ex;
            return {id, &m_exchanges[id]};
        }

        std::pair<std::string, Exchange*> createExchange(std::string exchange_name,
                                                         vector<Stream<double>*> const& close_streams,
                                                         double commission){
            ExchangeOptions opt;
            opt.commission = commission;
            Exchange ex{std::move(exchange_name), {}, opt, close_streams};
            auto id = ex.ID();
            m_exchanges[id] = ex;
            return {id, &m_exchanges[id]};
        }
        std::pair<std::string, Wallet*>createWallet(std::string const& exchange_id, Quantity const& instrument) {
            Wallet wallet(exchange_id, instrument);
            auto id = wallet.ID();
            m_wallets.insert_or_assign(wallet.ID(), std::move(wallet));
            return {id, &m_wallets.at(id)};
        }

        inline bool isPairTradable(string const& exchange_name, TradingPair const& pair_){
            return m_exchanges[exchange_name].isPairTradable(pair_);
        }

        inline Exchange&    exchanges(std::string const& exchange_id)    { return m_exchanges.at(exchange_id); }

        inline Wallet&      wallet(std::string const& wallet_id)           { return m_wallets.at(wallet_id); }

        Ledger*      ledger();

        auto saveAndReturn(Stream<double>* new_data)
        {
            double_streams[new_data->Name()] = new_data;
            return new_data;
        }

        auto source(std::vector<double> const& iterator_data)
        {
            return saveAndReturn(new IterableStream<double>(iterator_data));
        }

        Stream<double>* group(std::vector<Stream<double>*> const& vect)
        {
            return saveAndReturn(new Group<double>(vect));
        }

        template<class ClassType>
        auto sensor(ClassType const& _obj, std::function<double(ClassType const&)> const&  _func)
        {
            return saveAndReturn(new Sensor<ClassType, double>(_obj, _func));
        }

        auto constant(double const& value)
        {
            return saveAndReturn( new Constant<double>(value));
        }

        auto placeholder()
        {
            return saveAndReturn( new Placeholder<double>());
        }

        auto agg( std::function<double(std::vector<double> const&)> const& func,
                                     std::vector<Stream<double>*> const& vect)
        {
            return saveAndReturn( new Aggregate<double>(func, vect));
        }

        auto sum(std::vector<Stream<double>*> const& vect)
        {
            return agg([](vector<double> const& input){
                return std::accumulate(input.begin(), input.end(), 0.0); },
                       vect);
        }

        auto min(std::vector<Stream<double>*> const& vect)
        {
            return agg([](vector<double> const& input){
                return *(std::min_element(input.begin(), input.end())); }, vect);
        }

        auto max(std::vector<Stream<double>*> const& vect)
        {
            return agg([](vector<double> const& input){
                return *(std::max_element(input.begin(), input.end())); }, vect);
        }

        auto apply(Stream<double>* x,
                                      const std::function<double(double)>& fnc)
        {
            return saveAndReturn( new Apply(x, fnc));
        }

        auto  mul(Stream<double>* s1, Stream<double>* s2)
        {
            return saveAndReturn(new BinOp<double, double, double>([](double const& x1, double const& x2)
            { return x1 * x2; }, s1, s2 ));
        }

        template<typename T>
        auto select(vector<Stream<T>*> const& streams, std::function<bool(Stream<T>* )> const& fnc)
        {
            auto first = std::find_if(streams.begin(), streams.end(), [&fnc](Stream<T>* stream){
                return fnc(stream);
            });

            if(first != streams.end())
                return *first;

            throw std::runtime_error("No stream satisfies selector condition.");
        }

        ~Ctx();

    };

};

#define cctx Ctx::ctx()

#endif //TENSORTRADECPP_CONTEXT_H
