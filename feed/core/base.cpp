//
// Created by dewe on 6/29/21.
//
#include "base.h"
#include "wallets/portfolio.h"
#include <utility>

namespace ttc
{

    std::stack<string> Named::namespaces = {};

    unordered_map<string, int> Named::names = {};

    Named::Named(string _name){


        if(names.find(_name) != names.end())
        {
            names[_name] += 1;
            _name += ":/" + std::to_string(names[_name] - 1);
        }else
        {
            names[_name] = 0;
        }


        this->name = move(_name);

    }


}