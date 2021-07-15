//
// Created by dewe on 7/9/21.
//

#include "tensortrade/core/context.h"
#include "tensortrade/core/base.h"
#include <tensortrade/oms/orders/broker.h>
#include <tensortrade/env/default/stoppers.h>
#include <tensortrade/oms/wallets/ledger.h>
#include "tensortrade/feed/core/base.h"
#include "tensortrade/oms/services/execution/simulated.h"
#include "tensortrade/env/generic/environment.h"
#include "tensortrade/env/default/observers.h"

namespace ttc {

    Ctx* Ctx::m_instance = nullptr;
    std::string Ctx::sys_path ="TensorTrade/py_wrapper";
    Ctx::Ctx() {

        py::initialize_interpreter();
        // auto ret_code = TA_Initialize();
        auto sys = py::module_::import("sys");
        sys.attr("path").attr("append")(sys_path);

        py::print(sys.attr("path"));
        make_module = pybind11::module_{py::module_::import("data_wrapper")};

        m_ledger = std::make_unique<Ledger>();

    }

    std::tuple<Portfolio, Broker, Wallet*, Wallet*, BaseDataFeed<double>, Float64Stream>
            Ctx::build(const vector<ExchangeSpec> &exchange_specs)
    {

        vector<std::string> wallet_ids;
        Wallet* cash;
        Wallet* asset;
        Float64Stream price;

        vector<Float64Stream> close_streams;
        vector<Float64Stream> features_streams;

        for( auto const& exchange_spec: exchange_specs)
        {
            if(feature_map.contains(exchange_spec))
            {
                auto[_close_streams, _feature_streams] = feature_map.at(exchange_spec);
                close_streams.insert(close_streams.end(), _close_streams.begin(), _close_streams.end());
                features_streams.insert(features_streams.end(), _feature_streams.begin(), _feature_streams.end());
            }
            else
            {
                for(auto const& ticker: exchange_spec.ticker_specs)
                {
                    auto it = ticker.ticker.find("USD");
                    auto t = ticker.ticker;
                    if(it != string::npos)
                        t.replace(it, 3, "");

                    auto data_module = make_module.attr("DataWrapper")(ticker.ticker,
                                                                       ticker.market_type,
                                                                       exchange_spec.features_param.
                                                                       tech_indicators_spec);

                    for(auto const& feature: exchange_spec.features)
                    {
                        auto _data  = data(data_module, feature);

                        features_streams.emplace_back(source(vector<double>(_data.begin(), _data.end()))->rename(feature));

                        if (feature.find("close") != string::npos)
                        {
                            close_streams.emplace_back(source(
                                    vector<double>(_data.begin(), _data.end()))->rename("USD-" + t));
                            price = close_streams.back();
                        }
                    }
                }
                feature_map[exchange_spec] = {close_streams, features_streams};
            }

            if(close_streams.empty())
            {
                throw std::runtime_error("at least a stream py_wrapper containing close prices must be included");
            }

            auto[exchange_id, ex] = cctx->createExchange(exchange_spec.name, close_streams, exchange_spec.commision);

            for(auto& qty : exchange_spec.qty)
            {
                auto[wallet_id, wallet] = cctx->createWallet(exchange_id, qty);

                if(qty.instrument == USD){
                    cash = wallet;
                }else{
                    asset = wallet;
                }
                wallet_ids.push_back(wallet_id);
            }
        }

        BaseDataFeed<double> feeds(features_streams);
        feeds.compile();

        m_ledger->setSkipLog(exchange_specs.front().skipLog);
        return  {Portfolio{USD, wallet_ids, nullptr}, Broker{}, cash, asset, std::move(feeds), price};
    }

    Ctx::~Ctx()
        { py::finalize_interpreter(); delete m_instance; for(auto const& s: double_streams) delete s.second; }

    Ledger* Ctx::ledger()
        { return m_ledger.get(); }

}