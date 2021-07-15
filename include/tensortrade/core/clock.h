//
// Created by dewe on 6/29/21.
//

#ifndef TENSORTRADECPP_CLOCK_H
#define TENSORTRADECPP_CLOCK_H

#include "chrono"

namespace ttc {


    struct Clock {

        int start{}, step{};

        Clock() = default;

        inline auto now() const {
            return std::chrono::high_resolution_clock::now();
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
