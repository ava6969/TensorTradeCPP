cmake_minimum_required(VERSION 3.19)

set(CMAKE_TOOLCHAIN_FILE ~/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

project(TensorTradeCPP)
add_compile_options(-lconcepts)
find_path(XFRAME_INCLUDE_DIRS "xframe/xaxis.hpp")
include_directories(feed)
include_directories(oms)

set(Torch_DIR ~/libtorch/share/cmake/Torch)

link_directories(ta-lib/src/.libs)
file(GLOB LIB_TA_LIB ${CMAKE_CURRENT_SOURCE_DIR}/ta-lib/src/.libs/libta_lib.a)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS} -DBOOST_LOG_DYN_LINK")

find_package(Torch REQUIRED)
find_package(pybind11 CONFIG REQUIRED)
set(CMAKE_CXX_STANDARD 20)

include_directories(ta-lib/include)
include_directories(env)

add_executable(TensorTradeCPP
         main.cpp
        core/base.h core/clock.h
        feed/core/base.h feed/core/base.cpp
        feed/core/operators.h feed/core/feed.h
        feed/api/generic.h feed/api/float.h feed/api/boolean.h
        feed/api/dstring.h core/dexceptions.h oms/instruments/instrument.h oms/instruments/quantity.h
        oms/instruments/trading_pair.cpp oms/instruments/trading_pair.h oms/instruments/exchange_pair.h
        oms/instruments/instrument.cpp oms/instruments/quantity.cpp oms/exchanges/exchange.cpp oms/exchanges/exchange.h
        core/types.h core/dexceptions.cpp oms/orders/broker.cpp oms/orders/broker.h
        oms/orders/create.h oms/orders/criteria.cpp oms/orders/criteria.h oms/orders/order.cpp oms/orders/order.h
        oms/orders/order_listener.h oms/orders/order_spec.cpp oms/orders/order_spec.h oms/orders/trade.cpp
        oms/orders/trade.h oms/wallets/wallet.cpp oms/wallets/wallet.h oms/wallets/ledger.h
        oms/wallets/portfolio.cpp oms/wallets/portfolio.h oms/services/execution/simulated.h oms/services/execution/alpaca.cpp
        oms/services/execution/alpaca.h oms/services/slippage/random_slippage_model.h oms/services/slippage/slippage_model.h
        oms/services/execution/simulated.cpp oms/instruments/exchange_pair.cpp
        #tests/core/test_clock.cpp
        #tests/core/test_base.cpp tests/feed/test_api.cpp tests/feed/test_core.cpp tests/oms/test_instrument.cpp tests/oms/test_orders.cpp
        env/generic/components/action_scheme.h env/generic/components/informer.h env/generic/components/observer.h
        env/generic/components/renderer.h env/generic/components/reward_scheme.h
        env/generic/components/stopper.h env/generic/environment.h env/default/informers.h env/default/informers.cpp env/default/stoppers.cpp env/default/stoppers.h env/default/rewards.cpp env/default/rewards.h env/default/actions.cpp env/default/actions.h env/generic/components/action_listener.h env/default/observers.cpp env/default/observers.h)

target_include_directories(TensorTradeCPP PRIVATE ${XFRAME_INCLUDE_DIRS})

target_link_libraries(TensorTradeCPP  "${TORCH_LIBRARIES}"  pybind11::lto pybind11::embed pybind11::module ${LIB_TA_LIB})