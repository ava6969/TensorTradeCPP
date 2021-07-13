#include "tensortrade/core/context.h"
#include "tensortrade/env/generic/environment.h"
#include "tensortrade/env/default/rewards.h"
#include "tensortrade/env/default/stoppers.h"
#include "tabulate/table.hpp"

using namespace ttc;
using std::array;


vector<float> convertToList(torch::Tensor close_price)
{
    auto n = close_price.size(0);
    close_price = close_price.view(-1);
    vector<float> converted(n);
    memcpy(converted.data(), close_price.data_ptr<float>(), sizeof(float) * n);
    return converted;
}

void bhScheme(Ctx * context)
{
    int window_Size = 3;
    context->createActionScheme<BSH>(*context->Cash(), *context->Asset());
    context->createRewardScheme<RiskAdjustedReturns>(PerformanceMetric::SHARPE, 0, 0, window_Size);
    context->createObserverScheme<TensorTradeObserver>(window_Size);
    context->createInformerScheme<TTInformer>();
    context->createStopperScheme<MaxLossStopper>(0.1);
}

void simpleOrderScheme(Ctx * context)
{
    int window_Size = 3;
    context->createActionScheme<SimpleOrders>(std::nullopt, 10, std::nullopt, TradeType::MARKET);
//    context->createRewardScheme<SimpleProfit>(3);
    context->createRewardScheme<RiskAdjustedReturns>(PerformanceMetric::SHARPE, 0, 0, window_Size);
//    context->createRewardScheme<PBR>(context->Price());
    context->createObserverScheme<TensorTradeObserver>(window_Size);
    context->createInformerScheme<TTInformer>();
    context->createStopperScheme<MaxLossStopper>(0.1);
}


vector<int> simple_strategy(torch::Tensor const& close_price_tensor) //[p1, p2, p3]
{
    auto close_price = convertToList(close_price_tensor);
    std::vector<int> actions;
    actions.resize(3);
    if((close_price.front() < close_price.back()) and (close_price.back() < close_price[1]))
    {
        actions[0] = 1;
        actions[1] = 1;
        actions[2] = rand() % 10;
    }

    return actions;
}

vector<int> random_strategy(ttc::TradingEnv const& env) //[p1, p2, p3]
{
    vector<int> action;
    for(auto a : env.action_shape)
        action.push_back(rand() % a);
    return action;
}

int main()
{

    auto* context = Ctx::ctx();

//    TickerSpec btc_usd = {"BTCUSD", "crypto"};
//    TickerSpec eth_usd = {"ETHUSD", "crypto"};
//    TickerSpec ltc_usd = {"LTCUSD", "crypto"};
//
//    ExchangeSpec bit_stamp, bit_finex;
//
//    bit_stamp.name = "bitstamp";
//    bit_stamp.features = {"close"};
//    bit_stamp.ticker_specs = {btc_usd, eth_usd};
//    bit_stamp.qty = {10000 * USD, 0 * BTC, 0 * ETH};
//
//    bit_finex.name = "bitfinex";
//    bit_finex.features = {"close"};
//    bit_finex.ticker_specs = {btc_usd, ltc_usd};
//    bit_finex.qty = {10000 * USD, 0 * BTC, 0 * LTC};
    ExchangeSpec bit_stamp;
    TickerSpec btc_usd = {"BTCUSD", "crypto"};
    bit_stamp.name = "bitstamp";
    bit_stamp.features = {"close"};
    bit_stamp.ticker_specs = {btc_usd};
    bit_stamp.qty = {10000 * USD, 0 * BTC};

    auto env = context->build({bit_stamp}, simpleOrderScheme);

//    auto py_wrapper = context->Feed()->next<unordered_map<string, unordered_map<string, float>>>();
//    for(auto const& d: py_wrapper)
//    {
//        std::cout << d.first << "\n";
//        for(auto const d1: d.second)
//        {
//            std::cout << d1 << "\n";
//        }
//    }

    for(int i = 0; i < 2; i++)
    {
        bool _done = false;
        auto close_prices = env.reset();
        std::cout << "close_price window:\t" << close_prices << "\n";

        while(not _done)
        {
            vector<int> action = simple_strategy(close_prices);

            auto start = std::chrono::high_resolution_clock::now();
            auto[next_close_price, reward, done, info] = env.step(torch::tensor(action));
            auto delta = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - start).count()/1000;
            std::stringstream ss;

            std::cout << "step took " << delta << "seconds\n";
            std::cout << "net worth:\t" << info["net_worth"] << "\tstep:\t" << info["step"] << "\nreward:\t"
            << reward << "\tdone:\t" << (done ? "True" : "False") << "\n";

            std::cout << "observation:\t" << next_close_price << "\n";

            if(info["step"] >= 1000)
            {
                break;
            }

            _done = done;
            close_prices = next_close_price;
        }
    }

    return 0;
}
