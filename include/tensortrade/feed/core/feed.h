//
// Created by dewe on 6/29/21.
//

#ifndef TENSORTRADECPP_FEED_H
#define TENSORTRADECPP_FEED_H

#include "base.h"
#include "map"


using std::map;

namespace ttc
{

    template<typename PrimitiveDataType>
    class BaseDataFeed : public Group<PrimitiveDataType> {

    protected:
        vector<Stream<PrimitiveDataType>*> process{};
        bool compiled{};

    public:
        BaseDataFeed()
        {
            process ={};
            compiled = false;
        }

        BaseDataFeed(std::vector<Stream<PrimitiveDataType>*> streams):
        Group<PrimitiveDataType>(move(streams), "datafeed")
        {}

        void compile()
        {
            auto edges = this->gather();

            process = BaseDataFeed<PrimitiveDataType>::toposort(edges);

            compiled = true;

            this->reset();

        }

        virtual void run()
        {
            if( not compiled)
            {
                compile();
            }

            for(auto const& s : process)
            {
                s->forward();
            }

            this->forward();
        }

        std::unordered_map<string, PrimitiveDataType> next()
        {
            this->run();
            return this->mm_data;
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

    template<typename PrimitiveDataType>
    class DataFeed : public BaseDataFeed<PrimitiveDataType> {

        std::unordered_map<string, std::unordered_map<string, PrimitiveDataType>> mmm_data;

    public:

        DataFeed()=default;

        DataFeed(std::vector<Stream<PrimitiveDataType>*> const& streams): BaseDataFeed<PrimitiveDataType>(move(streams))
        {}

        void forward() override
        {
            for(auto const& x: this->m_inputs)
            {
                this->mmm_data[x->Name()]  = dynamic_cast<Group<PrimitiveDataType>* >(x)->asMapData();
            }
        }

        void reset() override
        {
            BaseDataFeed<PrimitiveDataType>::reset();
            for (auto* listener : this->listeners) {
                listener->reset();
            }
            this->mmm_data = {};
        }

        void run() override
        {
            BaseDataFeed<PrimitiveDataType>::run();

            for (Portfolio* listener : this->listeners) {
                listener->onNext(mmm_data);
            }
        }

        std::unordered_map<string, std::unordered_map<string, PrimitiveDataType>> mmm_next()
        {
            this->run();
            return this->mmm_data;
        }

        };




}


#endif //TENSORTRADECPP_FEED_H
