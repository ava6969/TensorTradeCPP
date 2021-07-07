//
// Created by dewe on 6/29/21.
//

#ifndef TENSORTRADECPP_BASE_H
#define TENSORTRADECPP_BASE_H

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <variant>
#include "optional"
#include "clock.h"

namespace ttc
{
    class Identifiable
    {

    protected:
        std::string id;

    public:

        Identifiable()
        {
            boost::mt19937 ran;
            boost::uuids::basic_random_generator<boost::mt19937> gen(&ran);
            auto data = gen();
            id = std::string(data.begin(), data.end());

        }

        [[nodiscard]] std::string ID() const
        {
            return id;
        }

        void setID(std::string const& identifier)
        {
            id = identifier;
        }

    };

    class TimeIndexed
    {

    public:
        Clock _clock;

        virtual void clock( Clock const& clock)
        {
            _clock = clock;
        }

    };


    class TimedIdentifiable : public Identifiable, public TimeIndexed
    {

        boost::gregorian::date::date_type created_at{};

    public:

        TimedIdentifiable()
        {
            created_at = _clock.now();
        }

        void clock( Clock const& clock) override
        {
            _clock = clock;
            created_at = _clock.now();
        }

        int createdAt() const
        {
            throw std::runtime_error("havent implemented yet");
//            return created_at;
        }

    };

    template<typename Listener>
    class Observable
    {
    protected:

        std::vector<Listener*> listeners;

    public:

        Observable()=default;

        Observable* attach(Listener* listener)
        {
            listeners.emplace_back(listener);
            return this;
        }

        Observable* detach(Listener* listener)
        {
            Listener l = listener;
            std::erase(listeners, l);
            return this;
        }

    };



}

#endif //TENSORTRADECPP_BASE_H
