//
// Created by dewe on 7/8/21.
//

#include "catch.hpp"
#include <boost/coroutine/all.hpp>
#include <iostream>

#include "include/oms/instruments/instrument.h"
#include "include/env/default/rewards.h"
#include "pybind11/embed.h"
#include "pybind11/numpy.h"

namespace py = pybind11;
using namespace boost::coroutines;
using namespace ttc;
using namespace py::literals;

TEST_CASE("Simple Profit [Get Reward]")
{

    auto net_worth = vector<float>{100, 400, 350, 450, 200, 400, 330, 560};

    py::scoped_interpreter interpreter;
    auto make_module = pybind11::module_{py::module_::import("pandas")};
    auto np_module = pybind11::module_{py::module_::import("numpy")};
    py::print(make_module);

    py::list l;
    Portfolio portfolio(USD, {}, nullptr);
    Clock clk;
    portfolio._clock = &clk;

    for(auto & p : net_worth)
    {
        portfolio.onNext({{"internal", {{"net_worth", p}}}});
        l.append(p);
        clk.increment();
    }

    auto net_worth_panda = make_module.attr("Series")(l);
    auto pct_chg = net_worth_panda.attr("pct_change")();

    SimpleProfit reward_scheme;
    auto pct_chg_list = pct_chg.attr("tolist")().cast<py::list>();

    REQUIRE( abs(reward_scheme.getReward(portfolio) - pct_chg_list[7].cast<float>()) < 0.0001);

    SimpleProfit reward_scheme2(3);
    auto r = (1 + pct_chg_list[7].cast<float>()) *
            (1 + pct_chg_list[6].cast<float>()) *
            (1 + pct_chg_list[5].cast<float>());

    REQUIRE( abs(reward_scheme2.getReward(portfolio) - r + 1) < 0.0001);
    int sz = 8;
    auto returns = net_worth_panda[py::slice(sz-3, sz, 1)].attr("pct_change")().attr("dropna")();
    auto returns_list = returns.attr("tolist")().cast<py::list>();
    auto expected_ratio = (np_module.attr("mean")(returns_list) + py::float_(1e-9)) / (np_module.attr("std")(returns_list) + py::float_(1e-9));

    RiskAdjustedReturns r_scheme(PerformanceMetric::SHARPE, 0, 1);

    vector<float> ret_vector{ returns_list[0].cast<float>(), returns_list[1].cast<float>()};

    auto sr = r_scheme.sharpe_ratio(ret_vector);

    REQUIRE(sr - expected_ratio.cast<float>() < 0.0001);

    r_scheme = RiskAdjustedReturns(PerformanceMetric::SORTINO, 0, 0, 1);


    auto ret = r_scheme.sortino_ratio(ret_vector);
    auto locals = py::dict();
    py::exec(R"(
        import pandas as pd
        import numpy as np
        returns = pd.Series([-0.175, 0.6969697])
        downside_returns = returns.copy()
        downside_returns[returns < 0] = returns ** 2
        expected_return = np.mean(returns)
        downside_std = np.sqrt(np.std(downside_returns))
        expected_ratio = (expected_return + 1E-9) / (downside_std + 1E-9)
    )", locals);

    py::print(locals["expected_ratio"]);
    std::cout << ret << "\n";

    REQUIRE(ret - locals["expected_ratio"].cast<float>() < 0.0001);
}


TEST_CASE("Sortino ratio")
{
    auto net_worth = vector<float>{400, 330, 560};

    py::scoped_interpreter interpreter;
    auto make_module = pybind11::module_{py::module_::import("pandas")};
    auto np_module = pybind11::module_{py::module_::import("numpy")};
    py::print(make_module);

    py::list l;
    Portfolio portfolio(USD, {}, nullptr);
    Clock clk;
    portfolio._clock = &clk;

    for(auto & p : net_worth)
    {
        portfolio.onNext({{"internal", {{"net_worth", p}}}});
        l.append(p);
        clk.increment();
    }


    RiskAdjustedReturns r_scheme1(PerformanceMetric::SHARPE, 0, 1);
    auto sr = r_scheme1.getReward(portfolio);

    auto r_scheme = RiskAdjustedReturns(PerformanceMetric::SORTINO, 0, 0, 1);
    auto ret = r_scheme.getReward(portfolio);

    auto locals = py::dict();
    py::exec(R"(
        import pandas as pd
        import numpy as np
        returns = pd.Series([-0.175, 0.6969697])
        downside_returns = returns.copy()
        downside_returns[returns < 0] = returns ** 2
        expected_return = np.mean(returns)
        downside_std = np.sqrt(np.std(downside_returns))
        expected_ratio = (expected_return + 1E-9) / (downside_std + 1E-9)
    )", locals);

    py::print(locals["expected_ratio"]);
    std::cout << ret << "\n";

    REQUIRE(ret - locals["expected_ratio"].cast<float>() < 0.0001);

}