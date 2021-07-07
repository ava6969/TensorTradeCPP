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

    public:

        explicit Apply(std::shared_ptr<Stream<T>> _inp,std::function<T(T)> const& _func,
                       string name="apply"): Stream<T>(move(name),vector<std::shared_ptr<Stream<T>>>{_inp}),
                       func(_func) {}

        StreamType forward()
        {
            auto node = this->inputs.front();
            assert(node);
            return this->func(std::get<T>(node->Value()));
        }
    };

    template<typename T>
    class Lag : public Stream<T>
    {
        int lag{}, runs{};
        std::queue<T> history;

    public:
        explicit Lag(int lag): Stream<T>("lag"), lag(lag) {}

        inline T forward() override
        {
            auto node = this->inputs[0];
            if(runs < lag)
            {
                runs++;
                history.push(node->value);
                return std::nan("lag_nan");
            }
            history.push(node->value);
            auto ret = history.back();
            history.pop();
            return ret;
        }

        void reset() override
        {
            runs = 0;
            history = {};
        }

    };

    template<typename T>
    class Aggregate : public Stream<T>
    {

        std::function<T(std::vector<T> const&)> func;
    public:

        Aggregate(std::function<T(std::vector<T> const&)> _fnc,
                  std::vector< std::shared_ptr<Stream<T>>> _inputs): Stream<T>("agg", move(_inputs)), func(move(_fnc)) {}

        StreamType forward()
        {
            std::vector<T> _inp(this->inputs.size());
            std::transform(this->inputs.begin(), this->inputs.end(), _inp.begin(),[](std::shared_ptr<Stream<T> > const& s)
            {
                return std::get<T>(s->Value());
            });

            return func(_inp);
        }
    };

    template<typename T>
    class Warmup : public Stream<T>
    {

    };

    template<typename T>
    class Reduce : public Stream<T>
    {

    public:
        Reduce(std::vector<std::shared_ptr<Stream<T > > > _inputs): Stream<T>("reduce", move(_inputs))
        {}

        std::shared_ptr<Stream<T>> agg(    std::function<T(std::vector<T> const&)> func)
        {
            return std::make_shared<Aggregate<T > >(func, this->inputs);
        }

        std::shared_ptr<Stream<T>>  sum()
        {
            return agg([](vector<T> const& input){
                return std::accumulate(input.begin(), input.end(), 0);
            });
        }

        std::shared_ptr<Stream<T>>  min()
        {
            return agg([](vector<T> const& input){
                return std::min_element(input.begin(), input.end());
            });
        }

        std::shared_ptr<Stream<T>>   max()
        {
            return agg([](vector<T> const& input){
                return std::max_element(input.begin(), input.end());
            });
        }


        StreamType forward()
        {
            std::vector<T> _inp(this->inputs.size());
            std::transform(this->inputs.begin(), this->inputs.end(), _inp.begin(),[](std::shared_ptr<Stream<T> > const& s)
            {
               return std::get<T>(s->Value());
            });
            return _inp;
        }

    };

    template<typename T>
    class ForwardFill : public Stream<T>
    {

    };

    template<typename T>
    class FillNa : public Stream<T>
    {

    };

    template<typename T>
    class Accumulator : public Stream<T> {


    };

    template<typename T>
    class Copy : public Stream<T> {


    };

    template<typename T>
    class Freeze : public Stream<T> {


    };

    template<typename T>
    class BinOp : public Stream<T> {

        std::function<T(T const&, T const&)> func;

    public:
        BinOp(std::function<T(T const&, T const&)>  _fnc, std::vector<FloatStream> _inputs):
              Stream<T>("bin_op", move(_inputs)), func(move(_fnc)) {}

        StreamType forward()
        {
            return func(std::get<T>(this->inputs[0]->Value()), std::get<T>(this->inputs[1]->Value()));
        }

    };



};


#endif //TENSORTRADECPP_OPERATORS_H
