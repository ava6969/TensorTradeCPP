//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_BOOLEAN_H
#define TENSORTRADECPP_BOOLEAN_H


namespace ttc {

    static Stream<bool>&& invert(Stream<bool> const& s) {
        return apply<bool, bool>(s, [power](bool const& x){ return not x; });
    }

}

#endif //TENSORTRADECPP_BOOLEAN_H
