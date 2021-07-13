//
// Created by dewe on 6/29/21.
//

#ifndef TENSORTRADECPP_CLOCK_H
#define TENSORTRADECPP_CLOCK_H

#include "boost/date_time/posix_time/posix_time.hpp"
using namespace boost::posix_time;
using namespace boost::gregorian;

namespace ttc {


    struct Clock {

        int start{}, step{};

        Clock() = default;

        inline auto now() const {
            return second_clock::local_time().date();
        }

        inline auto now(std::string const &format) const {
            return to_simple_string(second_clock::local_time().date());
        }

        void increment() {
            step++;
        }

        inline auto Step() const { return step; }

        inline auto Start() const { return start; }

        void reset() {
            step = start;
        }

        bool operator==(Clock const& clk) const
        {
            return clk.step == step and clk.start == start;
        };

    };

}
#endif //TENSORTRADECPP_CLOCK_H
