//
// Created by dewe on 7/5/21.
//

#ifndef TENSORTRADECPP_STOPPERS_H
#define TENSORTRADECPP_STOPPERS_H


#include "tensortrade/env/generic/components/stopper.h"
#include "torch/torch.h"

namespace ttc {
    class MaxLossStopper : public Stopper {
        float m_max_allowed_loss;

    public:
        MaxLossStopper(float max_allowed_loss):m_max_allowed_loss(max_allowed_loss){}

        bool stop(class TradingEnv *env) override;
    };
}

#endif //TENSORTRADECPP_STOPPERS_H
