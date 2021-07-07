//
// Created by dewe on 7/5/21.
//

#ifndef TENSORTRADECPP_ACTIONS_H
#define TENSORTRADECPP_ACTIONS_H

#include <orders/order_listener.h>
#include <orders/criteria.h>
#include "generic/components/action_scheme.h"
#include "torch/torch.h"
#include "generic/components/action_listener.h"
#include "../../core/types.h"

namespace ttc
{

    class TensorTradeActionScheme : public ActionScheme<torch::Tensor> {

    protected:
        class Portfolio* m_portfolio;
        class Broker* m_broker;
        std::vector<ActionListener<std::vector<int>> *> listeners;
    public:

        explicit TensorTradeActionScheme(Portfolio* portfolio, Broker* broker);

        void clock( Clock const& clock) override;

        void perform(class TradingEnv* env, torch::Tensor const& action) override;

        virtual std::vector<std::shared_ptr< class Order> > getOrders( torch::Tensor const& _action,
                Portfolio* portfolio) = 0 ;

        class Portfolio * portfolio() const override;

        void attach(class ActionListener<std::vector<int>> *listener) { listeners.push_back(listener); }
    };

    class BSH : public TensorTradeActionScheme
    {
        const class Wallet& cash;
        const class Wallet& asset;
        int action;

    public:
        BSH(Portfolio* portfolio, Broker* broker, const Wallet &cash, const Wallet &asset);

        std::vector<int> actionShape() override { return {2}; }

        std::vector<std::shared_ptr<class Order>> getOrders(torch::Tensor const& _action,
                Portfolio *portfolio) override;

        void reset() override
        {
            TensorTradeActionScheme::reset();
            action = 0;
        }
    };

    class SimpleOrders : public TensorTradeActionScheme
    {
        TradeType type;
        float min_order_pct, min_order_abs;
        std::vector<int> d_action_shape, m_duration;
        std::vector<float> m_trade_sizes;
        std::vector<size_t> action_types;

        std::vector<OrderListener *> m_listeners;
        std::vector<std::shared_ptr<Criteria> > m_criteria;

    public:
        SimpleOrders(Portfolio* portfolio, Broker* broker,
                     std::vector<std::shared_ptr<Criteria > > criteria,
                     std::vector<float> trade_size={1./10, 1./5, 3./10, 2./5, 1./2, 3./5, 7./10, 4./5, 9./10, 1.},
                        std::vector<int> durations={},
                        TradeType tradeType=TradeType::MARKET,
                        std::vector<OrderListener*> listeners={},
                        float min_order_pct=0.02, float min_order_abs=0.00);

        SimpleOrders(Portfolio* portfolio, Broker* broker,
                     std::vector<std::shared_ptr< Criteria > >criteria,
                     int trade_size=10,
                     std::vector<int> durations={},
                     TradeType tradeType=TradeType::MARKET,
                     std::vector<OrderListener*>  listeners={},
                     float min_order_pct=0.02, float min_order_abs=0.00);

        std::vector<int> actionShape() override ;

        std::vector<std::shared_ptr<class Order>> getOrders(torch::Tensor const& _action,
                Portfolio *portfolio) override;

    };

    class ManagedRiskOrders : public TensorTradeActionScheme
    {
        TradeType type;
        float min_order_pct, min_order_abs;
        std::vector<int> d_action_shape, m_duration;
        std::vector<float> m_trade_sizes, stop, take;
        std::vector<size_t> action_types;
        std::vector<OrderListener *> m_listeners;

    public:
        ManagedRiskOrders(Portfolio* portfolio,
                          Broker* broker,
                     std::vector<float> stop={0.02, 0.04, 0.06},
                     std::vector<float> take={0.01, 0.02, 0.03},
                     std::vector<float> trade_size={1./10, 1./5, 3./10, 2./5, 1./2, 3./5, 7./10, 4./5, 9./10, 1.},
                     std::vector<int> durations={},
                     TradeType tradeType=TradeType::MARKET,
                     std::vector<OrderListener*> listeners={},
                     float min_order_pct=0.02, float min_order_abs=0.00);

        ManagedRiskOrders(Portfolio* portfolio, Broker* broker,
                          std::vector<float> stop,
                          std::vector<float> take,
                     int trade_size=10,
                     std::vector<int> durations={},
                     TradeType tradeType=TradeType::MARKET,
                     std::vector<OrderListener*>  listeners={},
                     float min_order_pct=0.02, float min_order_abs=0.00);

        std::vector<int> actionShape() override ;

        std::vector<std::shared_ptr<class Order>> getOrders(torch::Tensor const& _action,
                                                            Portfolio *portfolio) override;

    };

}


#endif //TENSORTRADECPP_ACTIONS_H
