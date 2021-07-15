//
// Created by dewe on 6/29/21.
//

#pragma once
#include "string"
#include "vector"
#include "unordered_map"
#include "set"
#include "stack"
#include "tensortrade/core/base.h"
#include "tensortrade/oms/wallets/portfolio.h"
#include <cstddef>
#include <algorithm>
#include <utility>
#include "torch/torch.h"


namespace ttc
{

    using std::string;
    using std::move;
    using std::vector;
    using std::unordered_map;
    using std::optional;
    using std::pair;
    using std::set;

    template<typename OutputType>
    class Stream;

    template<typename PrimitiveType>
    class Group;

    class Named {

    protected:


        static unordered_map<string, int> names;
        string name{};

    public:

        static std::stack<string> namespaces;
        explicit Named(string name="generic");
        [[nodiscard]] virtual inline string Name() const { return name; }

        template<class T=double> Stream<T>* rename(string _name, string const &sep = ":/")
        {
            if(not Named::namespaces.empty())
            {
                _name = Named::namespaces.top() + sep + _name;
            }
            name = std::move(_name);
            return dynamic_cast<Stream<T>*>(this);
        }

    };

    class NameSpace : public Named {

    public:
        explicit NameSpace(string _name) : Named(move(_name)) {
            Named::namespaces.push(name);
        }

        ~NameSpace() {
            Named::namespaces.pop();
        }
    };

    template<typename T>
    class Stream : public Named, public Observable<Portfolio> {

    protected:
        T m_data;
    public:

        explicit Stream(string _name="stream"):Named(move(_name)) {}

        virtual void forward() = 0;

        virtual bool has_next()
        {
            return true;
        }

        auto value() const { return m_data; }

        virtual void reset()
        {
            m_data = {};
        }

        vector<pair<Stream<T>*, Stream<T>*>> gather() {
            vector<Stream<T>* > v;
            vector<pair<Stream<T>* , Stream<T>* > > e;
            auto result =  _gather(this, v, e);
            return result;
        }

        static vector<pair<Stream<T>*, Stream<T>*>> _gather(Stream<T>* stream, vector<Stream<T>* >& vertices,
                                                            vector<pair<Stream<T>* , Stream<T>* > >&  edges) {

            if (std::find(vertices.begin(), vertices.end(), stream) == vertices.end())
            {
                vertices.push_back(stream);

                if(auto group_type = dynamic_cast<Group<T>*>(stream))
                {
                    for (auto* s : group_type->inputs()) {
                        edges.emplace_back(s, stream);
                    }

                    for (auto* s : group_type->inputs()) {
                        Stream::_gather(s, vertices, edges);
                    }
                }

            }

            return edges;
        }

        static std::vector<Stream<T>*>  toposort(vector<pair<Stream* , Stream* >> edges)
        {
            auto splitter = [](vector<pair<Stream* , Stream* >> const& _e) -> std::pair<std::set<Stream*  >, std::set<Stream*  >> {
                std::vector<Stream* > src, tgt;

                for (auto const&[s, t] : _e) {
                    src.push_back(s);
                    tgt.push_back(t);
                }
                return {std::set<Stream*>(src.begin(), src.end()),
                        std::set<Stream*>(tgt.begin(), tgt.end())};
            };

            auto difference = [](std::set<Stream* > const& src, std::set<Stream* >  const& tgt){

                std::vector<Stream* > starting;
                std::set_difference(src.begin(), src.end(), tgt.begin(), tgt.end(), std::back_inserter(starting));
                return starting;
            };

            auto[src, tgt] = splitter(edges);
            std::vector<Stream* > starting = difference(src, tgt);

            auto process = starting;

            while (not starting.empty()) {
                auto start = starting.back();
                starting.pop_back();

                vector<pair<Stream* , Stream* >> temp;
                for(auto e: edges)
                {
                    if(e.first != start)
                    {
                        temp.template emplace_back(e);
                    }
                }

                edges = temp;

                std::tie(src, tgt) = splitter(edges);

                auto diff = difference(src, tgt);
                for (auto v: diff) {
                    if (find(starting.begin(), starting.end(), v) == starting.end())
                        starting.push_back(v);
                }

                if (find(process.begin(), process.end(), start) == process.end())
                    process.push_back(start);

            }

            return process;
        }
        virtual ~Stream() = default;
    };

    using Float64Stream = Stream<double>*;

    template<typename OutputType>
    class IterableStream : public Stream<OutputType> {

        bool is_gen{false}, stop{false};
        std::vector<OutputType> iterable;
        decltype(iterable.begin()) ptr;
        OutputType current;

        explicit IterableStream(vector<OutputType> const& source,
                                string name="source"):
                                Stream<OutputType>(std::move(name)), ptr(0)
        {

            iterable = source;
            ptr = iterable.begin();
            try {
                current = *ptr;
            }catch(std::exception const&)
            {
                stop = true;
            }
        }

    public:
        void forward() override
        {
            this->m_data = current;
            std::advance(ptr, 1);
            current = *ptr;
            stop = (ptr + 1) == iterable.end();
        }

        size_t size()
        {
            return iterable.size();
        }

        bool has_next() override
        {
            return not this->stop;
        }

        void reset() override
        {
            ptr = iterable.begin();
            stop = false;
            try {
                current = *ptr;
            }catch(std::exception const&)
            {
                stop = true;
            }

            Stream<OutputType>::reset();
        }

        friend class Ctx;
    };

    template<typename PrimitiveType=double>
    class Group : public Stream<PrimitiveType> {

    protected:

        vector< Stream<PrimitiveType>* > m_inputs;
        std::unordered_map<string, PrimitiveType> mm_data;
        explicit Group( vector<Stream<PrimitiveType>*> _inputs,
                        std::string name="group"): Stream<PrimitiveType>(move(name)), m_inputs(_inputs){
        }

        Group(){
            m_inputs = {};
            mm_data = {};
        }

    public:


        void forward() override
        {
            for(auto const& x: this->m_inputs)
            {
                this->mm_data[x->Name()]  = x->value();
            }
        }

        void reset()
        {
            Stream<PrimitiveType>::reset();
            this->mm_data = {};
            for (auto const &stream: m_inputs) {
                stream->reset();
            }
        }

        auto asMapData() const { return mm_data; }

        inline auto inputs() const{ return m_inputs; }

        friend class Ctx;
    };

    template<class ClassType, typename OutputType>
    class Sensor : public Stream<OutputType> {

        ClassType const& obj;
        std::function<double(ClassType const&)> func;


        Sensor(ClassType const& _obj,
               std::function<OutputType(ClassType const&)>  _func, std::string name="sensor"):
                Stream<OutputType>(move(name)), obj(_obj), func(std::move(_func)){}
    public:
        void forward() override
        {
            this->m_data = func(obj);
        }

        friend class Ctx;
    };

    template<typename OutputType=double>
    class Constant : public Stream<OutputType>  {

        OutputType constant;

        explicit Constant(OutputType const& value,
                          std::string name="constant"):
                          Stream<OutputType>(move(name)),
                          constant(value)
        {}

    public:
        void forward()
        {
            this->m_data = constant;
        }

        friend class Ctx;
    };

    template<typename OutputType=double,
            typename ListenerType=Portfolio>
    class Placeholder : public Stream<OutputType>  {

        explicit Placeholder(std::string name="placeholder"):
        Stream<OutputType>(move(name))
        {}

    public:
        void push(OutputType const& _value)
        {
            this->value = _value;
        }

        void forward() override
        {
            // does nothing
        }

        void reset() override
        {
            this->m_data = {};
        }

        friend class Ctx;

    };



}

