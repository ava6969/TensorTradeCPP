//
// Created by dewe on 6/29/21.
//

#ifndef TENSORTRADECPP_OPERATORS_H
#define TENSORTRADECPP_OPERATORS_H

#include <queue>
#include "base.h"
#include "functional"

namespace ttc {

    class Apply : public Stream
    {
        std::function<double(double)> func;

    public:

        explicit Apply(std::shared_ptr<Stream> _inp,std::function<double(double)> const& _func,
                       string name="apply"): Stream(move(name),vector<std::shared_ptr<Stream>>{_inp}),
                       func(_func) {}

        StreamType forward()
        {
            auto node = this->inputs.front();
            assert(node);
            return this->func(std::get<double>(node->Value()));
        }
    };

    class Lag : public Stream
    {
        int lag{}, runs{};
        std::queue<double> history;

    public:
        explicit Lag(int lag): Stream("lag"), lag(lag) {}

        inline StreamType forward() override
        {
            auto node = this->inputs[0];
            if(runs < lag)
            {
                runs++;
                history.push(std::get<double>(node->value.value()));
                return std::nan("lag_nan");
            }
            history.push(std::get<double>(node->Value()));
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

    class Aggregate : public Stream
    {

        std::function<double(std::vector<double> const& )> func;
    public:

        Aggregate(std::function<double(std::vector<double> const&)> _fnc,
                  std::vector< std::shared_ptr<Stream>> _inputs): Stream("agg", move(_inputs)), func(move(_fnc)) {}

        StreamType forward()
        {
            std::vector<double> _inp(this->inputs.size());
            std::transform(this->inputs.begin(), this->inputs.end(), 
                           _inp.begin(), [](std::shared_ptr<Stream > const& s)
            {
                return std::get<double>(s->Value());
            });

            return func(_inp);
        }
    };


    class Warmup : public Stream
    {

    };


    class Reduce : public Stream
    {

    public:
        Reduce(std::vector<std::shared_ptr<Stream > > _inputs): Stream("reduce", move(_inputs))
        {}

        std::shared_ptr<Stream> agg(std::function<double(std::vector<double> const&)> func)
        {
            return std::make_shared<Aggregate>(func, this->inputs);
        }

        std::shared_ptr<Stream>  sum()
        {
            return agg([](vector<double> const& input){ return std::accumulate(input.begin(), input.end(), 0);
            });
        }

//        std::shared_ptr<Stream>  min()
//        {
//            return agg([] (vector<double> const& input){ return std::min_element(input.begin(), input.end());
//            });
//        }
//
//        std::shared_ptr<Stream>   max()
//        {
//            return agg([](vector<double> const& input){ return std::max_element(input.begin(), input.end());
//            });
//        }


        StreamType forward()
        {
            std::vector<double> _inp(this->inputs.size());
            std::transform(this->inputs.begin(), this->inputs.end(), _inp.begin(),[](std::shared_ptr<Stream > const& s)
            {
               return std::get<double>(s->Value());
            });
            return _inp;
        }

    };


    class BinOp : public Stream {

        std::function<double(double const&, double const&)> func;

    public:
        BinOp(std::function<double(double const&, double const&)>  _fnc, std::vector<Float64Stream> _inputs):
                Stream("bin_op", move(_inputs)), func(move(_fnc)) {}

        StreamType forward()
        {
            return func(std::get<double>(this->inputs[0]->Value()), std::get<double>(this->inputs[1]->Value()));
        }

    };

//    template<typename T>
//    class ForwardFill : public Stream<
//    {
//
//    };
//
//    template<typename T>
//    class FillNa : public Stream<T>
//    {
//
//    };
//
//    template<typename T>
//    class Accumulator : public Stream<T> {
//
//
//    };
//
//    template<typename T>
//    class Copy : public Stream<T> {
//
//
//    };
//
//    template<typename T>
//    class Freeze : public Stream<T> {
//
//
//    };




};


#endif //TENSORTRADECPP_OPERATORS_H
