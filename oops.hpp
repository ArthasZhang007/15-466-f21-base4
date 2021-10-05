#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <thread>
#include <chrono>

using namespace std;

typedef std::map<std::string, std::string> atr_type;
class Player{
    public:
        //item -> attribute
        atr_type atr;
};
class State{
public:
    std::string description;
    class Choices{
    public:
        std::string description = "";
        function<State*(atr_type &)> trans = nullptr;
    };
    std::vector<Choices> choices;
    size_t current_choice = 0;

    //last choice 
    void pushup(){current_choice = (current_choice + choices.size() - 1) % choices.size();}
    //next choice
    void pushdown(){current_choice = (current_choice + 1) % choices.size();}
    //actually choose (current)
    State* choose(atr_type &atr){
        if(choices[current_choice].trans == nullptr)
        {
            return this;
        }
        return choices[current_choice].trans(atr);
    }
    //actually choose (indexing)
    State* choose(atr_type &atr, size_t i){
        //assert(i >= 0);
        //assert(i < choices.size());
        if(choices[i].trans == nullptr)
        {
            return this;
        }
        return choices[i].trans(atr);
    }
};
typedef State::Choices Choices;