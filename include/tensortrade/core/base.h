//
// Created by dewe on 6/29/21.
//

#ifndef TENSORTRADECPP_BASE_H
#define TENSORTRADECPP_BASE_H

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <variant>
#include "optional"
#include "clock.h"
#include "sstream"

namespace ttc
{
    class Identifiable
    {

    protected:
        std::string id;

    public:

        Identifiable()
        {
            boost::uuids::random_generator generator;
            boost::uuids::uuid uuid1 = generator();
            std::stringstream ss;
            ss << uuid1;
            id = ss.str();

        }

        Identifiable(Identifiable const&)=default;
        Identifiable& operator=(Identifiable const&)=default;
        bool operator==(Identifiable const& other) const
        {
            return id == other.id;
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

    struct TimeIndexed
    {

        Clock* _clock{nullptr};

        virtual void clock( Clock* m_clock)
        {
            _clock = m_clock;
        }

    };


    class TimedIdentifiable : public Identifiable, public TimeIndexed
    {

        std::chrono::time_point<std::chrono::system_clock,
        std::chrono::system_clock::duration> created_at{};

    public:

        TimedIdentifiable()
        {
            created_at = _clock->now();
        }

        TimedIdentifiable(TimedIdentifiable const&)=default;
        TimedIdentifiable& operator=(TimedIdentifiable const&)=default;

        void clock( Clock* m_clock) override
        {
            TimeIndexed::clock(m_clock);
            created_at = _clock->now();
        }

        [[nodiscard]] auto createdAt() const
        {
            return created_at;
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
            std::erase(listeners, listener);
            return this;
        }

    };



}

#endif //TENSORTRADECPP_BASE_H
