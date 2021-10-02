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
    auto win = [&](atr_type &atr){
        return win_state;
    };
    auto lose = [&](atr_type &atr){
        return lose_state;
    };
    State init_state{"To be or not to be?", 
        {
            {
                "fight the demon",
                win
            },
            {
                "commit suicide",
                lose
            },
            {
                "quit the game"
            }
        }
    };
    State istate = init_state;
//std::cout<<init_state<<std::endl;
    atr_type m;
    m.clear();

    //main loop
    while(!istate.choices.empty()){
        std::cout<<istate<<std::endl;
        State next_state = istate.choices[0].trans(m);
        std::cout<<next_state<<std::endl;
        istate = next_state;
    }
    //init_state.pushdown();
    
}
int main(){
    init();
    return 0;
}