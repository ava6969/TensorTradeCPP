//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_ORDER_H
#define TENSORTRADECPP_ORDER_H

#include <tensortrade/oms/exchanges/exchange.h>
#include "uuid/uuid.h"
#include "string"
#include "optional"
#include "vector"
#include "functional"
#include "tensortrade/core/types.h"
#include "tensortrade/core/base.h"
#include "order_spec.h"

using CriteriaArg = std::function<bool(class Order const& order, class Exchange const&)>;

namespace ttc {

    using std::string;
    using std::vector;

    enum class OrderStatus {
        PENDING,
        OPEN,
        CANCELLED,
        PARTIALLY_FILLED,
        FILLED
    };

    static string str(OrderStatus const& status)
    {
        switch (status) {

            case OrderStatus::PENDING:
                return "pending";
            case OrderStatus::OPEN:
                return "open";
            case OrderStatus::CANCELLED:
                return "cancelled";
            case OrderStatus::PARTIALLY_FILLED:
                return "partially_filled";
            default:
                return "filled";
        };
    }

    class Order : public TimedIdentifiable, public Observable<OrderListener> {

        int step{};
        TradeSide side{};
        TradeType trade_type{};
        class ExchangePair m_exchange_pair;
        std::optional<Quantity> m_quantity, m_remaining;
        class Portfolio m_portfolio;
        float price{};
        std::optional<std::function<bool(Order const& order, Exchange const&)>> criteria;
        string path_id{};
        std::optional<int> start{};
        std::optional<int> end{};
        OrderStatus status{};
        vector<struct OrderSpec> specs;
        vector<class Trade> trades{};

    public:

        Order(int step, const TradeSide &side,
              const TradeType &trade_type,
              ExchangePair const& _exchange_pair,
              optional <Quantity> _quantity,
              Portfolio  _portfolio,
              double price,
              const string &path_id = "",
              std::optional<int> start = std::nullopt, std::optional<int> end = std::nullopt,
              std::optional<std::function<bool(Order const& order, Exchange const&)>> const& _criteria={});

        Order(Order const& other)=default;
        Order& operator=(Order const& other)=default;
        bool operator==(Order const& other) const
        {
            return this->path_id == other.path_id;
        }
        bool operator!=(Order const& other) const
        {
            return this->operator==(other);
        };

        [[nodiscard]] inline auto type() const { return trade_type; }
        [[nodiscard]] inline auto Side() const { return side; }
        [[nodiscard]] inline auto Price() const { return price; }
        [[nodiscard]] ExchangePair const& exchangePair() const;
        [[nodiscard]] Quantity remaining() const;
        [[nodiscard]] Portfolio& portfolio();
        [[nodiscard]] class TradingPair pair() const;

        [[nodiscard]] inline auto Status() const { return status; }
        [[nodiscard]] inline auto pathID() const { return path_id; }
        [[nodiscard]] inline auto End() const { return end; }

        double size();

        [[nodiscard]] Instrument baseInstrument() const;
        [[nodiscard]] Instrument quoteInstrument() const;

        [[nodiscard]] inline bool isBuy() const { return side == TradeSide::BUY; }
        [[nodiscard]] inline bool isSell() const { return side == TradeSide::SELL; }
        [[nodiscard]] inline bool isLimit() const { return trade_type == TradeType::LIMIT; }
        [[nodiscard]] inline bool isMarket() const { return trade_type == TradeType::MARKET; }
        [[nodiscard]] bool isExecutable() const;
        [[nodiscard]] bool isExpired() const;
        [[nodiscard]] inline auto isCancelled() const { return status == OrderStatus::CANCELLED;}
        [[nodiscard]] inline auto isActive() const { return status != OrderStatus::FILLED and status != OrderStatus::CANCELLED;}

        bool isComplete();
        void addOrderSpec(OrderSpec order_spec);
        void execute();
        void fill(Trade const&);
        void cancel(std::string const& reason="CANCELLED");
        void release(std::string const& reason="RELEASE (NO REASON)");

        std::optional<Order> complete();
        std::unordered_map<string, string> toDict()
        {
            return {};
        }
    };

}

#endif //TENSORTRADECPP_ORDER_H
