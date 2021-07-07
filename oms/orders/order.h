//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_ORDER_H
#define TENSORTRADECPP_ORDER_H

#include <exchanges/exchange.h>
#include "uuid/uuid.h"
#include "string"
#include "optional"
#include "vector"
#include "functional"
#include "../../core/types.h"
#include "../../core/base.h"
#include "order_spec.h"

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
        std::optional<Quantity<float>> m_quantity, m_remaining;
        std::unique_ptr<class Portfolio> m_portfolio;
        float price{};
        std::shared_ptr<Criteria> criteria;
        string path_id{};
        std::optional<int> start{};
        std::optional<int> end{};
        OrderStatus status{};
        vector<struct OrderSpec> specs;
        vector<std::unique_ptr<class Trade>> trades{};

    public:

        Order()=default;
        Order(int step, const TradeSide &side, const TradeType &trade_type, ExchangePair  _exchange_pair,
              optional <Quantity<float>> _quantity, Portfolio *_portfolio, float price, const string &path_id = "",
              std::optional<int> start = std::nullopt, std::optional<int> end = std::nullopt,
              std::shared_ptr<Criteria>_criteria={});

        [[nodiscard]] inline auto type() const { return trade_type; }
        [[nodiscard]] inline auto Side() const { return side; }
        [[nodiscard]] inline auto Price() const { return price; }
        [[nodiscard]] ExchangePair* exchangePair();
        [[nodiscard]] const Quantity<float>* remaining() const;
        [[nodiscard]] Portfolio* portfolio();
        [[nodiscard]] class TradingPair const& pair() const;

        [[nodiscard]] inline auto Status() const { return status; }
        [[nodiscard]] inline auto pathID() const { return path_id; }
        [[nodiscard]] inline auto End() const { return end; }

        float size();

        [[nodiscard]] Instrument const& baseInstrument() const;
        [[nodiscard]] Instrument const& quoteInstrument() const;

        inline bool isBuy() const { return side == TradeSide::BUY; }
        inline bool isSell() const { return side == TradeSide::SELL; }
        [[nodiscard]] inline bool isLimit() const { return trade_type == TradeType::LIMIT; }
        inline bool isMarket() const { return trade_type == TradeType::MARKET; }
        [[nodiscard]] bool isExecutable() const;
        [[nodiscard]] bool isExpired() const;
        inline auto isCancelled() const { return status == OrderStatus::CANCELLED;}
        inline auto isActive() const { return status != OrderStatus::FILLED and status != OrderStatus::CANCELLED;}

        bool isComplete() const;

        Order* addOrderSpec(OrderSpec order_spec);

        void execute();
        void fill(std::unique_ptr<Trade> );

        std::unique_ptr<Order> complete();

        void cancel(std::string const& reason="CANCELLED");
        void release(std::string const& reason="RELEASE (NO REASON)");

        std::unordered_map<string, string> toDict()
        {
            return {};
        }
    };

}

#endif //TENSORTRADECPP_ORDER_H
