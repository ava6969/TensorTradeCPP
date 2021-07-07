//
// Created by dewe on 6/29/21.
//

#ifndef TENSORTRADECPP_FEED_H
#define TENSORTRADECPP_FEED_H

#include "../../feed/core/base.h"
#include "map"

#define  FloatStreamType  \
std::variant<float, std::unordered_map<string, float>, std::vector<float>, \
std::unordered_map<string, unordered_map<string, float>>>

using std::map;

namespace ttc
{

    class DataFeed : public Stream<float> {

        vector<Stream<float>*> process{};
        bool compiled{false}, map_of_map;

    public:

        DataFeed(std::vector<FloatStream> streams, bool map_of_map=true):Stream<float>("datafeed", move(streams)),
        map_of_map(map_of_map)
        {}

        void compile()
        {
            auto edges = this->gather();

            process = DataFeed::toposort(edges);

            compiled = true;

            this->reset();

        }

        void run() override
        {
            if( not compiled)
            {
                compile();
            }

            for(auto const& s : process)
            {
                s->run();
            }

            Stream::run();
        }

        FloatStreamType forward() override
        {
            return map_of_map ? forward_map_of_map() : forward_map_();
        }

        FloatStreamType forward_map_of_map()
        {
            unordered_map<string, unordered_map<string,float>> return_v;
            for(auto const& x: this->inputs)
            {
                auto v = x->Value();
                return_v[x->Name()] = std::get<unordered_map<string, float>>(v);
            }
            return return_v;
        }

        FloatStreamType forward_map_()
        {
            unordered_map<string,float> return_v;
            for(auto const& x: this->inputs)
            {
                return_v[x->Name()] = std::get<float>(x->Value());
            }
            return return_v;
        }

        template<class T>
        T next()
        {
            this->run();
            return std::get<T>(this->Value());
        }

        bool has_next() override
        {
            return std::all_of(process.begin(), process.end(), [](auto* x) { return x->has_next(); });
        }

        void reset() override
        {
            for(auto const& s : process)
            {
                s->reset();
            }
        }

    };

}


#endif //TENSORTRADECPP_FEED_H
