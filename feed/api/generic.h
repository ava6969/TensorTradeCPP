//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_GENERIC_H
#define TENSORTRADECPP_GENERIC_H

#include "../../feed/core/base.h"
#include "../../feed/core/operators.h"

namespace ttc {


    template<typename T>
    static std::shared_ptr<Stream<T>> apply(std::shared_ptr<Stream<T>> x,
                                std::function<T(T)> fnc)
    {
        return std::make_shared<Apply<T>>(x, fnc);
    }

    template<typename T>
    inline static std::shared_ptr<Stream<T>> select(vector<std::shared_ptr<Stream<T>>> streams,
                                                    std::function<bool(std::shared_ptr<Stream<T>> const& )> const& fnc)
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
