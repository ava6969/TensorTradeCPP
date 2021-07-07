//
// Created by dewe on 6/30/21.
//

#ifndef TENSORTRADECPP_ORDER_LISTENER_H
#define TENSORTRADECPP_ORDER_LISTENER_H

namespace ttc
{
    class OrderListener {

    public:

        virtual void onExecute(class Order*){};
        virtual void onCancel(Order *) {};
        virtual void onFill(Order*, class Trade*) {};
        virtual void onComplete(Order*) {};

    };

}


#endif //TENSORTRADECPP_ORDER_LISTENER_H
