//
// Created by dewe on 6/29/21.
//

#ifndef TENSORTRADECPP_OPERATORS_H
#define TENSORTRADECPP_OPERATORS_H

#include <queue>
#include "base.h"
#include "functional"

namespace ttc {

    template<typename T>
    class Apply : public Stream<T>
    {
        std::function<T(T)> func;
        Stream<T>* node;
    public:

        explicit Apply(Stream<T>* _inp, std::function<T(T)> const& _func,
                       string name="apply"):Stream<T>(move(name)),func(_func), node(_inp) {}

        void forward()
        {
            this->m_data = this->func(node->value());
        }
    };

    template<typename T>
    class Lag : public Stream<T>
    {
        int lag{}, runs{};
        std::queue<double> history;

    public:
        explicit Lag(int lag): Stream<T>("lag"), lag(lag) {}

        inline void forward() override
        {
            auto node = this->inputs[0];
            if(runs < lag)
            {
                runs++;
                history.push(std::get<double>(node->value.value()));
                this->value = std::nan("lag_nan");
            }
            history.push(node->value());
            this->m_data = history.back();
            history.pop();

        }

        void reset() override
        {
            runs = 0;
            history = {};
        }

    };

    template<typename T>
    class Aggregate : public Group<T>
    {
        std::function<T(std::vector<T> const& )> func;

    public:

        Aggregate(std::function<T(std::vector<T> const&)> _fnc,
                  std::vector<Stream<T>* > const& _inputs):Group<T>(_inputs, "agg"),
                  func(move(_fnc)){}

                  void forward()
        {
            std::vector<T> _inp(this->m_inputs.size());
            //todo: use ranges
            std::transform(this->m_inputs.begin(), this->m_inputs.end(), _inp.begin(), [](Stream<T>* s)
            {
                return s->value();
            });

            this->m_data = func(_inp);
        }
    };

    template<typename L, typename R, typename OutputType>
    class BinOp : public Group<OutputType> {

        std::function<OutputType(L const&, R const&)> func;

    public:
        BinOp(std::function<OutputType(L const&, R const&)> _fnc,
              Stream<L>* l, Stream<R>* r): Group<OutputType>({l, r}, "bin_op"), func(move(_fnc)) {}

        void forward() override
        {
            this->m_data = func(this->m_inputs[0]->value(), this->m_inputs[1]->value());
        }

    };

};


#endif //TENSORTRADECPP_OPERATORS_H
