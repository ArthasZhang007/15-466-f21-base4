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
        if(choices[current_choice].trans == nullptr)
        {
            return *this;
        }
        return choices[current_choice].trans(atr);
    }
    //actually choose (indexing)
    State choose(atr_type &atr, size_t i){
        //assert(i >= 0);
        //assert(i < choices.size());
        if(choices[i].trans == nullptr)
        {
            return *this;
        }
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











State id_state{
    "id"
};
void message(std::string mes){
    std::cout<<mes<<"\n\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}
State id_message(std::string mes){
    message(mes);
    return id_state;
}

State game_over;
State bad_endding1, bad_endding2, bad_endding3;
State good_endding;
State east_room_1F, north_room_1F;
State middle_state_1F;
State start_state;



void state_init(){
game_over = {
    "Game is over, try again!",
    {{
        "Restart!",
        [](atr_type &atr){
            atr.clear();
            return start_state;
        }
    }}
};
bad_endding1 = {
    "After trembling for hours in the freezing unknown place...... you are frozen to death."
    ,
    {{
        "Game Over!",
        [](atr_type &atr){return game_over;}
    }}
};
bad_endding2 = {
    "A sharp sword comes from the mirror and cut through your throat!"
    ,
    {{
        "Game Over!",
        [](atr_type &atr){return game_over;}
    }}
};
start_state = {
    "You wake up from a long dream, it feels like a million year has passed. "
    "Everything around you is pure darkness"
    "Now you feel a warm thing in your hand..... it seems like a button, do you want to push it?"
    ,
    {
        {
            "Yes",
            [](atr_type &atr){return middle_state_1F;}
        }
        ,
        {
            "No",
            [](atr_type &atr){return bad_endding1;}
        }
    }
};
middle_state_1F = {
    "Suddenly, multiple electric lights are on.... actually, wait. These are just kerosene lamps."
    "That's why the room is still so dim. Is this a mansion? prison? paradise? British classroom?"
    "You have absolutely no clue. The only thing you can see is that there are 8 doors around you, in 0', 45', 90' 135' 180' 225' "
    "270' 315' degree respectively, which one do you want to go?"
    ,
    {
        {
            "0",
            [](atr_type &atr){
                if(atr.find("door_0") != atr.end())
                {
                    return east_room_1F;
                }
                else{
                    return id_message("East room is locked. You can not open it now");
                }
            }
        },
        {
            "45",
            [](atr_type &atr){
                return id_message("This is a fake room");
            }
        },
        {
            "90",
            [](atr_type &atr){
                return north_room_1F;
            }
        },
        {
            "135",
            [](atr_type &atr){
                return id_message("This is a fake room");
            }
        },
        {
            "180",
            [](atr_type &atr){
                if(atr["east_room"] == "open"){
                    return east_room_1F;
                }
                return id_message("This is a fake room");
            }
        },
        {
            "225",
            [](atr_type &atr){
                return id_message("This is a fake room");
            }
        },
        {
            "270",
            [](atr_type &atr){
                return id_message("This is a fake room");
            }
        },
        {
            "315",
            [](atr_type &atr){
                return id_message("This is a fake room");
            }
        }
    }
};

north_room_1F = {
    "You pushed the door and found it ...... broken and crashed into fragments on the floor. "
    "A nice artistic portrait of some ...... king is in front of you. He is on a horse, using a long sword cutting through the throat of evil enemy. "
    "His soldiers are motivated by this greatly and start to fight as well, even if the enemies are strong. "
    "Now on the left side of the portrait, there is a mirror, on the right side there is another one. The left is decorated with diamonds and gold "
    "with a cross on it, the right one is made by simple wood and has a demon symbol on it, what do you want to do next? ",
    {
        {
            "look at the left mirror",
            [](atr_type &atr){
                return bad_endding2;
            }
        },
        {
            "look at the right mirror",
            [](atr_type &atr){
                atr["east_room"] = "open";
                message("You see a handsome boy with a great cowboy hat in the mirror, isn't it cool?");
                return id_message("You hear a crack sound from outside!");
            } 
        },
        {
            "exit the room",
            [](atr_type &atr){
                return middle_state_1F;
            } 
        }
    }
};

east_room_1F = {
    "It is so weird that door opens automatically, while just few minutes ago the door is as hard as a rock"
};

}



























//demo
void init(){
    State win_state{ "You win!!!!!"
    };
    State lose_state{ "You lose!!!!"
    };
    State init_state{
        "To be or not to be?", 
        {
            {
                "fight the demon",
                [&](atr_type &atr){
                    if(atr["power"] == "powerful"){
                        std::cout<<"great!\n"; //力大砖飞！
                        atr.erase("power");
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
    istate = start_state;
    m["power"] = "powerful";
    int i = -1;
    while(true)
    {
        std::cout<<istate<<std::endl;
        if(istate.choices.empty())break;
        
        std::cin>>i;
        State next_state = istate.choose(m, i - 1);
        if(next_state.description !="id"){
            istate = next_state;
        }
    }

    /*istate = init_state;
    m.clear();
    while(true)
    {
        std::cout<<istate<<std::endl;
        if(istate.choices.empty())break;
        State next_state = istate.choose(m);
        istate = next_state;
    }*/
    
}
int main(){
    state_init();
    init();
    return 0;
}