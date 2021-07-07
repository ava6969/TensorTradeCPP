//
// Created by dewe on 6/29/21.
//

#pragma once
#include "string"
#include "vector"
#include "unordered_map"
#include "set"
#include "stack"
#include "../../core/base.h"
#include "wallets/portfolio.h"
#include <cstddef>
#include <algorithm>
#include <utility>
#include "torch/torch.h"


#define  StreamType  \
std::variant<T, std::unordered_map<string, T>, std::vector<T>, std::unordered_map<string, unordered_map<string, T>>>


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
        vector<std::shared_ptr<Stream<T > > > inputs;
        std::optional<StreamType> value{std::nullopt};
        using TStream = std::shared_ptr<Stream<T>>;
    public:

        explicit Stream<T>(string _name="stream", vector<std::shared_ptr<Stream<T> > > _inputs={}):
                           Named(move(_name)), inputs(std::move(_inputs)) {}

        auto forward_T()
        {
            return std::get<T>(forward());
        }

        auto forward_map()
        {
            return std::get<std::map<string, T>>(forward());
        }

        auto forward_list()
        {
            return std::get<std::vector<T>>(forward());
        }

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
                listener->onNext(std::get<unordered_map<string, unordered_map<string, float>>>(value.value()));
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

                for (auto const& s : stream->inputs) {
                    edges.emplace_back(s.get(), stream);
                }

                for (auto const& s : stream->inputs) {
                    Stream::_gather(s.get(), vertices, edges);
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

            return move(process);
        }

    };

    using FloatStream = std::shared_ptr<Stream<float>>;

    template<typename T>
    class IterableStream : public Stream<T> {

        bool is_gen{false}, stop{false};
        vector<T> iterable;
        T current;
        decltype(iterable.begin()) ptr;

    public:
        explicit IterableStream(vector<T> source, string name="source"): Stream<T>(std::move(name)), ptr(0)
        {
            iterable = source;
            ptr = iterable.begin();
            try {
                ptr++;
                current = *ptr;
            }catch(std::exception const&)
            {
                stop = true;
            }
        }

        StreamType forward() override
        {
            auto v = current;
            try {
                ptr++;
                current = *ptr;
            }catch(std::exception const&)
            {
                stop = true;
            }
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
                ptr++;
                current = *ptr;
            }catch(std::exception const&)
            {
                stop = true;
            }

            Stream<T>::reset();
        }
    };

    template<typename T>
    class Group : public Stream<T> {

        std::map<string, std::shared_ptr<Stream<T>> > streams;

    public:

        explicit Group(vector<std::shared_ptr<Stream<T> > > _inputs, std::string name="group"):
        Stream<T>(move(name), move(_inputs)) {

            for(auto const& x : this->inputs)
                streams[x->Name()] = x;
        }

        StreamType forward() override
        {
            std::unordered_map<string, T> res;
            for(auto const& x: this->inputs)
            {
                res[x->Name()] = std::get<T>(x->Value());
            }
            return res;
        }

        auto operator[](string const& name)
        {
            return streams[name];
        }

    };

    template<class ClassType, typename T>
    class Sensor : public Stream<T> {

        static const string generic_name;
        ClassType* obj;
        std::function<T(ClassType*)> func;

    public:
        Sensor(ClassType* _obj, std::function<T(ClassType*)>  _func, std::string name="sensor"):
        Stream<T>(move(name)), obj(_obj), func(std::move(_func)){}

        StreamType forward() override
        {
            return func(obj);
        }

    };

    template<typename T>
    class Constant : public Stream<T> {

        torch::Tensor constant;

    public:
        explicit Constant(T const& value, std::string name="constant"):Stream<T>(move(name)), constant(value)
        {}

        std::variant<T, std::map<string, T>, std::vector<T>> forward()
        {
            return constant;
        }

    };

    template<typename T>
    class Placeholder : public Stream<T> {

    public:
        explicit Placeholder(std::string name="placeholder"):Stream<T>(move(name))
        {
        }

        void push(T const& _value)
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


    template<typename T>
    static std::shared_ptr<Stream<T>> source(std::vector<T> vect)
    {
        return std::make_shared<IterableStream<T>>(std::move(vect));
    }

    template<typename T>
    static std::shared_ptr<Stream<T>> group(std::vector<std::shared_ptr<Stream<T>>> vect)
    {
        return std::make_shared<Group<T>>(std::move(vect));
    }

    template<class ClassType, typename T>
    static std::shared_ptr<Stream<T>> sensor(ClassType* _obj, std::function<T(ClassType*)>  _func)
    {
        return std::make_shared<Sensor<ClassType, T>>(_obj, _func);
    }

    template<typename T>
    static std::shared_ptr<Stream<T>> constant(T const& value)
    {
        return std::make_shared<Constant<T>>(value);
    }

    template<typename T>
    static std::shared_ptr<Stream<T>> placeholder()
    {
        return std::make_shared<Placeholder<T>>();
    }

}

