#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
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
        function<State(atr_type &)> trans = nullptr;
    };
    std::vector<Choices> choices;
    size_t current_choice = 0;

    //last choice 
    void pushup(){current_choice = (current_choice + choices.size() - 1) % choices.size();}
    //next choice
    void pushdown(){current_choice = (current_choice + 1) % choices.size();}
    //actually choose (current)
    State choose(atr_type &atr){
        return choices[current_choice].trans(atr);
    }
    //actually choose (indexing)
    State choose(atr_type &atr, size_t i){
        //assert(i >= 0);
        //assert(i < choices.size());
        return choices[i].trans(atr);
    }
};
ostream &operator <<(ostream &os, State S){
    os << "Q : " << S.description << std::endl;
    for(size_t i = 0; i<S.choices.size();i++){
        os << i + 1 << " : " << S.choices[i].description << std::endl;
    }
    os << "Current Choice :" << S.current_choice + 1 <<std::endl;
    return os;
}


//demo
void init(){
    State win_state{ "You win!!!!!"
    };
    State lose_state{ "You lose!!!!"
    };
    State init_state{"To be or not to be?", 
        {
            {
                "fight the demon",
                [&](atr_type &atr){
                    if(atr["power"] == "powerful"){
                        std::cout<<"great!\n"; //力大砖飞！
                        return win_state;
                    }
                    std::cout<<"lack of power!\n"; //没有力量
                    return lose_state;
                }
            },
            {
                "commit suicide",
                [&](atr_type &atr){
                    return lose_state;
                }
            },
            {
                "quit the game"
            }
        }
    };
    State istate;
    atr_type m;
    istate = init_state;
    m["power"] = "powerful";
    while(true)
    {
        std::cout<<istate<<std::endl;
        if(istate.choices.empty())break;
        State next_state = istate.choose(m);
        istate = next_state;
    }

    istate = init_state;
    m.clear();
    while(true)
    {
        std::cout<<istate<<std::endl;
        if(istate.choices.empty())break;
        State next_state = istate.choose(m);
        istate = next_state;
    }
    
}
int main(){
    init();
    return 0;
}