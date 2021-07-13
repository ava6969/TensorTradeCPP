//
// Created by dewe on 7/5/21.
//

#ifndef TENSORTRADECPP_ENVIRONMENT_H
#define TENSORTRADECPP_ENVIRONMENT_H

#include "tensortrade/core/base.h"
#include "tensortrade/env/generic/components/action_scheme.h"
#include "tensortrade/env/generic/components/reward_scheme.h"
#include "tensortrade/env/generic/components/observer.h"
#include "tensortrade/env/generic/components/stopper.h"
#include "tensortrade/env/generic/components/informer.h"
#include "tensortrade/env/generic/components/renderer.h"
#include "torch/torch.h"
namespace ttc
{
    using std::move;
    class TradingEnv : public TimeIndexed
    {

        std::unique_ptr<ActionScheme<torch::Tensor>> m_action_scheme{};
        std::unique_ptr<RewardScheme> m_reward_scheme{};
        std::unique_ptr<Observer<torch::Tensor>> m_observer{};
        std::unique_ptr<Stopper> m_stopper{};
        std::unique_ptr<Informer<float>> m_informer{};
        std::unique_ptr<Renderer<float>> m_renderer{};

    public:
        TradingEnv()=default;
        TradingEnv(std::unique_ptr<ActionScheme<torch::Tensor>> actionScheme,
                   std::unique_ptr<RewardScheme> rewardScheme,
                   std::unique_ptr<Observer<torch::Tensor>> observer,
                   std::unique_ptr<Stopper> stopper,
                   std::unique_ptr<Informer<float>> informer,
                   std::unique_ptr<Renderer<float>> renderer= nullptr):
                   m_action_scheme(move(actionScheme)),
                   m_reward_scheme(move(rewardScheme)),
                   m_observer(move(observer)),
                   m_stopper(move(stopper)),
                   m_informer(move(informer)),
                   m_renderer(move(renderer)),
                   action_shape(m_action_scheme->actionShape()),
                   observation_shape(m_observer->observationSpace())
        {
            clock(new Clock());
            m_action_scheme->clock(_clock);
            m_reward_scheme->clock(_clock);
            m_observer->clock(_clock);
            m_stopper->clock(_clock);
            m_informer->clock(_clock);
        }

        std::vector<int> action_shape;
        std::vector<int> observation_shape;

        inline auto* actionScheme() { return m_action_scheme.get(); }
        inline auto* observer() { return m_observer.get(); }

        std::tuple<torch::Tensor, float, bool, std::unordered_map<std::string, float>>
        step( torch::Tensor const& action)
        {
            m_action_scheme->perform(this, action);

            auto obs = m_observer->observe(this);
            auto reward = m_reward_scheme->reward(this);
            auto done = m_stopper->stop(this);
            auto info = m_informer->info(this);

            _clock->increment();
            return {obs, reward, done, info};
        }

        torch::Tensor reset()
        {

            auto episode_id = rand();
            _clock->reset();
            if(m_action_scheme)
                m_action_scheme->reset();
            if(m_reward_scheme)
                m_reward_scheme->reset();
            if(m_observer)
                m_observer->reset();
            if(m_stopper)
                m_stopper->reset();
            if(m_informer)
                m_informer->reset();

            auto obs = m_observer->observe(this);
            _clock->increment();

            return obs;

        }

        void render()
        {
//            m_renderer->render(this);
        }

        void save()
        {
            m_renderer->save();
        }

        void close()
        {
            m_renderer->close();
        }

        ~TradingEnv() { delete _clock; }


    };

}


#endif //TENSORTRADECPP_ENVIRONMENT_H
