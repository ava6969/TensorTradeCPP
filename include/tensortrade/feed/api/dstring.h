//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_DSTRING_H
#define TENSORTRADECPP_DSTRING_H

#include "generic.h"
#include "string"
#include "functional"

using std::function;
using std::string;

namespace ttc
{

    static auto capitalize(Stream<string> const& s)
    {

        return apply<string, string>(s, [](string x) -> string {
                                        x[0] = toupper(x[0]);
                                        return x; });
    }
}


#endif //TENSORTRADECPP_DSTRING_H
