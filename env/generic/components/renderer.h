//
// Created by dewe on 7/5/21.
//

#ifndef TENSORTRADECPP_RENDERER_H
#define TENSORTRADECPP_RENDERER_H


template<typename ... Args>
class Renderer
{
public:
    virtual void render(class TradingEnv*, Args ... args) = 0;
    virtual void save(){};
    virtual void reset(){};
    virtual void close(){};
};

#endif //TENSORTRADECPP_RENDERER_H
