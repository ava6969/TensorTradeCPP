//
// Created by dewe on 7/5/21.
//

#ifndef TENSORTRADECPP_INFORMERS_H
#define TENSORTRADECPP_INFORMERS_H

#include "generic/components/informer.h"
#include "torch/torch.h"

namespace ttc
{

    class TTInformer : public Informer<float>
    {

    public:

        virtual std::unordered_map<std::string, float> info(class TradingEnv* env) const;
        virtual void reset() {};
    };

}

#endif //TENSORTRADECPP_INFORMERS_H
