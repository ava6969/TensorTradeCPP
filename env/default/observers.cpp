//
// Created by dewe on 7/5/21.
//

#include "observers.h"

namespace ttc
{


    TensorTradeObserver::TensorTradeObserver(Portfolio *portfolio, DataFeed *dataFeed,
                                                  DataFeed *rendererFeed, int window_size,
                                                  std::optional<int> min_periods)
                                                  {

        auto streams = create_internal_streams(portfolio);
        auto internal_group = group<float>(move(streams));
        internal_group->rename<Stream<float>>("internal");

        auto external_group = group<float>(dataFeed->Inputs());
        external_group->rename<Stream<float>>("external");

        if(rendererFeed)
        {

        }else
        {
            feed = std::make_unique<DataFeed>(std::vector<FloatStream>{internal_group, external_group});
        }

        this->window_size = window_size;
        this->min_periods = min_periods;

        auto initial_obs = feed->next<unordered_map<string, unordered_map<string, float>>>().at("external");
        auto n_feature = initial_obs.size();

        dshape = {window_size, static_cast<int>(n_feature)};
        history = ObservationHistory(window_size, {static_cast<long>(n_feature)});
        feed->attach(portfolio);

        //renderer_history

        feed->reset();
        warmup();


    }

    void TensorTradeObserver::warmup() {

        if(min_periods)
        {
            int i = 0;
            while (i++ < min_periods.value())
            {
                if(hasNext())
                {
                    history.push(tensorView(feed->next<unordered_map<string,
                                            unordered_map<string, float>>>().at("external"))); // view of values
                }
            }
        }
    }

    torch::Tensor TensorTradeObserver::observe(struct TradingEnv *env) {

        auto data = feed->next<unordered_map<string, unordered_map<string, float>>>();

        auto obs_row = data["external"];
        history.push(tensorView(obs_row));

        return history.observe();

    }

    bool TensorTradeObserver::hasNext() {
        return feed->has_next();
    }

    void TensorTradeObserver::reset() {
        history.reset();
        feed->reset();
        warmup();
    }

    torch::Tensor TensorTradeObserver::tensorView(const unordered_map<string, float> &inp) {
        vector<float> values(inp.size());
        std::transform(inp.begin(), inp.end(), values.begin(),[](auto const& entry){
           return entry.second;
        });
        return torch::tensor(values);
    }

}
