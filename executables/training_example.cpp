#include "context.h"


using namespace ttc;
using std::array;

void createSchemes(Context * context)
{
    context->createActionScheme<ManagedRiskOrders>();
    context->createRewardScheme<RiskAdjustedReturns>(PerformanceMetric::SHARPE, 0, 0, 3);
    context->createObserverScheme<TensorTradeObserver>(3);
    context->createInformerScheme<TTInformer>();
    context->createStopperScheme<MaxLossStopper>(0.1);
}

void scheme2(Context * context)
{
    context->createActionScheme<ManagedRiskOrders>();
    context->createRewardScheme<RiskAdjustedReturns>(PerformanceMetric::SORTINO, 0, 0, 5);
    context->createObserverScheme<TensorTradeObserver>(3);
    context->createInformerScheme<TTInformer>();
    context->createStopperScheme<MaxLossStopper>(0.1);
}

auto strategy1(torch::Tensor const& observation)
{

}

int main()
{

    auto* context = Context::context();

    TickerSpec ltc_usd = {"LTCUSD", "crypto"};
    TickerSpec btc_usd = {"BTCUSD", "crypto"};

    ExchangeSpec bit_stamp, forex;
    bit_stamp.name = "bitstamp";

//    bit_stamp.features = {"SMA", "SMA_1", "EMA", "close"};
//    bit_stamp.features_param.add("SMA", py::dict("timeperiod"_a=25, "price"_a="open"));
//    bit_stamp.features_param.add("SMA", py::dict("timeperiod"_a=20, "price"_a="open"));
//    bit_stamp.features_param.add("EMA", py::dict("timeperiod"_a=20, "price"_a="open"));

    bit_stamp.ticker_specs = {btc_usd};
    bit_stamp.instruments = {1000 * USD, 0 * BTC};

//    forex.name = "forex";
//
//    forex.features = {"open", "close", "volume", "SMA"};
//    forex.features_param.add("SMA", py::dict("timeperiod"_a=20, "price"_a="open"));

    auto env = context->build({bit_stamp}, scheme2);

    for(int i = 0; i < 2; i++)
    {
        bool _done = false;
        auto obs = env.reset();
        std::cout << "obs:\t" << obs << "\n";
        while(not _done)
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
            _done = done;
        }
    }

    return 0;
}
