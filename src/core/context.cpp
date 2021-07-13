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

    }

    std::tuple<Portfolio, Broker, Wallet*, Wallet*, DataFeed*> Ctx::build(const vector<ExchangeSpec> &exchange_specs)
    {

        vector<std::string> wallet_ids;
        Wallet* cash;
        Wallet* asset;
        Float64Stream price;
        vector<Float64Stream> close_streams;
        vector<Float64Stream> features_streams;

        for( auto const& exchange_spec: exchange_specs)
        {
            for(auto const& ticker: exchange_spec.ticker_specs)
            {
                auto it = ticker.ticker.find("USD");
                auto t = ticker.ticker;
                if(it != string::npos)
                    t.replace(it, 3, "");

                auto wrapper = make_module.attr("DataWrapper")(ticker.ticker,
                        ticker.market_type,
                        exchange_spec.features_param.tech_indicators_spec);

                for(auto const& feature: exchange_spec.features)
                {
                    auto _data  = data(wrapper, feature);

                    features_streams.emplace_back(source(vector<double>(_data.begin(), _data.end())));
                    features_streams.back()->rename(feature);

                    if (feature.find("close") != string::npos)
                    {
                        close_streams.push_back(source(vector<double>(_data.begin(), _data.end())));
                        close_streams.back()->rename("USD-" + t);
                        price = source(vector<double>(_data.begin(), _data.end()));
                    }
                }
            }

            if(close_streams.empty())
            {
                throw std::runtime_error("at least a stream py_wrapper containing close prices must be included");
            }

            if(not m_ledger)
                m_ledger = std::make_unique<Ledger>();

            Exchange ex{exchange_spec.name, {}, {}, close_streams};

            auto exchanged_id = ex.ID();
            Ctx::ctx()->registerExchange(move(ex));

            for(auto& qty : exchange_spec.qty)
            {
                Wallet wallet{exchanged_id, qty};
                Ctx::ctx()->registerWallet(wallet);

                if(qty.instrument == USD)
                {
                    cash = &Ctx::ctx()->wallet(wallet.ID());
                }else
                    asset = &Ctx::ctx()->wallet(wallet.ID());

                wallet_ids.push_back(wallet.ID());
            }
        }

        feeds.emplace_back(features_streams);
        feeds.back().compile();

        return  {Portfolio{USD, wallet_ids, nullptr}, Broker{}, cash, asset, &feeds.back()};
    }

    Ctx::~Ctx()
        { py::finalize_interpreter(); delete m_instance; }

    Ledger* Ctx::ledger()
        { return m_ledger.get(); }


}