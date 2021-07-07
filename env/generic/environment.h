//
// Created by dewe on 7/5/21.
//

#ifndef TENSORTRADECPP_ENVIRONMENT_H
#define TENSORTRADECPP_ENVIRONMENT_H

#include "../../core/base.h"
#include "components/action_scheme.h"
#include "components/reward_scheme.h"
#include "components/observer.h"
#include "components/stopper.h"
#include "components/informer.h"
#include "components/renderer.h"
#include "torch/torch.h"
namespace ttc
{

    class TradingEnv : public TimeIndexed
    {

        ActionScheme<torch::Tensor>* m_action_scheme;
        RewardScheme* m_reward_scheme;
        Observer<torch::Tensor>* m_observer;
        Stopper* m_stopper;
        Informer<float>* m_informer;
        Renderer<float>* m_renderer;


    public:
        TradingEnv(ActionScheme<torch::Tensor>* actionScheme,
                   RewardScheme* rewardScheme,
                   Observer<torch::Tensor>* observer,
                   Stopper* stopper,
                   Informer<float>* informer,
                   Renderer<float>* renderer= nullptr):
                   m_action_scheme(actionScheme),
                   m_reward_scheme(rewardScheme),
                   m_observer(observer),
                   m_stopper(stopper),
                   m_informer(informer),
                   m_renderer(renderer),
                   action_shape(m_action_scheme->actionShape()),
                   observation_shape(m_observer->observationSpace())
        {
            m_action_scheme->_clock = _clock;
            m_reward_scheme->_clock = _clock;
            m_observer->_clock = _clock;
            m_stopper->_clock = _clock;
            m_informer->_clock = _clock;

        }

        const std::vector<int> action_shape;
        const std::vector<int> observation_shape;

        inline auto* actionScheme() { return m_action_scheme; }
        inline auto* observer() { return m_observer; }

        std::tuple<torch::Tensor, float, bool, std::unordered_map<std::string, float>>
        step( torch::Tensor const& action)
        {
            m_action_scheme->perform(this, action);

            auto obs = m_observer->observe(this);
            auto reward = m_reward_scheme->reward(this);
            auto done = m_stopper->stop(this);
            auto info = m_informer->info(this);

            _clock.increment();
            return {obs, reward, done, info};
        }

        torch::Tensor reset()
        {

            auto episode_id = rand();
            _clock.reset();
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
            _clock.increment();

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


    };

}


#endif //TENSORTRADECPP_ENVIRONMENT_H
