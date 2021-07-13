//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_GENERIC_H
#define TENSORTRADECPP_GENERIC_H

#include "tensortrade/feed/core/base.h"
#include "tensortrade/feed/core/operators.h"

namespace ttc {


    static std::shared_ptr<Stream> apply(std::shared_ptr<Stream> x, const std::function<double(double)>& fnc)
    {
        return std::make_shared<Apply>(x, fnc);
    }

    inline static std::shared_ptr<Stream> select(vector<std::shared_ptr<Stream>> streams,
                                                    std::function<bool(std::shared_ptr<Stream> const& )> const& fnc)
    {
        auto first = std::find_if(streams.begin(), streams.end(), [&fnc](auto const& stream){
            return fnc(stream);
        });

        if(first != streams.end())
            return *first;

        throw std::runtime_error("No stream satisfies selector condition.");
    }


}


#endif //TENSORTRADECPP_GENERIC_H
