//
// Created by dewe on 7/5/21.
//

#ifndef TENSORTRADECPP_ACTION_LISTENER_H
#define TENSORTRADECPP_ACTION_LISTENER_H


template<typename T>
class ActionListener
{
public:
    virtual void onAction(T action) = 0;
};
#endif //TENSORTRADECPP_ACTION_LISTENER_H
