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


#define  StreamType  \
std::variant<double, std::unordered_map<string, double>, std::vector<double>, \
std::unordered_map<string, unordered_map<string, double>>>


namespace ttc
{

    using std::string;
    using std::move;
    using std::vector;
    using std::unordered_map;
    using std::optional;
    using std::pair;
    using std::set;

    class Named {

    protected:

        static std::stack<string> namespaces;
        static unordered_map<string, int> names;
        string name{};

    public:

        explicit Named(string name="generic");
        [[nodiscard]] virtual inline string Name() const { return name; }

        template<class T=Named>
        T* rename(string _name, string const &sep = ":/")
        {
            if(not Named::namespaces.empty())
            {
                _name = Named::namespaces.top() + sep + _name;
            }
            name = std::move(_name);
            return dynamic_cast<T*>(this);
        }

        static void clear() { namespaces = std::stack<string>(); }

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


    class Stream : public Named, public Observable<Portfolio> {

    protected:
        vector<std::shared_ptr<Stream > > inputs;

    public:

        explicit Stream(string _name="stream", vector<std::shared_ptr<Stream> > _inputs={}):
                           Named(move(_name)), inputs(std::move(_inputs)) {}

        std::optional<StreamType> value{std::nullopt};

        auto Inputs() const { return inputs; }

        virtual StreamType forward() = 0;

        StreamType Value() const  { return value.value(); }

        virtual bool has_next()
        {
            return true;
        }

        virtual void run()
        {
            this->value = this->forward();
            for (auto* listener: listeners) {
                listener->onNext(std::get<unordered_map<string, unordered_map<string, double>>>(value.value()));
            }
        }

        virtual void reset()
        {
            for (auto* listener: listeners) {
                listener->reset();
            }

            for (auto const &stream: inputs) {
                stream->reset();
            }

            value = std::nullopt;
        }

        vector<pair<Stream*, Stream*>> gather() {vector<Stream* > v;
            vector<pair<Stream* , Stream* > > e;
           auto result =  _gather(this, v, e);
            return result;
        }

        static vector<pair<Stream*, Stream*>> _gather(Stream* stream, vector<Stream* >& vertices,
                                                      vector<pair<Stream* , Stream* > >&  edges) {

            if (std::find(vertices.begin(), vertices.end(), stream) == vertices.end())
            {
                vertices.push_back(stream);

                for (auto const& s : stream->inputs) {
                    edges.emplace_back(s.get(), stream);
                }

                for (auto const& s : stream->inputs) {
                    Stream::_gather(s.get(), vertices, edges);
                }
            }
            return edges;
        }

        static std::vector<Stream*>  toposort(vector<pair<Stream* , Stream* >> edges)
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

            return move(process);
        }

    };

    using Float64Stream = std::shared_ptr<Stream>;

    class IterableStream : public Stream {

        bool is_gen{false}, stop{false};
        std::vector<double> iterable;
        double current;
        decltype(iterable.begin()) ptr;

    public:
        explicit IterableStream(vector<double> source, string name="source"): Stream(std::move(name)), ptr(0)
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

        StreamType forward() override
        {
            auto v = current;
//            std::advance(ptr, 1);
            ptr++;
            current = *ptr;
            stop = (ptr + 1) == iterable.end();
            return v;
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

            Stream::reset();
        }
    };


    class Group : public Stream {

        std::map<string, std::shared_ptr<Stream> > streams;

    public:

        explicit Group(vector<std::shared_ptr<Stream > > _inputs, std::string name="group"):
        Stream(move(name), move(_inputs)) {

            for(auto const& x : this->inputs)
                streams[x->Name()] = x;
        }

        StreamType forward() override
        {
            std::unordered_map<string, double> res;
            for(auto const& x: this->inputs)
            {
                res[x->Name()] = std::get<double>(x->Value());
            }
            return res;
        }

        auto operator[](string const& name)
        {
            return streams[name];
        }

    };

    template<class ClassType>
    class Sensor : public Stream {

        static const string generic_name;
        ClassType const& obj;
        std::function<double(ClassType const&)> func;

    public:
        Sensor(ClassType const& _obj, std::function<double(ClassType const&)>  _func, std::string name="sensor"):
        Stream(move(name)), obj(_obj), func(std::move(_func)){}

        StreamType forward() override
        {
            return func(obj);
        }

    };

    class Constant : public Stream {

        double constant;

    public:
        explicit Constant(double const& value, std::string name="constant"):Stream(move(name)), constant(value)
        {}

        StreamType forward()
        {
            return constant;
        }

    };

    class Placeholder : public Stream {

    public:
        explicit Placeholder(std::string name="placeholder"):Stream(move(name))
        {
        }

        void push(double const& _value)
        {
            this->value = _value;
        }

        StreamType forward() override
        {
            return this->value.value();
        }

        void reset() override
        {
            this->value = std::nullopt;
        }


    };


    static std::shared_ptr<Stream> source(std::vector<double> vect)
    {
        return std::make_shared<IterableStream>(std::move(vect));
    }

    static std::shared_ptr<Stream> group(std::vector<std::shared_ptr<Stream>> vect)
    {
        return std::make_shared<Group>(std::move(vect));
    }

    template<class ClassType>
    static std::shared_ptr<Stream> sensor(ClassType const& _obj, std::function<double(ClassType const&)>  _func)
    {
        return std::make_shared<Sensor<ClassType>>(_obj, _func);
    }

    static std::shared_ptr<Stream> constant(double const& value)
    {
        return std::make_shared<Constant>(value);
    }

    static std::shared_ptr<Stream> placeholder()
    {
        return std::make_shared<Placeholder>();
    }

}

