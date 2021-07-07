//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_CRITERIA_H
#define TENSORTRADECPP_CRITERIA_H

#include "functional"
#include "string"

namespace ttc {

    class CriteriaBinOp;
    class NotCriteria;
    using std::string;
    class Criteria {

    public:
        virtual bool check(const class Order*, const class Exchange*) const = 0;

        virtual inline bool isNull() { return false;}
        bool operator()(const Order* order, const Exchange* exchange) const;

        CriteriaBinOp operator& (Criteria*  other);
        CriteriaBinOp operator| (Criteria*  other);
        CriteriaBinOp operator^ (Criteria*  other);
        NotCriteria  operator! ();

        virtual ~Criteria() =default;

    };

    using std::move;

    class CriteriaBinOp : public Criteria {


        std::function<bool(bool, bool)> op;
        string op_str;

    public:
        CriteriaBinOp(Criteria* left,
                      Criteria* right,
                      std::function<bool(bool, bool)>op,
                      string const& op_str):left(left), right(right),
                      op(std::move(op)),
                      op_str(op_str)
        {}

        CriteriaBinOp(CriteriaBinOp const& _clone):left(_clone.left), right(_clone.right),
                                                   op(_clone.op),
                                                   op_str(_clone.op_str){}
        Criteria* left;
        Criteria* right;

        bool check(const Order * order, const Exchange* exchange) const override;

        [[nodiscard]] string str() const
        {

        }

        ~CriteriaBinOp()
        {
            delete left;
            delete right;
        }

    };


    class NotCriteria : public Criteria {

        Criteria& criteria;

    public:

        NotCriteria(Criteria& criteria):criteria(criteria) {}

        bool check(const Order * order, const Exchange * exchange) const override;

        string str() const
        {

        }
    };

    class Limit : public Criteria {

        float limit_price;
    public:

        explicit Limit(float const& limit_price) : limit_price(limit_price) {}

        bool check(const Order * order, const Exchange * exchange) const override;

        inline std::string str() const
        {
           return std::string("<limit: price=").append(std::to_string(limit_price)).append(">");
        }
    };

    class Stop : public Criteria {

    public:

        enum Direction : uint8_t
        {
            up = '^',
            down='v'
        };
    private:
        Direction dir;
        float percent;

    public:
        explicit Stop(Direction const& direction, float percent): dir(direction), percent(percent) {}

        bool check(const Order * order, const Exchange * exchange) const override;

        string str() const
        {

        }

    };

    class Timed : public Criteria {

        float m_duration;
    public:
        Timed( float duration): m_duration(duration) {}

        bool check(const Order * order, const Exchange * exchange) const override;

        std::string str() const
        {
            return std::string("<Timed: duration=").append(std::to_string(m_duration)).append(">");
        }
    };
}

#endif //TENSORTRADECPP_CRITERIA_H
