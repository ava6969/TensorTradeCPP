#include <iostream>
#include <exchanges/exchange.h>
#include <core/feed.h>
#include <orders/broker.h>
#include <default/stoppers.h>
#include "core/base.h"
#include "core/clock.h"
#include "feed/core/base.h"
#include "ta_libc.h"
#include "pybind11/embed.h"
#include "../../oms/services/execution/simulated.h"
#include "wallets/wallet.h"
#include "env/generic/environment.h"
#include "env/default/actions.h"
#include "env/default/observers.h"
#include "env/default/rewards.h"
#include "env/default/informers.h"

namespace py = pybind11;
using namespace ttc;

std::vector<double> data(py::object const& wrapper, std::string const& column)
{
    std::vector<double> result;
    auto d_list = wrapper.attr("get_series")(column).cast<py::list>();
    std::transform(d_list.begin(), d_list.end(), std::back_inserter(result), [](auto& f)
    { return f.template cast<double>(); });
    return result;
}


int main()
{
    auto ret_code = TA_Initialize();

    py::scoped_interpreter interpreter;
    auto sys = py::module_::import("sys");
    sys.attr("path").attr("append")("data");
    auto make_module = pybind11::module_{py::module_::import("data_wrapper")};
    py::print(make_module);
    auto wrapper = make_module.attr("DataWrapper")();

    auto close  = data(wrapper, "close");
    TA_Integer outBeg;
    TA_Integer outNbElement;
    std::vector<double> rsi(close.size());
    std::vector<double> macd(close.size());
    std::vector<double> macd_signal(close.size());
    std::vector<double> macd_histogram(close.size());

    TA_RSI(0, close.size(), close.data(), 20, &outBeg, &outNbElement, rsi.data());
    TA_MACD(0, close.size(), close.data(), 10, 50, 5, &outBeg, &outNbElement, macd.data(),
            macd_signal.data(), macd_histogram.data());

    auto close_source = source<float>(vector<float>(close.begin(), close.end()));
    close_source->rename("close");
    auto rsi_source = source<float>(vector<float>(rsi.begin(), rsi.end()));
    rsi_source->rename("rsi");
    auto macd_source = source<float>(vector<float>(macd.begin(), macd.end()));
    macd_source->rename("macd");
    auto macd_signal_source = source<float>(vector<float>(macd_signal.begin(), macd_signal.end()));
    macd_signal_source->rename("macd_signal");
    auto macd_histogram_source = source<float>(vector<float>(macd_histogram.begin(), macd_histogram.end()));
    macd_histogram_source->rename("macd_histogram");

    DataFeed feed({close_source, rsi_source, macd_source, macd_signal_source, macd_histogram_source});
    feed.compile();

    auto src = source<float>(vector<float>(close.begin(), close.end()));
    src->rename<Stream<float>>("USD-BTC");

    Exchange bitstamp{"bitstamp", ttc::Service(), ExchangeOptions{}, vector<FloatStream>{std::move(src)} };

    Wallet cash{&bitstamp, 10000 * USD};
    Wallet asset{&bitstamp, 10 * BTC};

    Portfolio portfolio(USD, {cash, asset}, nullptr);
    Broker broker;
    ManagedRiskOrders managedRiskOrders(&portfolio, &broker);
    SimpleProfit sp(5);
    TensorTradeObserver observer(&portfolio, &feed, nullptr, 20);
    MaxLossStopper stopper(0.3);
    TTInformer informer;

    ttc::TradingEnv env(&managedRiskOrders, &sp, &observer, &stopper, &informer);

    auto obs = env.reset();
    std::cout << "obs:\t" << obs << "\n";
    for(int i = 0; i < 10; i++)
    {
        bool done = false;

        while(not done)
        {
            vector<int> action;
            for(auto a : env.action_shape)
                action.push_back(rand() % a);

            auto[next_obs, reward, done, info] = env.step(torch::tensor(action));

            std::cout << "info:\n";
            for(auto const& k : info)
                std::cout << k << "\n";
            std::cout << "reward:\t" << reward << "\n";
            std::cout << "done:\t" << done << "\n";
            std::cout << "obs:\t" << next_obs << "\n";
        }

    }


    Wallet::releaseLedger();


    return 0;
}
